#include "appwindow.h"
#include "mainwindow.h"
#include "catalogview.h" // Подключаем наш новый каталог
#include "constructorview.h"
#include "profileview.h"
#include "helpdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScreen>
#include <QGuiApplication>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

AppWindow::AppWindow(int userId, const QString &username, QWidget *parent)
    : QMainWindow(parent), m_loggedInUsername(username), m_loggedInUserId(userId) // Просто сохраняем ID
{
    setWindowFlags(Qt::FramelessWindowHint);
    showFullScreen();
    setStyleSheet("QMainWindow { background-color: #FFFFFF; }");

    qDebug() << "AppWindow запущен для пользователя ID:" << m_loggedInUserId;

    setupUI();
}

void AppWindow::setupUI()
{
    m_mainWidget = new QWidget(this);
    setCentralWidget(m_mainWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createHeader());
    mainLayout->addWidget(createMenuBar());

    m_contentStack = new QStackedWidget(this);

    // 1. Страница каталога
    CatalogView *catalogPage = new CatalogView(this);
    if (m_loggedInUserId > 0) {
        catalogPage->setCurrentUser(m_loggedInUserId);
    }

    // 2. Страница конструктора
    ConstructorView *constructorPage = new ConstructorView(this);
    if (m_loggedInUserId > 0) {
        constructorPage->setCurrentUser(m_loggedInUserId);
    }

    // 3. Страница профиля
    ProfileView *profilePage = new ProfileView(this);

    if (m_loggedInUserId > 0) {
        profilePage->setCurrentUser(m_loggedInUserId);
    }

    // --- ПОДКЛЮЧАЕМ ПЕРЕХОД В КАТАЛОГ (Уже было) ---
    connect(profilePage, &ProfileView::goToCatalogRequested, this, [this]() {
        m_contentStack->setCurrentIndex(0); // 0 - индекс Каталога
        if (m_menuGroup && m_menuGroup->button(0)) {
            m_menuGroup->button(0)->setChecked(true);
        }
    });

    // --- НОВОЕ: ПОДКЛЮЧАЕМ ПЕРЕХОД В КОНСТРУКТОР ---
    connect(profilePage, &ProfileView::goToConstructorRequested, this, [this]() {
        m_contentStack->setCurrentIndex(1); // 1 - индекс Конструктора

        // Визуально переключаем вкладку в верхнем меню
        if (m_menuGroup && m_menuGroup->button(1)) {
            m_menuGroup->button(1)->setChecked(true);
        }
    });

    m_contentStack->addWidget(catalogPage);     // index 0
    m_contentStack->addWidget(constructorPage); // index 1
    m_contentStack->addWidget(profilePage);     // index 2

    mainLayout->addWidget(m_contentStack, 1);
}

AppWindow::~AppWindow() {}


QWidget* AppWindow::createHeader()
{
    QWidget *headerWidget = new QWidget(this);
    headerWidget->setFixedHeight(60);
    headerWidget->setStyleSheet("background-color: #FFFFFF;");

    QVBoxLayout *headerMainLayout = new QVBoxLayout(headerWidget);
    headerMainLayout->setContentsMargins(15, 15, 15, 0);
    headerMainLayout->setSpacing(15);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setAlignment(Qt::AlignVCenter);

    // --- ЛЕВАЯ ЧАСТЬ: Кнопка "Вернуться Назад" ---
    QPushButton *backBtn = new QPushButton("Вернуться Назад", headerWidget);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setFixedHeight(35);
    backBtn->setStyleSheet(
        "QPushButton { "
        "  background-color: #FFFFFF; "
        "  color: #333333; "
        "  border: 1px solid #D0D0D0; "
        "  border-radius: 8px; "
        "  padding: 0 15px; "
        "  font-family: 'Century Gothic'; "
        "  font-size: 13px; "
        "} "
        "QPushButton:hover { background-color: #F9F9F9; }"
        );

    // --- Плавный выход без черного/прозрачного экрана ---
    connect(backBtn, &QPushButton::clicked, this, [this]() {
        MainWindow *mainWin = new MainWindow();
        mainWin->show();

        QPropertyAnimation *fadeOut = new QPropertyAnimation(this, "windowOpacity");
        fadeOut->setDuration(300);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        fadeOut->setEasingCurve(QEasingCurve::InOutQuad);

        connect(fadeOut, &QPropertyAnimation::finished, this, &QMainWindow::close);

        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });

    topLayout->addWidget(backBtn);
    topLayout->addStretch(1);

    // --- ЦЕНТР: Иконка + Заголовок ---
    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(15);

    QLabel *circleIcon = new QLabel(headerWidget);
    circleIcon->setFixedSize(30, 30);
    circleIcon->setStyleSheet("background-color: #F8C3CD; border-radius: 15px;"); // Розовый круг

    QLabel *titleLabel = new QLabel("КОМПОНЕНТЫ И КОМПОЗИЦИИ ЦВЕТОЧНЫХ БУКЕТОВ", headerWidget);
    titleLabel->setStyleSheet(
        "font-family: 'Century Gothic'; "
        "font-size: 22px; "
        "color: #333333; "
        "letter-spacing: 1px;"
        );

    titleLayout->addWidget(circleIcon);
    titleLayout->addWidget(titleLabel);
    topLayout->addLayout(titleLayout);

    topLayout->addStretch(1);

    // --- ПРАВАЯ ЧАСТЬ: Кнопки '?' и 'X' ---
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(10);

    QString controlBtnStyle =
        "QPushButton { "
        "  background-color: #FFFFFF; "
        "  color: #333333; "
        "  border: 1px solid #D0D0D0; "
        "  border-radius: 8px; "
        "  font-family: 'Century Gothic'; "
        "  font-size: 16px; "
        "  font-weight: bold; "
        "} "
        "QPushButton:hover { background-color: #F8C3CD; color: #FFFFFF; border: none; }";

    QPushButton *helpBtn = new QPushButton("?", headerWidget);
    helpBtn->setFixedSize(35, 35);
    helpBtn->setCursor(Qt::PointingHandCursor);
    helpBtn->setStyleSheet(controlBtnStyle);
    connect(helpBtn, &QPushButton::clicked, this, [this]() {
        HelpDialog::execWithOverlay(HelpDialog::UserMode, this);
    });

    QPushButton *closeBtn = new QPushButton("×", headerWidget);
    closeBtn->setFixedSize(35, 35);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(controlBtnStyle);
    connect(closeBtn, &QPushButton::clicked, this, &QMainWindow::close);

    controlsLayout->addWidget(helpBtn);
    controlsLayout->addWidget(closeBtn);
    topLayout->addLayout(controlsLayout);

    headerMainLayout->addLayout(topLayout);

    // --- РОЗОВАЯ ЛИНИЯ ПОД ШАПКОЙ ---
    QFrame *separator = new QFrame(headerWidget);
    separator->setFixedHeight(3);
    separator->setStyleSheet("background-color: #E6A8B5; border: none;"); // Цвет линии
    headerMainLayout->addWidget(separator);

    return headerWidget;
}

