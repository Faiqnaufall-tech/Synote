#include "AppWindow.h"
#include "db/Database.h"
#include "sync/SyncClient.h"
#include "settings/SettingsDialog.h"
#include "ai/AIClient.h"
#include "ui/FlowLayout.h"
#include "ui/TagChip.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QUuid>
#include <QMessageBox>
#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QStatusBar>
#include <QInputDialog>
#include <QSet>
#include <QVector>
#include <QCompleter>
#include <QStringListModel>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

namespace {
QString nowIso() {
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
}

const QString kFilterAll = "__ALL__";
const QString kFilterNone = "__NONE__";
}

AppWindow::AppWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    loadProjects();
    loadTagFilters();
    loadNotes();
}

void AppWindow::setupUi() {
    setWindowTitle("Synote");
    resize(1100, 700);

    QWidget* central = new QWidget(this);
    QHBoxLayout* rootLayout = new QHBoxLayout(central);

    QVBoxLayout* leftLayout = new QVBoxLayout();
    searchBox_ = new QLineEdit();
    searchBox_->setPlaceholderText("Cari judul atau isi catatan...");
    notesList_ = new QListWidget();
    notesCountLabel_ = new QLabel();

    projectList_ = new QListWidget();
    projectList_->setMaximumHeight(160);

    projectFilterBox_ = new QComboBox();
    projectFilterBox_->setPlaceholderText("Filter project");

    QVBoxLayout* tagFilterLayout = new QVBoxLayout();
    tagFilterLayout->setSpacing(6);
    tagFilterLayout->addWidget(new QLabel("Filter Tag:"));

    tagFilterChipsContainer_ = new QWidget();
    auto* filterFlow = new FlowLayout(tagFilterChipsContainer_, 0, 6, 6);
    tagFilterChipsContainer_->setLayout(filterFlow);

    QScrollArea* filterScroll = new QScrollArea();
    filterScroll->setWidgetResizable(true);
    filterScroll->setFrameShape(QFrame::NoFrame);
    filterScroll->setWidget(tagFilterChipsContainer_);
    filterScroll->setMinimumHeight(70);

    tagFilterInput_ = new QLineEdit();
    tagFilterInput_->setPlaceholderText("Tambah tag filter, Enter");
    tagFilterCompleter_ = new QCompleter(allTags(), tagFilterInput_);
    tagFilterCompleter_->setCaseSensitivity(Qt::CaseInsensitive);
    tagFilterCompleter_->setFilterMode(Qt::MatchContains);
    tagFilterInput_->setCompleter(tagFilterCompleter_);
    connect(tagFilterInput_, &QLineEdit::returnPressed, this, [this]() {
        const QString input = tagFilterInput_->text().trimmed();
        if (input.isEmpty()) {
            return;
        }
        const QStringList tags = parseTags(input);
        for (const QString& t : tags) {
            addTagFilterChip(t);
        }
        tagFilterInput_->clear();
        loadNotes(searchBox_->text());
    });

    tagFilterLayout->addWidget(filterScroll);
    tagFilterLayout->addWidget(tagFilterInput_);

    QPushButton* newBtn = new QPushButton("Catatan Baru");
    connect(newBtn, &QPushButton::clicked, this, &AppWindow::onNewNote);

    leftLayout->addWidget(searchBox_);
    leftLayout->addWidget(new QLabel("Project"));
    leftLayout->addWidget(projectList_);
    leftLayout->addWidget(projectFilterBox_);
    leftLayout->addLayout(tagFilterLayout);
    leftLayout->addWidget(notesList_);
    leftLayout->addWidget(notesCountLabel_);
    leftLayout->addWidget(newBtn);

    QVBoxLayout* rightLayout = new QVBoxLayout();
    titleEdit_ = new QLineEdit();
    titleEdit_->setPlaceholderText("Judul catatan");

    bodyEdit_ = new QTextEdit();
    bodyEdit_->setPlaceholderText("Tulis isi catatan di sini...");

    QHBoxLayout* metaLayout = new QHBoxLayout();
    projectBox_ = new QComboBox();
    projectBox_->addItem("(Tanpa Project)", QVariant());
    metaLayout->addWidget(new QLabel("Project:"));
    metaLayout->addWidget(projectBox_);
    QPushButton* addProjectBtn = new QPushButton("Tambah Project");
    connect(addProjectBtn, &QPushButton::clicked, this, &AppWindow::onAddProject);
    metaLayout->addWidget(addProjectBtn);

    QVBoxLayout* tagLayout = new QVBoxLayout();
    tagLayout->setSpacing(6);
    tagLayout->addWidget(new QLabel("Tag:"));

    tagChipsContainer_ = new QWidget();
    auto* flow = new FlowLayout(tagChipsContainer_, 0, 6, 6);
    tagChipsContainer_->setLayout(flow);

    QScrollArea* chipScroll = new QScrollArea();
    chipScroll->setWidgetResizable(true);
    chipScroll->setFrameShape(QFrame::NoFrame);
    chipScroll->setWidget(tagChipsContainer_);
    chipScroll->setMinimumHeight(60);

    tagInput_ = new QLineEdit();
    tagInput_->setPlaceholderText("Tambah tag, tekan Enter");
    tagCompleter_ = new QCompleter(allTags(), tagInput_);
    tagCompleter_->setCaseSensitivity(Qt::CaseInsensitive);
    tagCompleter_->setFilterMode(Qt::MatchContains);
    tagInput_->setCompleter(tagCompleter_);
    connect(tagInput_, &QLineEdit::returnPressed, this, [this]() {
        const QString input = tagInput_->text().trimmed();
        if (input.isEmpty()) {
            return;
        }
        const QStringList tags = parseTags(input);
        for (const QString& t : tags) {
            addTagChip(t);
        }
        tagInput_->clear();
    });

    tagLayout->addWidget(chipScroll);
    tagLayout->addWidget(tagInput_);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    QPushButton* saveBtn = new QPushButton("Simpan");
    connect(saveBtn, &QPushButton::clicked, this, &AppWindow::onSaveNote);

    syncBtn_ = new QPushButton("Sync Sekarang");
    connect(syncBtn_, &QPushButton::clicked, this, &AppWindow::onSyncNow);

    QPushButton* settingsBtn = new QPushButton("Pengaturan");
    connect(settingsBtn, &QPushButton::clicked, this, &AppWindow::onOpenSettings);

    aiSummaryBtn_ = new QPushButton("AI: Ringkas");
    aiDraftBtn_ = new QPushButton("AI: Draft");
    aiTodoBtn_ = new QPushButton("AI: To-Do");
    connect(aiSummaryBtn_, &QPushButton::clicked, this, &AppWindow::onAiSummarize);
    connect(aiDraftBtn_, &QPushButton::clicked, this, &AppWindow::onAiDraft);
    connect(aiTodoBtn_, &QPushButton::clicked, this, &AppWindow::onAiTodo);

    actionLayout->addWidget(saveBtn);
    actionLayout->addWidget(syncBtn_);
    actionLayout->addWidget(settingsBtn);
    actionLayout->addWidget(aiSummaryBtn_);
    actionLayout->addWidget(aiDraftBtn_);
    actionLayout->addWidget(aiTodoBtn_);

    rightLayout->addWidget(titleEdit_);
    rightLayout->addWidget(bodyEdit_);
    rightLayout->addLayout(metaLayout);
    rightLayout->addLayout(tagLayout);
    rightLayout->addLayout(actionLayout);

    QSplitter* splitter = new QSplitter();
    QWidget* leftPane = new QWidget();
    leftPane->setLayout(leftLayout);
    QWidget* rightPane = new QWidget();
    rightPane->setLayout(rightLayout);
    splitter->addWidget(leftPane);
    splitter->addWidget(rightPane);
    splitter->setStretchFactor(1, 1);

    rootLayout->addWidget(splitter);
    central->setLayout(rootLayout);
    setCentralWidget(central);

    statusLabel_ = new QLabel("Status: Offline");
    statusBar()->addPermanentWidget(statusLabel_);

    connect(notesList_, &QListWidget::itemClicked, this, &AppWindow::onNoteSelected);
    connect(searchBox_, &QLineEdit::textChanged, this, &AppWindow::onSearchChanged);
    connect(projectFilterBox_, &QComboBox::currentIndexChanged, this, [this]() {
        loadNotes(searchBox_->text());
    });
    connect(projectList_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        const QString id = item->data(Qt::UserRole).toString();
        int idx = projectFilterBox_->findData(id);
        if (idx < 0) {
            idx = 0;
        }
        projectFilterBox_->setCurrentIndex(idx);
        loadNotes(searchBox_->text());
    });
}

