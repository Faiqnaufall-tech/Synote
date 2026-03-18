#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

class AppWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit AppWindow(QWidget* parent = nullptr);

private slots:
    void onNewNote();
    void onSaveNote();
    void onNoteSelected(QListWidgetItem* item);
    void onSearchChanged(const QString& text);
    void onSyncNow();
    void onOpenSettings();
    void onAiSummarize();
    void onAiDraft();
    void onAiTodo();
    void onAddProject();

private:
    void setupUi();
    void loadNotes(const QString& search = "");
    void loadProjects();
    void loadTagsForNote(const QString& noteId);
    void saveTagsForNote(const QString& noteId);
    QStringList parseTags(const QString& input) const;
    void clearTagChips();
    void addTagChip(const QString& tag);
    QStringList currentTags() const;
    void loadTagFilters();
    void clearTagFilterChips();
    void addTagFilterChip(const QString& tag);
    QStringList currentTagFilters() const;
    void removeTagFilter(const QString& tag);
    QStringList allTags() const;

    QListWidget* notesList_;
    QLineEdit* searchBox_;
    QListWidget* projectList_;
    QComboBox* projectFilterBox_;
    QWidget* tagFilterChipsContainer_;
    QLineEdit* tagFilterInput_;
    QCompleter* tagFilterCompleter_;
    QLabel* notesCountLabel_;
    QLineEdit* titleEdit_;
    QTextEdit* bodyEdit_;
    QComboBox* projectBox_;
    QWidget* tagChipsContainer_;
    QLineEdit* tagInput_;
    QCompleter* tagCompleter_;
    QLabel* statusLabel_;
    QPushButton* syncBtn_;
    QPushButton* aiSummaryBtn_;
    QPushButton* aiDraftBtn_;
    QPushButton* aiTodoBtn_;

    QString currentNoteId_;
};
