#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>

class AdminWindow : public QMainWindow {
    Q_OBJECT
public:
    // ДОБАВИЛИ int adminId
    explicit AdminWindow(int adminId, QWidget *parent = nullptr);

private:
    void setupUI();
    QWidget* createHeader();
    QWidget* createSidebar();
    QLabel* createSectionLabel(const QString &text);
    QPushButton* createMenuButton(const QString &text, const QString &iconPath);

    int m_adminId; // <--- ДОБАВИТЬ ЭТУ ПЕРЕМЕННУЮ ДЛЯ ХРАНЕНИЯ ID

    QWidget *m_mainWidget;
    QButtonGroup *m_menuGroup;
    QStackedWidget *m_contentStack;
};

#endif // ADMINWINDOW_H