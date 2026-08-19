#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QPoint>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QShowEvent>
#include <QGraphicsOpacityEffect>

class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow();

    // Найди строку в сигнале и замени на эту:
signals:
    void registrationSuccessful(int userId, const QString &username);
    void closingStarted();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onGradientTimeout();
    void onRegisterClicked();
    void updateLanguageLabel();
    void showLanguageMenu();
    void handleLanguageMenuAction(QAction *action); // Будет реализована в cpp
    void fadeOutAndClose();

private:
    void setupUI();

    qreal m_gradientShift;
    QTimer *m_gradientTimer;

    QLabel *languageLabel;
    QPushButton *closeButton;
    QLineEdit *usernameField;
    QLineEdit *passwordField;
    QLineEdit *repeatPasswordField; // <--- ВОТ ЭТО НОВОЕ ПОЛЕ
    QPushButton *registerButton;

    QGraphicsOpacityEffect *m_opacityEffect;
    QPoint m_dragPosition;
};

#endif // REGISTERWINDOW_H