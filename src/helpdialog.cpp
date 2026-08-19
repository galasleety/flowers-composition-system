#include "helpdialog.h"
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QVariantAnimation> // НОВОЕ: Для плавной и безглючной анимации оверлея
#include <QScrollArea>

HelpDialog::HelpDialog(Mode mode, QWidget *parent)
    : QDialog(parent), m_mode(mode)
{
    // Добавляем флаг Qt::NoDropShadowWindowHint — он убивает системную тень!
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(700, 500);

    setupUI();

    if (m_mode == UserMode) setupUserContent();
    else setupAdminContent();

    if (m_menuGroup->button(0)) {
        m_menuGroup->button(0)->setChecked(true);
    }
}

// =======================================================
// ИДЕАЛЬНАЯ АНИМАЦИЯ ОВЕРЛЕЯ БЕЗ МЕРЦАНИЙ
// =======================================================
void HelpDialog::execWithOverlay(Mode mode, QWidget *parent) {
    if (!parent) return;

    // 1. Создаем статичный оверлей (без анимации, чтобы экран не мигал черным)
    QWidget *overlay = new QWidget(parent->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);"); // Полупрозрачный черный
    overlay->resize(parent->window()->size());
    overlay->show();

    // 2. Создаем само окно
    HelpDialog dialog(mode, parent->window());
    dialog.move(parent->window()->geometry().center() - dialog.rect().center());
    dialog.setWindowOpacity(0.0); // Делаем прозрачным для старта

    // 3. Плавное появление ТОЛЬКО маленького окна
    QPropertyAnimation *fadeIn = new QPropertyAnimation(&dialog, "windowOpacity");
    fadeIn->setDuration(150); // Очень быстрая и приятная анимация
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

    // 4. Ждем закрытия
    dialog.exec();

    // 5. Как только закрыли - мгновенно удаляем оверлей
    overlay->deleteLater();
}

// Простое плавное исчезновение при нажатии на крестик
void HelpDialog::fadeOutAndClose() {
    QPropertyAnimation *fadeOut = new QPropertyAnimation(this, "windowOpacity");
    fadeOut->setDuration(150);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    // Как только окно растворится — закрываем диалог
    connect(fadeOut, &QPropertyAnimation::finished, this, &QDialog::reject);
    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}
void HelpDialog::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0,0, 0, 0);
    mainLayout->setSpacing(0);

    QFrame *leftPanel = new QFrame(this);
    leftPanel->setFixedWidth(220);
    leftPanel->setStyleSheet("QFrame { background-color: #FFF0F3; border-top-left-radius: 20px; border-bottom-left-radius: 20px; }");

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(15, 30, 15, 30);
    leftLayout->setSpacing(10);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    QLabel *iconLbl = new QLabel("❔", leftPanel);
    iconLbl->setStyleSheet("font-size: 24px; background: transparent;");
    QLabel *titleLbl = new QLabel("СПРАВКА", leftPanel);
    titleLbl->setStyleSheet("font-family: 'Century Gothic'; font-size: 18px; font-weight: bold; color: #D86B7A; letter-spacing: 1px; background: transparent;");
    titleLayout->addWidget(iconLbl);
    titleLayout->addWidget(titleLbl);
    titleLayout->addStretch();
    leftLayout->addLayout(titleLayout);

    leftLayout->addSpacing(20);

    m_menuGroup = new QButtonGroup(this);
    m_menuGroup->setExclusive(true);
    connect(m_menuGroup, &QButtonGroup::idToggled, this, &HelpDialog::onMenuToggled);

    m_menuLayout = new QVBoxLayout();
    m_menuLayout->setSpacing(8);
    leftLayout->addLayout(m_menuLayout);
    leftLayout->addStretch();

    QFrame *rightPanel = new QFrame(this);
    rightPanel->setStyleSheet("QFrame { background-color: #FFFFFF; border-top-right-radius: 20px; border-bottom-right-radius: 20px; }");

    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(30, 20, 20, 30);
    rightLayout->setSpacing(15);

    QHBoxLayout *topRightLayout = new QHBoxLayout();
    topRightLayout->addStretch();
    QPushButton *closeBtn = new QPushButton("×", rightPanel);
    closeBtn->setFixedSize(30, 30);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: transparent; border: 1px solid #EAEAEA; border-radius: 15px; font-family: 'Century Gothic'; font-size: 18px; color: #555; } "
        "QPushButton:hover { background: #FFF0F3; color: #D86B7A; border-color: #D86B7A; }"
        );

    connect(closeBtn, &QPushButton::clicked, this, &HelpDialog::fadeOutAndClose);

    topRightLayout->addWidget(closeBtn);
    rightLayout->addLayout(topRightLayout);

    m_contentStack = new QStackedWidget(rightPanel);
    m_contentStack->setStyleSheet("background: transparent; border: none;");
    rightLayout->addWidget(m_contentStack, 1);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel, 1);
}

