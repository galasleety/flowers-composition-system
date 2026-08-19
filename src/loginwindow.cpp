#include "loginwindow.h"
#include "errordialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QLinearGradient>
#include <QLocale>
#include <QInputMethod>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QPushButton>
#include <QScreen>
#include <QGuiApplication>
#include <QTimer>
#include <QMouseEvent>
#include <QDebug>
#include <QPropertyAnimation>
#include <QSql>
#include <QCryptographicHash>
#include <QSqlQuery>
#include <QSqlError>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent),
    m_gradientShift(0.0),
    m_gradientTimer(nullptr),
    m_opacityEffect(nullptr)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);

    setFixedSize(410, 390);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(18);
    shadow->setColor(QColor(0, 0, 0, 100));
    shadow->setOffset(0, 6);
    setGraphicsEffect(shadow);

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }

    setupUI();

    m_gradientTimer = new QTimer(this);
    m_gradientTimer->setInterval(40);
    connect(m_gradientTimer, &QTimer::timeout, this, &LoginWindow::onGradientTimeout);
    m_gradientTimer->start();

    QInputMethod *inputMethod = QGuiApplication::inputMethod();
    if (inputMethod) {
        connect(inputMethod, &QInputMethod::localeChanged, this, &LoginWindow::updateLanguageLabel);
    }

    updateLanguageLabel();
}

LoginWindow::~LoginWindow() {}

// --- Плавное появление окна ---
void LoginWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (!m_opacityEffect) {
        m_opacityEffect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(m_opacityEffect);
    }
    m_opacityEffect->setOpacity(0.0);

    QPropertyAnimation *anim = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    anim->setDuration(300); // Золотая середина: достаточно быстро, но плавно
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->start(QPropertyAnimation::DeleteWhenStopped);
}

// --- Плавное исчезновение окна при нажатии на крестик ---
// --- Плавное исчезновение окна при нажатии на крестик ---
void LoginWindow::fadeOutAndClose()
{
    emit closingStarted(); // Сообщаем главному окну, что начали закрываться

    if (!m_opacityEffect) {
        close();
        return;
    }

    QPropertyAnimation *anim = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    anim->setDuration(250);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::InQuad);
    connect(anim, &QPropertyAnimation::finished, this, &LoginWindow::close);
    anim->start(QPropertyAnimation::DeleteWhenStopped);
}

void LoginWindow::onLoginClicked()
{
    QString name = usernameOrEmailField->text().trimmed();
    QString pass = passwordField->text();

    if (name.isEmpty() || pass.isEmpty()) {
        ErrorDialog err("Ошибка", "Заполните все поля.", this);
        err.exec();
        return;
    }

    // Хешируем пароль
    QString hashed = QString(QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Sha256).toHex());

    QSqlQuery query;
    // ВАЖНО: заменил user_id на id, как ты и просила
    if (!query.prepare("SELECT id, username, role FROM users WHERE username = :u AND password_hash = :p")) {
        qDebug() << "ОШИБКА SQL (Prepare):" << query.lastError().text();
        return;
    }

    query.bindValue(":u", name);
    query.bindValue(":p", hashed);

    if (query.exec()) {
        if (query.next()) {
            int userId = query.value(0).toInt(); // Это наше 'id'
            QString user = query.value(1).toString();
            QString role = query.value(2).toString();

            qDebug() << "Вход выполнен! ID:" << userId;
            emit loginSuccessful(userId, user, role);
            fadeOutAndClose();
        } else {
            ErrorDialog err("Ошибка", "Неверное имя или пароль.", this);
            err.exec();
        }
    } else {
        qDebug() << "ОШИБКА SQL (Exec):" << query.lastError().text();
    }
}

void LoginWindow::onGradientTimeout()
{
    m_gradientShift += 0.005;
    if (m_gradientShift >= 1.0) m_gradientShift -= 1.0;
    update();
}

void LoginWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int cornerRadius = 20;

    QPointF a( width() * 0.15 - m_gradientShift * width(), -height() * 0.3 );
    QPointF b( width() * 0.85 + m_gradientShift * width(), height() * 1.2 );
    QLinearGradient gradient(a, b);

    gradient.setColorAt(0.00, QColor("#FFFFFF"));
    gradient.setColorAt(0.30, QColor("#FFF9FB"));
    gradient.setColorAt(0.70, QColor("#FFF4F8"));
    gradient.setColorAt(1.00, QColor("#FFF6F9"));

    painter.setBrush(QBrush(gradient));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), cornerRadius, cornerRadius);

    QLinearGradient sheen(0, 0, width(), height());
    sheen.setColorAt(0.0, QColor(255,255,255,30));
    sheen.setColorAt(0.6, QColor(255,255,255,10));
    sheen.setColorAt(1.0, QColor(0,0,0,8));
    painter.setBrush(sheen);
    painter.drawRoundedRect(rect().adjusted(1,1,-1,-1), cornerRadius-1, cornerRadius-1);
}

void LoginWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(15);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addSpacing(10);

    QLabel *titleLabel = new QLabel("Вход в аккаунт", this);
    titleLabel->setStyleSheet(
        "font-family: \"Century Gothic\", Futura, sans-serif;"
        "font-size: 26px;"
        "font-weight: bold;"
        "color: #444444;"
        );
    topLayout->addWidget(titleLabel);

    topLayout->addStretch(1);

    languageLabel = new QLabel(this);
    languageLabel->setStyleSheet(
        "font-size: 14px;"
        "color: #888888;"
        "padding: 5px 10px;"
        "border: 1px solid #D0D0D0;"
        "border-radius: 10px;"
        "background-color: #F9F9F9;"
        );
    languageLabel->setAlignment(Qt::AlignCenter);
    languageLabel->setFixedSize(50, 30);
    languageLabel->setCursor(Qt::PointingHandCursor);

    topLayout->addWidget(languageLabel);
    topLayout->addSpacing(5);

    closeButton = new QPushButton("×", this);
    closeButton->setFixedSize(30, 30);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #888888; border: none; font-size: 30px; font-weight: 300; }"
        "QPushButton:hover { color: #F8C3CD; }"
        );
    topLayout->addWidget(closeButton);
    mainLayout->addLayout(topLayout);

    // ПОДКЛЮЧАЕМ КРЕСТИК К АНИМАЦИИ ЗАТУХАНИЯ
    connect(closeButton, &QPushButton::clicked, this, &LoginWindow::fadeOutAndClose);

    QString fieldStyle = QString(
        "QLineEdit { background-color: #F9F9F9; color: #333333; border: 1px solid #D0D0D0; border-radius: 12px; padding: 15px 15px; font-size: 16px; font-family: \"Century Gothic\", Futura, sans-serif; }"
        "QLineEdit:focus { border: 2px solid #F8C3CD; background-color: #FFFFFF; }"
        );

    usernameOrEmailField = new QLineEdit(this);
    usernameOrEmailField->setPlaceholderText("Имя пользователя");
    usernameOrEmailField->setStyleSheet(fieldStyle);

    passwordField = new QLineEdit(this);
    passwordField->setPlaceholderText("Пароль");
    passwordField->setEchoMode(QLineEdit::Password);
    passwordField->setStyleSheet(fieldStyle);

    mainLayout->addWidget(usernameOrEmailField);
    mainLayout->addWidget(passwordField);
    mainLayout->addSpacing(20);

    QString primaryButtonStyle = QString(
        "QPushButton { background-color: #F8C3CD; color: #333333; border: 1px solid #F8C3CD; border-radius: 20px; padding: 12px 25px; font-size: 17px; font-weight: 500; font-family: \"Century Gothic\", Futura, sans-serif; }"
        "QPushButton:hover { background-color: #FFAAAA; border: 1px solid #FFAAAA; color: #111111; }"
        "QPushButton:pressed { background-color: #E0B0B6; border: 1px solid #E0B0B6; }"
        );

    loginButton = new QPushButton("ВОЙТИ", this);
    loginButton->setStyleSheet(primaryButtonStyle);
    loginButton->setFixedHeight(50);
    loginButton->setFixedWidth(170);
    loginButton->setCursor(Qt::PointingHandCursor);

    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);

    mainLayout->addWidget(loginButton, 0, Qt::AlignHCenter);
}

void LoginWindow::updateLanguageLabel()
{
    QString langCode = "EN";
    QInputMethod *inputMethod = QGuiApplication::inputMethod();
    QLocale currentLocale = inputMethod ? inputMethod->locale() : QLocale::system();
    QString bcp47 = currentLocale.bcp47Name().toLower();

    if (bcp47.startsWith("ru")) langCode = "RU";
    else if (bcp47.startsWith("en")) langCode = "EN";
    else langCode = bcp47.mid(0, 2).toUpper();

    if (languageLabel) languageLabel->setText(langCode);
}

void LoginWindow::showLanguageMenu()
{
    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu {  border: 1px solid #D0D0D0;  border-radius: 10px;  background-color: #FFFFFF;  padding: 5px;  font-family: \"Century Gothic\", Futura, sans-serif; }"
        "QMenu::item {  padding: 8px 15px;  border-radius: 7px;  color: #333333; }"
        "QMenu::item:selected {  background-color: #F8C3CD;  color: #111111; }"
        );

    menu.addAction("Русский (RU)")->setData("ru");
    menu.addAction("English (EN)")->setData("en");

    connect(&menu, &QMenu::triggered, this, &LoginWindow::handleLanguageMenuAction);
    QPoint pos = mapToGlobal(languageLabel->geometry().bottomLeft());
    menu.exec(pos);
}

void LoginWindow::handleLanguageMenuAction(QAction *action)
{
    QString langCode = action->data().toString();
    if (!langCode.isEmpty()) languageLabel->setText(langCode.toUpper());
}

void LoginWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && languageLabel->geometry().contains(event->pos())) {
        showLanguageMenu();
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void LoginWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void LoginWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LocaleChange || event->type() == QEvent::LanguageChange) {
        updateLanguageLabel();
    }
}