#ifndef NOTESPANEL_H
#define NOTESPANEL_H

#include <QWidget>
#include <QGridLayout>
#include <QShowEvent>
#include <QLabel>

class NotesPanel : public QWidget {
    Q_OBJECT
public:
    explicit NotesPanel(QWidget *parent = nullptr);
    void setCurrentUser(int userId);
    void loadNotes();

protected:
    void showEvent(QShowEvent *event) override;

private:
    void setupUI();
    void openNoteDialog(int noteId = -1); // -1 означает "Создать новую", иначе "Редактировать"
    void deleteNote(int noteId);

    int m_userId;
    QGridLayout *m_grid;
    QLabel *m_countLabel;
};

#endif // NOTESPANEL_H