QPushButton* HelpDialog::createMenuButton(const QString &text) {
    QPushButton *btn = new QPushButton(text);
    btn->setCheckable(true);
    btn->setFixedHeight(42);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton { "
        "  background: transparent; border: none; border-radius: 12px; "
        "  text-align: left; padding-left: 20px; "
        "  font-family: 'Century Gothic'; font-size: 14px; color: #666; "
        "} "
        "QPushButton:hover { background: rgba(216, 107, 122, 0.1); color: #333; } "
        "QPushButton:checked { background: #D86B7A; color: #FFFFFF; font-weight: bold; }"
        );
    return btn;
}

QWidget* HelpDialog::createContentPage(const QString &title, const QString &contentHtml) {
    QWidget *page = new QWidget();
    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; } "
        "QScrollBar:vertical { border: none; background: transparent; width: 6px; margin: 0px; } "
        "QScrollBar::handle:vertical { background: #E0E0E0; border-radius: 3px; min-height: 30px; } "
        "QScrollBar::handle:vertical:hover { background: #D0D0D0; } "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; } "
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        );

    QWidget *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout *layout = new QVBoxLayout(scrollContent);
    layout->setContentsMargins(0, 0, 15, 0);
    layout->setSpacing(15);
    layout->setAlignment(Qt::AlignTop);

    QLabel *titleLbl = new QLabel(title, scrollContent);
    titleLbl->setStyleSheet("font-family: 'Century Gothic'; font-size: 26px; font-weight: bold; color: #222;");
    layout->addWidget(titleLbl);

    QLabel *contentLbl = new QLabel(contentHtml, scrollContent);
    contentLbl->setWordWrap(true);
    contentLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    contentLbl->setStyleSheet("QLabel { font-family: 'Century Gothic'; font-size: 14px; color: #555; line-height: 1.5; }");
    layout->addWidget(contentLbl, 1);

    scrollArea->setWidget(scrollContent);
    pageLayout->addWidget(scrollArea);

    return page;
}

void HelpDialog::setupUserContent() {
    QStringList menuItems = {"О приложении", "Каталог", "Конструктор", "Мой профиль"};
    for (int i = 0; i < menuItems.size(); ++i) {
        QPushButton *btn = createMenuButton(menuItems[i]);
        m_menuGroup->addButton(btn, i);
        m_menuLayout->addWidget(btn);
    }

    QString aboutHtml = R"(
        <p>Добро пожаловать в <b>Флористическую Систему</b> — ваш персональный цифровой помощник для создания идеальных цветочных композиций.</p>
        <h3 style='color: #D86B7A; font-weight: bold; margin-top: 15px; font-size: 16px;'>Что умеет приложение?</h3>
        <ul style='margin-left: -20px; margin-top: 5px; margin-bottom: 10px;'>
            <li><b>Исследовать:</b> Огромная база цветов, упаковки и аксессуаров со всеми характеристиками.</li>
            <li><b>Творить:</b> Умный визуальный конструктор букетов, который следит за бюджетом.</li>
            <li><b>Анализировать:</b> Встроенный интеллект подскажет, если цветы не сочетаются по сезону, эстетике или выделяют вредные друг для друга вещества.</li>
            <li><b>Сохранять:</b> Все ваши шедевры сохраняются в профиле как готовые «рецепты».</li>
        </ul>
        <p>Программа создана, чтобы избавить вас от рутины и позволить сфокусироваться на чистом творчестве!</p>
    )";
    m_contentStack->addWidget(createContentPage("О приложении", aboutHtml));

    QString catalogHtml = R"(
        <p>Раздел <b>Каталог</b> — это интерактивная энциклопедия всех доступных материалов.</p>
        <h3 style='color: #D86B7A; font-weight: bold; margin-top: 15px; font-size: 16px;'>Как пользоваться:</h3>
        <ul style='margin-left: -20px; margin-top: 5px;'>
            <li><b>Навигация:</b> Используйте меню слева для переключения между цветами, лентами, коробками и крафтом.</li>
            <li><b>Карточка товара:</b> Нажмите на любой элемент, чтобы увидеть его детальные характеристики: длину стебля, размер бутона, стойкость в днях и уровень пыльцы.</li>
            <li><b>Вариации:</b> Многие цветы и ленты доступны в разных оттенках. В карточке товара вы можете переключаться между ними, чтобы увидеть реальные фото.</li>
        </ul>
        <p><i>Подсказка:</i> Используйте строку поиска сверху, чтобы быстро найти нужный сорт!</p>
    )";
    m_contentStack->addWidget(createContentPage("Справочник каталога", catalogHtml));

    QString constructorHtml = R"(
        <p><b>Конструктор</b> — это сердце приложения, где рождаются ваши композиции.</p>
        <h3 style='color: #D86B7A; font-weight: bold; margin-top: 15px; font-size: 16px;'>Шаги создания букета:</h3>
        <ul style='margin-left: -20px; margin-top: 5px;'>
            <li><b>Добавление:</b> Выбирайте компоненты из панели справа и переносите их на холст.</li>
            <li><b>Настройка:</b> Указывайте точное количество каждого цветка. Система автоматически пересчитает общую стоимость.</li>
            <li><b>Проверка совместимости:</b> Если вы добавите конфликтующие цветы (например, те, что быстро вянут вместе), система выдаст предупреждение.</li>
            <li><b>Упаковка:</b> Не забудьте добавить финальный штрих — бумагу, коробку или ленту.</li>
        </ul>
        <p>Когда букет готов, нажмите кнопку <b>«Сохранить рецепт»</b>.</p>
    )";
    m_contentStack->addWidget(createContentPage("Как работает конструктор", constructorHtml));

    QString profileHtml = R"(
        <p>Раздел <b>Мой профиль</b> — это ваше личное рабочее пространство.</p>
        <h3 style='color: #D86B7A; font-weight: bold; margin-top: 15px; font-size: 16px;'>Ваши рецепты</h3>
        <p>Здесь отображается галерея всех созданных вами букетов. Вы можете:</p>
        <ul style='margin-left: -20px; margin-top: 5px;'>
            <li>Просматривать итоговую стоимость и состав.</li>
            <li>Открывать рецепт, чтобы посмотреть, какие именно цветы и в каком количестве были использованы.</li>
            <li>Удалять старые или неудачные композиции.</li>
        </ul>
        <p>Также здесь вы можете выйти из своего аккаунта.</p>
    )";
    m_contentStack->addWidget(createContentPage("Ваш профиль", profileHtml));
}

