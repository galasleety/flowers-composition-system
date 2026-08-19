#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QWidget>
#include <QString>

class AppWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Замени конструктор:
    explicit AppWindow(int userId, const QString &username, QWidget *parent = nullptr);
    ~AppWindow();

private slots:
    void onMenuButtonToggled(int id, bool checked);

private:
    void setupUI();
    QWidget* createHeader();
    QWidget* createMenuBar();

    QString m_loggedInUsername;
    int m_loggedInUserId; // На будущее для базы данных

    QWidget *m_mainWidget;
    QStackedWidget *m_contentStack;
    QButtonGroup *m_menuGroup;
};

#endif // APPWINDOW_H