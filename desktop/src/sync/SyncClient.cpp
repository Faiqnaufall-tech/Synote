#include "SyncClient.h"
#include "settings/Settings.h"

#include <QSqlQuery>
#include <QVariant>
#include <QUuid>
#include <QSqlError>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

namespace {
QByteArray doRequest(const QString& method, const QUrl& url, const QString& token, const QJsonObject& payload, int* statusOut) {
    QNetworkAccessManager manager;
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!token.isEmpty()) {
        req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    }

    QNetworkReply* reply = nullptr;
    if (method == "POST") {
        reply = manager.post(req, QJsonDocument(payload).toJson());
    } else {
        reply = manager.get(req);
    }

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (statusOut) {
        *statusOut = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    }
    const QByteArray data = reply->readAll();
    reply->deleteLater();
    return data;
}
}

SyncClient::SyncClient(QObject* parent) : QObject(parent) {}

bool SyncClient::ensureDeviceId(QString* deviceIdOut, QString* errorOut) {
    QSqlQuery query;
    if (!query.exec("SELECT device_id FROM sync_state WHERE id = 1")) {
        if (errorOut) *errorOut = query.lastError().text();
        return false;
    }
    if (query.next()) {
        if (deviceIdOut) *deviceIdOut = query.value(0).toString();
        return true;
    }

    const QString deviceId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlQuery insert;
    insert.prepare("INSERT INTO sync_state (id, device_id, last_sync_at) VALUES (1, ?, NULL)");
    insert.addBindValue(deviceId);
    if (!insert.exec()) {
        if (errorOut) *errorOut = insert.lastError().text();
        return false;
    }
    if (deviceIdOut) *deviceIdOut = deviceId;
    return true;
}

QString SyncClient::getLastSync() {
    QSqlQuery query;
    if (!query.exec("SELECT last_sync_at FROM sync_state WHERE id = 1")) {
        return QString();
    }
    if (query.next()) {
        return query.value(0).toString();
    }
    return QString();
}

bool SyncClient::setLastSync(const QString& isoTime, QString* errorOut) {
    QSqlQuery query;
    query.prepare("UPDATE sync_state SET last_sync_at = ? WHERE id = 1");
    query.addBindValue(isoTime);
    if (!query.exec()) {
        if (errorOut) *errorOut = query.lastError().text();
        return false;
    }
    return true;
}