void AppWindow::onNewNote() {
    currentNoteId_.clear();
    titleEdit_->clear();
    bodyEdit_->clear();
    clearTagChips();
    tagInput_->clear();
    projectBox_->setCurrentIndex(0);
    titleEdit_->setFocus();
}

void AppWindow::onSaveNote() {
    const QString title = titleEdit_->text().trimmed();
    const QString body = bodyEdit_->toPlainText().trimmed();

    if (title.isEmpty()) {
        QMessageBox::warning(this, "Validasi", "Judul tidak boleh kosong.");
        return;
    }

    QSqlQuery query;
    const QString now = nowIso();
    QVariant projectId = projectBox_->currentData();

    if (currentNoteId_.isEmpty()) {
        const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        query.prepare("INSERT INTO notes (id, title, body, project_id, created_at, updated_at, version) VALUES (?, ?, ?, ?, ?, ?, 1)");
        query.addBindValue(id);
        query.addBindValue(title);
        query.addBindValue(body);
        query.addBindValue(projectId);
        query.addBindValue(now);
        query.addBindValue(now);
        if (!query.exec()) {
            QMessageBox::critical(this, "DB", query.lastError().text());
            return;
        }
        currentNoteId_ = id;
    } else {
        query.prepare("UPDATE notes SET title = ?, body = ?, project_id = ?, updated_at = ?, version = version + 1 WHERE id = ?");
        query.addBindValue(title);
        query.addBindValue(body);
        query.addBindValue(projectId);
        query.addBindValue(now);
        query.addBindValue(currentNoteId_);
        if (!query.exec()) {
            QMessageBox::critical(this, "DB", query.lastError().text());
            return;
        }
    }

    saveTagsForNote(currentNoteId_);
    loadNotes(searchBox_->text());
}

