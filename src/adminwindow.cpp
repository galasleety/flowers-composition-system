#include "adminwindow.h"
#include "flowerspage.h"
#include "userspage.h"
#include "packagingpage.h"
#include "accessoriespage.h"
#include "rulespage.h"
#include "seasonalitypage.h"
#include "palettepage.h" // Добавили страницу палитры
#include "mainwindow.h"
#include "helpdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QIcon>
#include <QPropertyAnimation>
#include <QStackedWidget>

AdminWindow::AdminWindow(int adminId, QWidget *parent) : QMainWindow(parent), m_adminId(adminId) {
    setWindowFlags(Qt::FramelessWindowHint);
    showFullScreen();
    setStyleSheet("QMainWindow { background-color: #FFFFFF; }");

    setupUI();
}

void AdminWindow::setupUI() {
    m_mainWidget = new QWidget(this);
    setCentralWidget(m_mainWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(m_mainWidget);
    mainLayout->setContentsMargins(30, 25, 30, 30);
    mainLayout->setSpacing(40);

    // Сначала создаем сайдбар, так как нам нужен m_menuGroup для логики стека
    mainLayout->addWidget(createSidebar());

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    rightLayout->addWidget(createHeader());
    rightLayout->addSpacing(20);

    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background-color: #EAEAEA; border: none; height: 2px;");
    rightLayout->addWidget(separator);

    rightLayout->addSpacing(20);

    m_contentStack = new QStackedWidget(this);
    m_contentStack->setStyleSheet("background-color: transparent; border: none;");

    // --- ИНИЦИАЛИЗАЦИЯ СТРАНИЦ ---
    UsersPage *usersPage = new UsersPage(this);
    usersPage->setCurrentUser(m_adminId);

    FlowersPage *flowersPage = new FlowersPage(this);
    PackagingPage *packagingPage = new PackagingPage(this);
    AccessoriesPage *accessoriesPage = new AccessoriesPage(this);
    PalettePage *palettePage = new PalettePage(this); // Реальная страница вместо placeholder
    RulesPage *rulesPage = new RulesPage(this);
    SeasonalityPage *seasonalityPage = new SeasonalityPage(this);

    // --- ДОБАВЛЕНИЕ В СТЕК (СТРОГО ПО ПОРЯДКУ 0-6) ---
    m_contentStack->addWidget(usersPage);        // 0
    m_contentStack->addWidget(flowersPage);      // 1
    m_contentStack->addWidget(packagingPage);    // 2
    m_contentStack->addWidget(accessoriesPage);  // 3
    m_contentStack->addWidget(palettePage);      // 4
    m_contentStack->addWidget(rulesPage);        // 5
    m_contentStack->addWidget(seasonalityPage);  // 6

    rightLayout->addWidget(m_contentStack, 1);
    mainLayout->addLayout(rightLayout, 1);

    // --- ЛОГИКА ПЕРЕКЛЮЧЕНИЯ ---
    connect(m_menuGroup, &QButtonGroup::idClicked, this, [=](int id) {
        if (id >= 0 && id < m_contentStack->count()) {
            m_contentStack->setCurrentIndex(id);

            // Вызываем обновления данных для конкретных страниц
            switch(id) {
            case 0: usersPage->loadUsers(); break;
            case 1: flowersPage->loadFlowers(); break;
            case 2: packagingPage->loadPackaging(); break;
            case 3: accessoriesPage->loadAccessories(); break;
            case 4: palettePage->loadColors(); break; // Загрузка палитры
            case 5: rulesPage->loadRules(); break;
            case 6: seasonalityPage->loadData(); break; // Загрузка сезонности
            }
        }
    });

    m_contentStack->setCurrentIndex(0);
}

QWidget* AdminWindow::createSidebar() {
    QFrame *panel = new QFrame(this);
    panel->setFixedWidth(280);
    panel->setStyleSheet("QFrame { background-color: #FFF0F3; border-radius: 30px; border: none; }");

    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(20, 45, 20, 35);
    layout->setSpacing(12); // Расстояние между кнопками

    QLabel *logoLabel = new QLabel("FLORIST\nSYSTEM", panel);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 20px; font-weight: bold; color: #D86B7A;");
    layout->addWidget(logoLabel);

    layout->addSpacing(45);

    m_menuGroup = new QButtonGroup(this);
    m_menuGroup->setExclusive(true);

    // --- УПРАВЛЕНИЕ ---
    layout->addWidget(createSectionLabel("УПРАВЛЕНИЕ"));
    QPushButton *btnUsers = createMenuButton("Пользователи", ":/sources/icons/users.png");
    m_menuGroup->addButton(btnUsers, 0);
    layout->addWidget(btnUsers);

    layout->addSpacing(25); // Отступ между секциями

    // --- КАТАЛОГ ---
    layout->addWidget(createSectionLabel("КАТАЛОГ"));
    QPushButton *btnFlowers = createMenuButton("Цветы", ":/sources/icons/flower.png");
    QPushButton *btnPackaging = createMenuButton("Упаковка", ":/sources/icons/box.png");
    QPushButton *btnAccessories = createMenuButton("Аксессуары", ":/sources/icons/ribbon.png");

    m_menuGroup->addButton(btnFlowers, 1);
    m_menuGroup->addButton(btnPackaging, 2);
    m_menuGroup->addButton(btnAccessories, 3);

    layout->addWidget(btnFlowers);
    layout->addWidget(btnPackaging);
    layout->addWidget(btnAccessories);

    layout->addSpacing(25);

    // --- СПРАВОЧНИКИ ---
    layout->addWidget(createSectionLabel("СПРАВОЧНИКИ"));
    QPushButton *btnPalette = createMenuButton("Палитра цветов", ":/sources/icons/palette.png");
    QPushButton *btnRules = createMenuButton("Правила и Логика", ":/sources/icons/rules.png");
    QPushButton *btnSeason = createMenuButton("Сезонность", ":/sources/icons/sun.png");

    // Индексы теперь идут без пропусков
    m_menuGroup->addButton(btnPalette, 4);
    m_menuGroup->addButton(btnRules, 5);
    m_menuGroup->addButton(btnSeason, 6);

    layout->addWidget(btnPalette);
    layout->addWidget(btnRules);
    layout->addWidget(btnSeason);

    layout->addStretch();
    btnUsers->setChecked(true);

    return panel;
}

QWidget* AdminWindow::createHeader() {
    QWidget *headerWidget = new QWidget(this);
    headerWidget->setFixedHeight(40);
    headerWidget->setStyleSheet("background-color: transparent;");

    QHBoxLayout *topLayout = new QHBoxLayout(headerWidget);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(15);
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

    // Плавный выход на главное окно
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

    // --- ЦЕНТР: Заголовок ---
    QLabel *titleLabel = new QLabel("ПАНЕЛЬ АДМИНИСТРАТОРА", headerWidget);
    titleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 24px; font-weight: bold; color: #4A4A4A;");

    topLayout->addWidget(titleLabel);

    // Растяжка, чтобы сдвинуть кнопки управления вправо
    topLayout->addStretch(1);

    // --- ПРАВАЯ ЧАСТЬ: Кнопки управления ---
    QString controlBtnStyle = "QPushButton { background-color: #FFFFFF; border: 1px solid #D0D0D0; border-radius: 8px; font-weight: bold; } QPushButton:hover { background-color: #F8C3CD; color: #FFFFFF; }";

    QPushButton *helpBtn = new QPushButton("?", headerWidget);
    helpBtn->setFixedSize(35, 35);
    helpBtn->setCursor(Qt::PointingHandCursor);
    helpBtn->setStyleSheet(controlBtnStyle);
    connect(helpBtn, &QPushButton::clicked, this, [this]() {
        HelpDialog::execWithOverlay(HelpDialog::AdminMode, this);
    });

    QPushButton *closeBtn = new QPushButton("×", headerWidget);
    closeBtn->setFixedSize(35, 35);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(controlBtnStyle);

    connect(closeBtn, &QPushButton::clicked, this, [this]() {
        QPropertyAnimation *fadeOut = new QPropertyAnimation(this, "windowOpacity");
        fadeOut->setDuration(300);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        connect(fadeOut, &QPropertyAnimation::finished, this, &QMainWindow::close);
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });

    topLayout->addWidget(helpBtn);
    topLayout->addWidget(closeBtn);

    return headerWidget;
}

QLabel* AdminWindow::createSectionLabel(const QString &text) {
    QLabel *label = new QLabel(text, this);
    label->setStyleSheet("font-family: 'Century Gothic'; font-size: 10px; font-weight: bold; color: #A0A0A0; margin-left: 15px; background: transparent;");
    return label;
}

QPushButton* AdminWindow::createMenuButton(const QString &text, const QString &iconPath) {
    QPushButton *btn = new QPushButton(text, this);
    btn->setCheckable(true);
    btn->setFixedHeight(45);
    btn->setIcon(QIcon(iconPath));
    btn->setIconSize(QSize(20, 20));
    btn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent; border: none; border-radius: 12px;"
        "   text-align: left; padding-left: 15px; font-family: 'Century Gothic'; font-size: 14px; color: #555555;"
        "}"
        "QPushButton:hover { background-color: rgba(248, 195, 205, 100); }"
        "QPushButton:checked { background-color: #FFFFFF; color: #D86B7A; font-weight: bold; }"
        );
    return btn;
}