bool SyncClient::syncNow(QString* errorOut) {
    Settings s = Settings::load();
    if (s.baseUrl.isEmpty() || s.token.isEmpty()) {
        if (errorOut) *errorOut = "Base URL atau token belum diset di Settings.";
        return false;
    }

    QString deviceId;
    if (!ensureDeviceId(&deviceId, errorOut)) {
        return false;
    }

    const QString lastSync = getLastSync();

    QJsonArray notes;
    {
        QSqlQuery q;
        if (lastSync.isEmpty()) {
            q.prepare("SELECT id, title, body, project_id, created_at, updated_at, deleted_at, version FROM notes");
        } else {
            q.prepare("SELECT id, title, body, project_id, created_at, updated_at, deleted_at, version FROM notes WHERE updated_at > ?");
            q.addBindValue(lastSync);
        }
        if (q.exec()) {
            while (q.next()) {
                QJsonObject obj;
                obj["id"] = q.value(0).toString();
                obj["title"] = q.value(1).toString();
                obj["body"] = q.value(2).toString();
                obj["project_id"] = q.value(3).toString();
                obj["created_at"] = q.value(4).toString();
                obj["updated_at"] = q.value(5).toString();
                obj["deleted_at"] = q.value(6).toString();
                obj["version"] = q.value(7).toInt();
                notes.append(obj);
            }
        }
    }

    QJsonArray projects;
    {
        QSqlQuery q;
        if (lastSync.isEmpty()) {
            q.prepare("SELECT id, name, created_at, updated_at, deleted_at, version FROM projects");
        } else {
            q.prepare("SELECT id, name, created_at, updated_at, deleted_at, version FROM projects WHERE updated_at > ?");
            q.addBindValue(lastSync);
        }
        if (q.exec()) {
            while (q.next()) {
                QJsonObject obj;
                obj["id"] = q.value(0).toString();
                obj["name"] = q.value(1).toString();
                obj["created_at"] = q.value(2).toString();
                obj["updated_at"] = q.value(3).toString();
                obj["deleted_at"] = q.value(4).toString();
                obj["version"] = q.value(5).toInt();
                projects.append(obj);
            }
        }
    }

    QJsonArray tags;
    {
        QSqlQuery q("SELECT id, name FROM tags");
        while (q.next()) {
            QJsonObject obj;
            obj["id"] = q.value(0).toString();
            obj["name"] = q.value(1).toString();
            tags.append(obj);
        }
    }

    QJsonArray noteTags;
    {
        QSqlQuery q("SELECT note_id, tag_id FROM note_tags");
        while (q.next()) {
            QJsonObject obj;
            obj["note_id"] = q.value(0).toString();
            obj["tag_id"] = q.value(1).toString();
            noteTags.append(obj);
        }
    }

    QJsonArray summaries;
    {
        QSqlQuery q;
        if (lastSync.isEmpty()) {
            q.prepare("SELECT id, note_id, project_id, content, created_at FROM summaries");
        } else {
            q.prepare("SELECT id, note_id, project_id, content, created_at FROM summaries WHERE created_at > ?");
            q.addBindValue(lastSync);
        }
        if (q.exec()) {
            while (q.next()) {
                QJsonObject obj;
                obj["id"] = q.value(0).toString();
                obj["note_id"] = q.value(1).toString();
                obj["project_id"] = q.value(2).toString();
                obj["content"] = q.value(3).toString();
                obj["created_at"] = q.value(4).toString();
                summaries.append(obj);
            }
        }
    }

    QJsonObject pushPayload;
    pushPayload["notes"] = notes;
    pushPayload["projects"] = projects;
    pushPayload["tags"] = tags;
    pushPayload["note_tags"] = noteTags;
    pushPayload["summaries"] = summaries;
    pushPayload["device_id"] = deviceId;

    int status = 0;
    const QByteArray pushResp = doRequest("POST", QUrl(s.baseUrl + "/sync/push"), s.token, pushPayload, &status);
    if (status < 200 || status >= 300) {
        if (errorOut) *errorOut = "Gagal push: " + QString::fromUtf8(pushResp);
        return false;
    }

    QUrl pullUrl(s.baseUrl + "/sync");
    if (!lastSync.isEmpty()) {
        QUrlQuery query;
        query.addQueryItem("since", lastSync);
        pullUrl.setQuery(query);
    }

    const QByteArray pullResp = doRequest("GET", pullUrl, s.token, QJsonObject(), &status);
    if (status < 200 || status >= 300) {
        if (errorOut) *errorOut = "Gagal pull: " + QString::fromUtf8(pullResp);
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(pullResp);
    if (!doc.isObject()) {
        if (errorOut) *errorOut = "Response pull tidak valid";
        return false;
    }

    const QJsonObject obj = doc.object();

    QSqlQuery tx("BEGIN");

    const QJsonArray pullProjects = obj.value("projects").toArray();
    for (const QJsonValue& v : pullProjects) {
        const QJsonObject p = v.toObject();
        QSqlQuery q;
        q.prepare("INSERT OR REPLACE INTO projects (id, name, created_at, updated_at, deleted_at, version) VALUES (?, ?, ?, ?, ?, ?)");
        q.addBindValue(p.value("id").toString());
        q.addBindValue(p.value("name").toString());
        q.addBindValue(p.value("created_at").toString());
        q.addBindValue(p.value("updated_at").toString());
        q.addBindValue(p.value("deleted_at").toString());
        q.addBindValue(p.value("version").toInt());
        q.exec();
    }

    const QJsonArray pullNotes = obj.value("notes").toArray();
    for (const QJsonValue& v : pullNotes) {
        const QJsonObject n = v.toObject();
        QSqlQuery q;
        q.prepare("INSERT OR REPLACE INTO notes (id, title, body, project_id, created_at, updated_at, deleted_at, version) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
        q.addBindValue(n.value("id").toString());
        q.addBindValue(n.value("title").toString());
        q.addBindValue(n.value("body").toString());
        q.addBindValue(n.value("project_id").toString());
        q.addBindValue(n.value("created_at").toString());
        q.addBindValue(n.value("updated_at").toString());
        q.addBindValue(n.value("deleted_at").toString());
        q.addBindValue(n.value("version").toInt());
        q.exec();
    }

    const QJsonArray pullTags = obj.value("tags").toArray();
    for (const QJsonValue& v : pullTags) {
        const QJsonObject t = v.toObject();
        QSqlQuery q;
        q.prepare("INSERT OR REPLACE INTO tags (id, name) VALUES (?, ?)");
        q.addBindValue(t.value("id").toString());
        q.addBindValue(t.value("name").toString());
        q.exec();
    }

    const QJsonArray pullNoteTags = obj.value("note_tags").toArray();
    for (const QJsonValue& v : pullNoteTags) {
        const QJsonObject nt = v.toObject();
        QSqlQuery q;
        q.prepare("INSERT OR IGNORE INTO note_tags (note_id, tag_id) VALUES (?, ?)");
        q.addBindValue(nt.value("note_id").toString());
        q.addBindValue(nt.value("tag_id").toString());
        q.exec();
    }

    const QJsonArray pullSummaries = obj.value("summaries").toArray();
    for (const QJsonValue& v : pullSummaries) {
        const QJsonObject sObj = v.toObject();
        QSqlQuery q;
        q.prepare("INSERT OR REPLACE INTO summaries (id, note_id, project_id, content, created_at) VALUES (?, ?, ?, ?, ?)");
        q.addBindValue(sObj.value("id").toString());
        q.addBindValue(sObj.value("note_id").toString());
        q.addBindValue(sObj.value("project_id").toString());
        q.addBindValue(sObj.value("content").toString());
        q.addBindValue(sObj.value("created_at").toString());
        q.exec();
    }

    QSqlQuery commit("COMMIT");

    const QString serverNow = obj.value("server_now").toString();
    if (!serverNow.isEmpty()) {
        setLastSync(serverNow, errorOut);
    }

    return true;
}