void AppWindow::onNoteSelected(QListWidgetItem* item) {
    const QString noteId = item->data(Qt::UserRole).toString();

    QSqlQuery query;
    query.prepare("SELECT title, body, project_id FROM notes WHERE id = ? AND deleted_at IS NULL");
    query.addBindValue(noteId);
    if (!query.exec() || !query.next()) {
        return;
    }

    currentNoteId_ = noteId;
    titleEdit_->setText(query.value(0).toString());
    bodyEdit_->setPlainText(query.value(1).toString());
    const QString projectId = query.value(2).toString();
    int index = projectBox_->findData(projectId);
    if (index < 0) {
        index = 0;
    }
    projectBox_->setCurrentIndex(index);
    loadTagsForNote(noteId);
}

void AppWindow::onSearchChanged(const QString& text) {
    loadNotes(text);
}

void AppWindow::onSyncNow() {
    statusLabel_->setText("Status: Syncing...");
    if (syncBtn_) {
        syncBtn_->setEnabled(false);
    }
    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);
    QString* error = new QString();

    auto future = QtConcurrent::run([error]() -> bool {
        SyncClient client;
        return client.syncNow(error);
    });

    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, error]() {
        const bool ok = watcher->result();
        if (!ok) {
            statusLabel_->setText("Status: Sync gagal");
            QMessageBox::warning(this, "Sync", *error);
        } else {
            statusLabel_->setText("Status: Synced");
        }
        if (syncBtn_) {
            syncBtn_->setEnabled(true);
        }
        delete error;
        watcher->deleteLater();
    });

    watcher->setFuture(future);
}

