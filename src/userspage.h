#ifndef USERSPAGE_H
#define USERSPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QTableWidget>

class UsersPage : public QWidget {
    Q_OBJECT
public:
    explicit UsersPage(QWidget *parent = nullptr);

    void setCurrentUser(int adminId);
    void loadUsers();

private slots:
    void onSearchTextChanged(const QString &text);
    void onUserSelected();
    void onDeleteUserClicked();
    void onEditUserClicked();

    // Слоты для кнопок удаления
    void onDeleteNoteClicked();
    void onDeleteBouquetClicked();
    void onDeleteFavClicked();

    // НОВЫЕ СЛОТЫ: Сохранение данных при редактировании ячейки (двойной клик)
    void onNoteItemChanged(QTableWidgetItem *item);
    void onBouquetItemChanged(QTableWidgetItem *item);

private:
    void setupUI();

    QWidget* createNotesTab();
    QWidget* createBouquetsTab();
    QWidget* createFavoritesTab();

    void loadUserNotes(int userId);
    void loadUserBouquets(int userId);
    void loadUserFavorites(int userId);

    int m_currentUserId;

    QLineEdit *m_searchEdit;
    QListWidget *m_usersList;

    QWidget *m_rightPanel;
    QFrame *m_userInfoCard;
    QLabel *m_userIconLabel;

    QStackedWidget *m_textStack;

    QLabel *m_userNameLabel;
    QLabel *m_userIdViewLabel;
    QLabel *m_userRoleViewLabel;

    QLineEdit *m_nameEditField;
    QLabel *m_userIdEditLabel;
    QComboBox *m_roleEditCombo;

    QPushButton *m_btnEditUser;
    QPushButton *m_btnDeleteUser;

    bool m_isEditing;

    QWidget *m_bottomContainer;
    QButtonGroup *m_tabsGroup;
    QStackedWidget *m_lowerStack;

    QTableWidget *m_notesTable;
    QTableWidget *m_bouquetsTable;
    QTableWidget *m_favsTable;

    QPushButton *m_btnDeleteNote;
    QPushButton *m_btnDeleteBouquet;
    QPushButton *m_btnDeleteFav;
};

#endif // USERSPAGE_H