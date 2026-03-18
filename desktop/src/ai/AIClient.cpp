#include "AIClient.h"
#include "settings/Settings.h"

#include <QSqlQuery>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace {
const int kMaxContextChars = 6000;
}

AIClient::Result AIClient::summarizeNote(const QString& noteId, const QString& title, const QString& body) {
    const QString ctx = buildContext(noteId);
    QJsonObject payload;
    payload["title"] = title;
    payload["body"] = body;
    payload["context"] = ctx;
    return postAI("/ai/summarize", payload);
}

AIClient::Result AIClient::draftFromTitle(const QString& title, const QString& hints) {
    const QString ctx = buildContext(QString());
    QJsonObject payload;
    payload["title"] = title;
    payload["hints"] = hints;
    payload["context"] = ctx;
    return postAI("/ai/draft", payload);
}

AIClient::Result AIClient::todoFromNote(const QString& title, const QString& body) {
    QJsonObject payload;
    payload["title"] = title;
    payload["body"] = body;
    return postAI("/ai/todo", payload);
}

AIClient::Result AIClient::postAI(const QString& path, const QJsonObject& payload) {
    Settings s = Settings::load();
    if (s.baseUrl.isEmpty() || s.token.isEmpty()) {
        return {false, "", "Base URL atau token belum diisi."};
    }

    QNetworkAccessManager manager;
    QNetworkRequest req(QUrl(s.baseUrl + path));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", ("Bearer " + s.token).toUtf8());

    QNetworkReply* reply = manager.post(req, QJsonDocument(payload).toJson());
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray data = reply->readAll();
    reply->deleteLater();

    if (status < 200 || status >= 300) {
        return {false, "", "Gagal memanggil AI: " + QString::fromUtf8(data)};
    }

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return {false, "", "Response AI tidak valid"};
    }

    const QString text = doc.object().value("text").toString().trimmed();
    if (text.isEmpty()) {
        return {false, "", "Response AI kosong"};
    }

    return {true, text, ""};
}

QString AIClient::buildContext(const QString& excludeNoteId) {
    QSqlQuery q;
    if (excludeNoteId.isEmpty()) {
        q.prepare("SELECT title, body FROM notes WHERE deleted_at IS NULL ORDER BY updated_at DESC");
    } else {
        q.prepare("SELECT title, body FROM notes WHERE deleted_at IS NULL AND id != ? ORDER BY updated_at DESC");
        q.addBindValue(excludeNoteId);
    }
    if (!q.exec()) {
        return QString();
    }

    QString ctx;
    while (q.next()) {
        const QString title = q.value(0).toString();
        const QString body = q.value(1).toString();
        const QString chunk = "- " + title + ": " + body.left(300) + "\n";
        if (ctx.size() + chunk.size() > kMaxContextChars) {
            break;
        }
        ctx += chunk;
    }
    return ctx.trimmed();
}