void AppWindow::onOpenSettings() {
    SettingsDialog dlg(this);
    dlg.exec();
}

void AppWindow::onAddProject() {
    bool ok = false;
    const QString name = QInputDialog::getText(this, "Project Baru", "Nama project:", QLineEdit::Normal, "", &ok).trimmed();
    if (!ok || name.isEmpty()) {
        return;
    }

    QSqlQuery query;
    const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString now = nowIso();
    query.prepare("INSERT INTO projects (id, name, created_at, updated_at, version) VALUES (?, ?, ?, ?, 1)");
    query.addBindValue(id);
    query.addBindValue(name);
    query.addBindValue(now);
    query.addBindValue(now);
    if (!query.exec()) {
        QMessageBox::critical(this, "DB", query.lastError().text());
        return;
    }
    loadProjects();
    int index = projectBox_->findData(id);
    if (index >= 0) {
        projectBox_->setCurrentIndex(index);
    }
}

void AppWindow::onAiSummarize() {
    if (titleEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "AI", "Judul catatan masih kosong.");
        return;
    }

    if (currentNoteId_.isEmpty()) {
        onSaveNote();
        if (currentNoteId_.isEmpty()) {
            return;
        }
    }

    if (aiSummaryBtn_) aiSummaryBtn_->setEnabled(false);
    if (aiDraftBtn_) aiDraftBtn_->setEnabled(false);
    if (aiTodoBtn_) aiTodoBtn_->setEnabled(false);

    QFutureWatcher<AIClient::Result>* watcher = new QFutureWatcher<AIClient::Result>(this);
    const QString noteId = currentNoteId_;
    const QString title = titleEdit_->text();
    const QString body = bodyEdit_->toPlainText();

    auto future = QtConcurrent::run([noteId, title, body]() -> AIClient::Result {
        AIClient client;
        return client.summarizeNote(noteId, title, body);
    });

    const QString summaryNoteId = noteId;
    connect(watcher, &QFutureWatcher<AIClient::Result>::finished, this, [this, watcher, summaryNoteId]() {
        const auto result = watcher->result();
        if (!result.ok) {
            QMessageBox::warning(this, "AI", result.error);
        } else {
            QSqlQuery q;
            q.prepare("INSERT INTO summaries (id, note_id, content, created_at) VALUES (?, ?, ?, ?)");
            q.addBindValue(QUuid::createUuid().toString(QUuid::WithoutBraces));
            q.addBindValue(summaryNoteId);
            q.addBindValue(result.text);
            q.addBindValue(nowIso());
            q.exec();

            QDialog dlg(this);
            dlg.setWindowTitle("Ringkasan AI");
            dlg.resize(600, 400);
            QVBoxLayout* layout = new QVBoxLayout(&dlg);
            QTextEdit* text = new QTextEdit();
            text->setReadOnly(true);
            text->setPlainText(result.text);
            QPushButton* closeBtn = new QPushButton("Tutup");
            connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
            layout->addWidget(text);
            layout->addWidget(closeBtn);
            dlg.exec();
        }

        if (aiSummaryBtn_) aiSummaryBtn_->setEnabled(true);
        if (aiDraftBtn_) aiDraftBtn_->setEnabled(true);
        if (aiTodoBtn_) aiTodoBtn_->setEnabled(true);
        watcher->deleteLater();
    });

    watcher->setFuture(future);
}