void HelpDialog::setupAdminContent() {
    QStringList menuItems = {"Управление", "Каталог", "Справочники"};
    for (int i = 0; i < menuItems.size(); ++i) {
        QPushButton *btn = createMenuButton(menuItems[i]);
        m_menuGroup->addButton(btn, i);
        m_menuLayout->addWidget(btn);
    }

    QString manageHtml = R"(
        <p>Добро пожаловать в <b>Панель Администратора</b>! Вы имеете полный контроль над системой.</p>
        <h3 style='color: #D86B7A; font-weight: bold; margin-top: 15px; font-size: 16px;'>Пользователи</h3>
        <p>В этом разделе вы управляете доступом:</p>
        <ul style='margin-left: -20px; margin-top: 5px;'>
            <li><b>Просмотр:</b> Список всех зарегистрированных флористов.</li>
            <li><b>Редактирование:</b> Возможность изменить данные пользователя.</li>
            <li><b>Безопасность:</b> Сброс паролей или блокировка подозрительных аккаунтов.</li>
        </ul>
    )";
    m_contentStack->addWidget(createContentPage("Управление системой", manageHtml));

    QString adminCatalogHtml = R"(
        <p>Раздел <b>Каталог</b> позволяет вам наполнять базу данных новыми товарами.</p>
        <h3 style='color: #D86B7A; font-weight: bold; margin-top: 15px; font-size: 16px;'>Редактирование товаров</h3>
        <ul style='margin-left: -20px; margin-top: 5px;'>
            <li><b>Добавление:</b> Создавайте новые цветы, упаковку и аксессуары, заполняя их характеристики (стойкость, токсичность, размер).</li>
            <li><b>Вариации:</b> Добавляйте к одному товару разные цвета (например, Роза: красная, белая, желтая) и загружайте для каждого свое фото в высоком качестве.</li>
            <li><b>Модерация:</b> Удаляйте элементы, которые больше не поставляются.</li>
        </ul>
        <p><i>Внимание:</i> Удаление товара может повлиять на сохраненные рецепты пользователей!</p>
    )";
    m_contentStack->addWidget(createContentPage("Редактирование каталога", adminCatalogHtml));

    QString dictHtml = R"(
        <p><b>Справочники</b> — это фундамент, на котором строится логика всего приложения.</p>
        <h3 style='color: #D86B7A; font-weight: bold; margin-top: 15px; font-size: 16px;'>Доступные справочники:</h3>
        <ul style='margin-left: -20px; margin-top: 5px;'>
            <li><b>Палитра цветов:</b> Создание и привязка HEX-кодов. Позволяет системе понимать, какие оттенки используются в товарах.</li>
            <li><b>Сезонность:</b> Настройка месяцев цветения. Благодаря этому конструктор сможет предупреждать пользователей о недоступности цветка зимой или летом.</li>
            <li><b>Правила и Логика:</b> Самый важный раздел! Здесь вы задаете правила совместимости.</li>
        </ul>
    )";
    m_contentStack->addWidget(createContentPage("Настройка справочников", dictHtml));
}

void HelpDialog::onMenuToggled(int id, bool checked) {
    if (checked && id >= 0 && id < m_contentStack->count()) {
        m_contentStack->setCurrentIndex(id);
    }
}
