#ifndef PROFILEVIEW_H
#define PROFILEVIEW_H

#include <QWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include "favoritespanel.h" // Обязательно подключаем панель избранного
#include "bouquetspanel.h"
#include "notespanel.h"

class ProfileView : public QWidget {
    Q_OBJECT
public:
    explicit ProfileView(QWidget *parent = nullptr);
    void setCurrentUser(int userId); // Метод для получения ID от AppWindow
    void refresh() { if(m_favPanel) m_favPanel->loadFavorites(); }

signals:
    void goToCatalogRequested(); // Сигнал "проброса" для AppWindow
    void goToConstructorRequested();

private:
    void setupUI();
    QWidget* createSidebar(); // <--- Он должен быть здесь
    QPushButton* createMenuButton(const QString &text, const QString &iconPath, bool checked = false);

    // Секция приватных переменных (Gatekeeper ошибок "undeclared identifier")
    QButtonGroup *m_menuGroup;
    QStackedWidget *m_contentStack;
    FavoritesPanel *m_favPanel; // Объявляем указатель здесь
    int m_currentUserId;        // Объявляем переменную здесь
    BouquetsPanel *m_bouquetsPanel;
    NotesPanel *m_notesPanel; // ДОБАВИТЬ СЮДА
};

#endif