void AppWindow::onAiDraft() {
    const QString title = titleEdit_->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "AI", "Judul catatan masih kosong.");
        return;
    }

    if (aiSummaryBtn_) aiSummaryBtn_->setEnabled(false);
    if (aiDraftBtn_) aiDraftBtn_->setEnabled(false);
    if (aiTodoBtn_) aiTodoBtn_->setEnabled(false);

    QFutureWatcher<AIClient::Result>* watcher = new QFutureWatcher<AIClient::Result>(this);
    const QString hints = bodyEdit_->toPlainText();

    auto future = QtConcurrent::run([title, hints]() -> AIClient::Result {
        AIClient client;
        return client.draftFromTitle(title, hints);
    });

    connect(watcher, &QFutureWatcher<AIClient::Result>::finished, this, [this, watcher]() {
        const auto result = watcher->result();
        if (!result.ok) {
            QMessageBox::warning(this, "AI", result.error);
        } else {
            bodyEdit_->setPlainText(result.text);
        }
        if (aiSummaryBtn_) aiSummaryBtn_->setEnabled(true);
        if (aiDraftBtn_) aiDraftBtn_->setEnabled(true);
        if (aiTodoBtn_) aiTodoBtn_->setEnabled(true);
        watcher->deleteLater();
    });

    watcher->setFuture(future);
}

void AppWindow::onAiTodo() {
    const QString title = titleEdit_->text().trimmed();
    const QString body = bodyEdit_->toPlainText().trimmed();
    if (title.isEmpty() && body.isEmpty()) {
        QMessageBox::warning(this, "AI", "Catatan masih kosong.");
        return;
    }

    if (aiSummaryBtn_) aiSummaryBtn_->setEnabled(false);
    if (aiDraftBtn_) aiDraftBtn_->setEnabled(false);
    if (aiTodoBtn_) aiTodoBtn_->setEnabled(false);

    QFutureWatcher<AIClient::Result>* watcher = new QFutureWatcher<AIClient::Result>(this);

    auto future = QtConcurrent::run([title, body]() -> AIClient::Result {
        AIClient client;
        return client.todoFromNote(title, body);
    });

    connect(watcher, &QFutureWatcher<AIClient::Result>::finished, this, [this, watcher]() {
        const auto result = watcher->result();
        if (!result.ok) {
            QMessageBox::warning(this, "AI", result.error);
        } else {
            bodyEdit_->setPlainText(result.text);
        }
        if (aiSummaryBtn_) aiSummaryBtn_->setEnabled(true);
        if (aiDraftBtn_) aiDraftBtn_->setEnabled(true);
        if (aiTodoBtn_) aiTodoBtn_->setEnabled(true);
        watcher->deleteLater();
    });

    watcher->setFuture(future);
}

void AppWindow::loadNotes(const QString& search) {
    notesList_->clear();

    QSqlQuery query;
    const QString searchText = search.trimmed();
    const QString projectFilter = projectFilterBox_->currentData().toString();
    const QStringList tagFilters = currentTagFilters();

    QString sql = "SELECT DISTINCT n.id, n.title, n.updated_at FROM notes n ";
    if (!searchText.isEmpty()) {
        sql += "JOIN notes_fts f ON n.rowid = f.rowid ";
    }
    if (!tagFilters.isEmpty()) {
        sql += "JOIN note_tags nt ON nt.note_id = n.id ";
        sql += "JOIN tags t ON t.id = nt.tag_id ";
    }
    sql += "WHERE n.deleted_at IS NULL ";
    if (!searchText.isEmpty()) {
        sql += "AND notes_fts MATCH ? ";
    }
    if (projectFilter == kFilterNone) {
        sql += "AND n.project_id IS NULL ";
    } else if (!projectFilter.isEmpty() && projectFilter != kFilterAll) {
        sql += "AND n.project_id = ? ";
    }
    if (!tagFilters.isEmpty()) {
        sql += "AND t.name IN (";
        for (int i = 0; i < tagFilters.size(); ++i) {
            sql += (i == 0 ? "?" : ",?");
        }
        sql += ") ";
    }
    sql += "ORDER BY n.updated_at DESC";

    query.prepare(sql);
    if (!searchText.isEmpty()) {
        query.addBindValue(searchText + "*");
    }
    if (projectFilter != kFilterNone && !projectFilter.isEmpty() && projectFilter != kFilterAll) {
        query.addBindValue(projectFilter);
    }
    for (const QString& t : tagFilters) {
        query.addBindValue(t);
    }

    if (!query.exec()) {
        return;
    }

    int count = 0;
    while (query.next()) {
        QListWidgetItem* item = new QListWidgetItem(query.value(1).toString());
        item->setData(Qt::UserRole, query.value(0).toString());
        notesList_->addItem(item);
        count++;
    }
    notesCountLabel_->setText(QString::number(count) + " catatan");
}