QWidget* AppWindow::createMenuBar()
{
    QWidget *menuWidget = new QWidget(this);
    menuWidget->setFixedHeight(70);
    menuWidget->setStyleSheet("background-color: #FFFFFF;");

    QHBoxLayout *menuLayout = new QHBoxLayout(menuWidget);
    menuLayout->setContentsMargins(40, 10, 40, 10);
    menuLayout->setAlignment(Qt::AlignVCenter);

    menuLayout->addStretch(1);

    // --- ЦЕНТР: Кнопки навигации ---
    QHBoxLayout *tabsLayout = new QHBoxLayout();
    tabsLayout->setSpacing(30);

    m_menuGroup = new QButtonGroup(this);
    m_menuGroup->setExclusive(true);

    QStringList tabNames = {"Каталог", "Конструктор", "Мой профиль"};

    QString tabStyle =
        "QPushButton { "
        "  background-color: transparent; "
        "  color: #777777; "
        "  border: none; "
        "  font-family: 'Century Gothic'; "
        "  font-size: 16px; "
        "  padding: 10px 20px; "
        "  min-width: 140px; "
        "} "
        "QPushButton:hover { color: #333333; } "
        "QPushButton:checked { "
        "  color: #333333; "
        "  border-bottom: 3px solid #D86B7A; "
        "  font-weight: bold; "
        "}";

    for (int i = 0; i < tabNames.size(); ++i) {
        QPushButton *tabBtn = new QPushButton(tabNames[i], menuWidget);
        tabBtn->setCheckable(true);
        tabBtn->setCursor(Qt::PointingHandCursor);
        tabBtn->setStyleSheet(tabStyle);

        m_menuGroup->addButton(tabBtn, i);
        tabsLayout->addWidget(tabBtn);

        if (i == 0) tabBtn->setChecked(true); // По умолчанию выбран "Каталог"
    }

    connect(m_menuGroup, &QButtonGroup::idToggled, this, &AppWindow::onMenuButtonToggled);

    menuLayout->addLayout(tabsLayout);
    menuLayout->addStretch(1);

    // --- ПРАВАЯ ЧАСТЬ: Иконка пользователя и имя ---
    QHBoxLayout *userLayout = new QHBoxLayout();
    userLayout->setSpacing(10);
    userLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel *userIcon = new QLabel(menuWidget);
    userIcon->setFixedSize(30, 30);
    userIcon->setAlignment(Qt::AlignCenter);

    QPixmap avatar(":/sources/icons/ikonka.svg"); // ПУТЬ К ТВОЕЙ ИКОНКЕ
    if (!avatar.isNull()) {
        userIcon->setPixmap(avatar.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    QLabel *userNameLabel = new QLabel(m_loggedInUsername, menuWidget);
    userNameLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 15px; color: #333333; font-weight: bold;");

    userLayout->addWidget(userIcon);
    userLayout->addWidget(userNameLabel);

    menuLayout->addLayout(userLayout);

    return menuWidget;
}

void AppWindow::onMenuButtonToggled(int id, bool checked)
{
    if (checked && m_contentStack) {
        if (id >= 0 && id < m_contentStack->count()) {
            m_contentStack->setCurrentIndex(id);

            // --- ДОБАВЬ ЭТОТ БЛОК ---
            // Если переключились на вкладку "Мой профиль" (индекс 2)
            if (id == 2) {
                // Пытаемся найти виджет профиля в стеке и обновить его
                ProfileView *pv = qobject_cast<ProfileView*>(m_contentStack->widget(id));
                if (pv) {
                    pv->refresh();
                    qDebug() << "[UI] Вкладка профиля обновлена";
                }
            }
            // ------------------------
        }
    }
}