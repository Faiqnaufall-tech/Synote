#pragma once

#include <QString>
#include <QJsonObject>

class AIClient {
public:
    struct Result {
        bool ok = false;
        QString text;
        QString error;
    };

    Result summarizeNote(const QString& noteId, const QString& title, const QString& body);
    Result draftFromTitle(const QString& title, const QString& hints);
    Result todoFromNote(const QString& title, const QString& body);

private:
    Result postAI(const QString& path, const QJsonObject& payload);
    QString buildContext(const QString& excludeNoteId);
};