void AppWindow::loadProjects() {
    projectBox_->clear();
    projectBox_->addItem("(Tanpa Project)", QVariant());

    QVector<QPair<QString, QString>> projects;
    QSqlQuery query("SELECT id, name FROM projects WHERE deleted_at IS NULL ORDER BY updated_at DESC");
    while (query.next()) {
        const QString id = query.value(0).toString();
        const QString name = query.value(1).toString();
        projects.append({id, name});
        projectBox_->addItem(name, id);
    }

    const QString currentFilter = projectFilterBox_->currentData().toString();
    projectFilterBox_->clear();
    projectFilterBox_->addItem("Semua Project", kFilterAll);
    projectFilterBox_->addItem("(Tanpa Project)", kFilterNone);
    for (const auto& p : projects) {
        projectFilterBox_->addItem(p.second, p.first);
    }

    int idx = projectFilterBox_->findData(currentFilter);
    if (idx < 0) {
        idx = 0;
    }
    projectFilterBox_->setCurrentIndex(idx);

    projectList_->clear();
    QListWidgetItem* allItem = new QListWidgetItem("Semua Project");
    allItem->setData(Qt::UserRole, kFilterAll);
    projectList_->addItem(allItem);
    QListWidgetItem* noneItem = new QListWidgetItem("(Tanpa Project)");
    noneItem->setData(Qt::UserRole, kFilterNone);
    projectList_->addItem(noneItem);
    for (const auto& p : projects) {
        QListWidgetItem* it = new QListWidgetItem(p.second);
        it->setData(Qt::UserRole, p.first);
        projectList_->addItem(it);
    }

    for (int i = 0; i < projectList_->count(); ++i) {
        if (projectList_->item(i)->data(Qt::UserRole).toString() == currentFilter) {
            projectList_->setCurrentRow(i);
            break;
        }
    }
}

void AppWindow::loadTagsForNote(const QString& noteId) {
    clearTagChips();
    QSqlQuery q;
    q.prepare(
        "SELECT t.name FROM note_tags nt "
        "JOIN tags t ON t.id = nt.tag_id "
        "WHERE nt.note_id = ?"
    );
    q.addBindValue(noteId);
    if (!q.exec()) {
        tagInput_->clear();
        return;
    }

    while (q.next()) {
        addTagChip(q.value(0).toString());
    }
}

void AppWindow::saveTagsForNote(const QString& noteId) {
    const QStringList tags = currentTags();

    QSqlQuery del;
    del.prepare("DELETE FROM note_tags WHERE note_id = ?");
    del.addBindValue(noteId);
    del.exec();

    for (const QString& tag : tags) {
        QString tagId;
        QSqlQuery find;
        find.prepare("SELECT id FROM tags WHERE name = ?");
        find.addBindValue(tag);
        if (find.exec() && find.next()) {
            tagId = find.value(0).toString();
        } else {
            tagId = QUuid::createUuid().toString(QUuid::WithoutBraces);
            QSqlQuery insert;
            insert.prepare("INSERT INTO tags (id, name) VALUES (?, ?)");
            insert.addBindValue(tagId);
            insert.addBindValue(tag);
            insert.exec();
        }

        QSqlQuery link;
        link.prepare("INSERT INTO note_tags (note_id, tag_id) VALUES (?, ?)");
        link.addBindValue(noteId);
        link.addBindValue(tagId);
        link.exec();
    }

    loadTagFilters();
}

QStringList AppWindow::parseTags(const QString& input) const {
    const QStringList parts = input.split(",", Qt::SkipEmptyParts);
    QSet<QString> uniq;
    QStringList out;
    for (const QString& p : parts) {
        const QString t = p.trimmed();
        if (t.isEmpty()) {
            continue;
        }
        if (!uniq.contains(t)) {
            uniq.insert(t);
            out << t;
        }
    }
    return out;
}

