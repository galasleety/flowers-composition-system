#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QGraphicsOpacityEffect>

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    // Сигнал теперь передает ТРИ параметра
    void loginSuccessful(int userId, const QString &username, const QString &role);
    void closingStarted();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onGradientTimeout();
    void onLoginClicked();
    void updateLanguageLabel();
    void showLanguageMenu();
    void handleLanguageMenuAction(QAction *action);
    void fadeOutAndClose();

private:
    void setupUI();

    qreal m_gradientShift;
    QTimer *m_gradientTimer;
    QLabel *languageLabel;
    QPushButton *closeButton;
    QLineEdit *usernameOrEmailField;
    QLineEdit *passwordField;
    QPushButton *loginButton;
    QGraphicsOpacityEffect *m_opacityEffect;
    QPoint m_dragPosition;
};

#endif