void AppWindow::clearTagChips() {
    if (!tagChipsContainer_ || !tagChipsContainer_->layout()) {
        return;
    }
    QLayout* layout = tagChipsContainer_->layout();
    while (QLayoutItem* item = layout->takeAt(0)) {
        QWidget* w = item->widget();
        delete item;
        if (w) {
            w->deleteLater();
        }
    }
}

void AppWindow::addTagChip(const QString& tag) {
    const QString t = tag.trimmed();
    if (t.isEmpty()) {
        return;
    }
    const QStringList existing = currentTags();
    if (existing.contains(t, Qt::CaseInsensitive)) {
        return;
    }

    TagChip* chip = new TagChip(t, tagChipsContainer_);
    connect(chip, &TagChip::removed, this, [this](const QString&) {});
    tagChipsContainer_->layout()->addWidget(chip);
}

QStringList AppWindow::currentTags() const {
    QStringList out;
    if (!tagChipsContainer_ || !tagChipsContainer_->layout()) {
        return out;
    }
    const QLayout* layout = tagChipsContainer_->layout();
    for (int i = 0; i < layout->count(); ++i) {
        QWidget* w = layout->itemAt(i)->widget();
        TagChip* chip = qobject_cast<TagChip*>(w);
        if (chip) {
            out << chip->text();
        }
    }
    return out;
}

void AppWindow::loadTagFilters() {
    clearTagFilterChips();
    QSqlQuery q("SELECT name FROM tags ORDER BY name ASC");
    while (q.next()) {
        addTagFilterChip(q.value(0).toString());
    }

    const QStringList tags = allTags();
    if (tagCompleter_) {
        tagCompleter_->setModel(new QStringListModel(tags, tagCompleter_));
    }
    if (tagFilterCompleter_) {
        tagFilterCompleter_->setModel(new QStringListModel(tags, tagFilterCompleter_));
    }

    loadNotes(searchBox_->text());
}

void AppWindow::clearTagFilterChips() {
    if (!tagFilterChipsContainer_ || !tagFilterChipsContainer_->layout()) {
        return;
    }
    QLayout* layout = tagFilterChipsContainer_->layout();
    while (QLayoutItem* item = layout->takeAt(0)) {
        QWidget* w = item->widget();
        delete item;
        if (w) {
            w->deleteLater();
        }
    }
}

void AppWindow::addTagFilterChip(const QString& tag) {
    const QString t = tag.trimmed();
    if (t.isEmpty()) {
        return;
    }

    const QStringList existing = currentTagFilters();
    if (existing.contains(t, Qt::CaseInsensitive)) {
        return;
    }

    TagChip* chip = new TagChip(t, tagFilterChipsContainer_);
    chip->setStyleSheet("#TagChip { border: 1px solid #9FA8DA; border-radius: 10px; padding: 2px 6px; background: #E8EAF6; }");
    connect(chip, &TagChip::removed, this, &AppWindow::removeTagFilter);
    tagFilterChipsContainer_->layout()->addWidget(chip);
}

QStringList AppWindow::currentTagFilters() const {
    QStringList out;
    if (!tagFilterChipsContainer_ || !tagFilterChipsContainer_->layout()) {
        return out;
    }
    const QLayout* layout = tagFilterChipsContainer_->layout();
    for (int i = 0; i < layout->count(); ++i) {
        QWidget* w = layout->itemAt(i)->widget();
        TagChip* chip = qobject_cast<TagChip*>(w);
        if (chip) {
            out << chip->text();
        }
    }
    return out;
}

void AppWindow::removeTagFilter(const QString& tag) {
    Q_UNUSED(tag);
    loadNotes(searchBox_->text());
}

QStringList AppWindow::allTags() const {
    QStringList out;
    QSqlQuery q("SELECT name FROM tags ORDER BY name ASC");
    while (q.next()) {
        out << q.value(0).toString();
    }
    return out;
}
