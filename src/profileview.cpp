#include "profileview.h"
#include <bouquetspanel.h>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>

ProfileView::ProfileView(QWidget *parent)
    : QWidget(parent), m_currentUserId(-1)
{
    setupUI();
}

void ProfileView::setCurrentUser(int userId) {
    m_currentUserId = userId;

    if (m_favPanel) m_favPanel->setCurrentUser(userId);
    if (m_bouquetsPanel) m_bouquetsPanel->setCurrentUser(userId);
    if (m_notesPanel) m_notesPanel->setCurrentUser(userId); // ДОБАВИТЬ СЮДА
}

void ProfileView::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(30, 20, 30, 20);
    mainLayout->setSpacing(40);

    mainLayout->addWidget(createSidebar());

    m_contentStack = new QStackedWidget(this);
    m_contentStack->setStyleSheet("background: transparent; border: none;");

    m_favPanel = new FavoritesPanel(this);
    connect(m_favPanel, &FavoritesPanel::goToCatalogRequested, this, &ProfileView::goToCatalogRequested);

    m_bouquetsPanel = new BouquetsPanel(this);
    connect(m_bouquetsPanel, &BouquetsPanel::goToConstructorRequested, this, &ProfileView::goToConstructorRequested);

    m_notesPanel = new NotesPanel(this);

    // Добавляем по порядку ID
    m_contentStack->addWidget(m_favPanel);      // 0
    m_contentStack->addWidget(m_bouquetsPanel); // 1
    m_contentStack->addWidget(m_notesPanel);    // 2

    mainLayout->addWidget(m_contentStack, 1);

    connect(m_menuGroup, &QButtonGroup::idClicked, this, [this](int id) {
        m_contentStack->setCurrentIndex(id);
        if (id == 0) m_favPanel->loadFavorites();
        if (id == 1) m_bouquetsPanel->loadBouquets();
        if (id == 2) m_notesPanel->loadNotes();
    });
}

QWidget* ProfileView::createSidebar() {
    QFrame *panel = new QFrame(this);
    panel->setFixedWidth(280);
    panel->setStyleSheet("QFrame { background-color: #FFF0F3; border-radius: 30px; border: none; }");

    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(20, 40, 20, 30);
    layout->setSpacing(15);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QLabel *title = new QLabel("Управление\nпрофилем", panel);
    title->setStyleSheet("font-family: 'Century Gothic'; font-size: 22px; font-weight: bold; color: #333; border: none;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    layout->addSpacing(30);

    m_menuGroup = new QButtonGroup(this);
    m_menuGroup->setExclusive(true);

    // Кнопки меню с привязкой к индексам стека
    QPushButton *btnFav = createMenuButton("Избранное", "", true);
    QPushButton *btnBouquets = createMenuButton("Мои букеты", "");
    QPushButton *btnNotes = createMenuButton("Заметки", "");

    // Назначаем ID: они должны совпадать с очередностью addWidget в setupUI
    m_menuGroup->addButton(btnFav, 0);      // Избранное -> Индекс 0
    m_menuGroup->addButton(btnBouquets, 1); // Букеты -> Индекс 1
    m_menuGroup->addButton(btnNotes, 2);    // Заметки -> Индекс 2 (если создашь панель)

    layout->addWidget(btnFav);
    layout->addWidget(btnBouquets);
    layout->addWidget(btnNotes);

    layout->addStretch();

    QLabel *version = new QLabel("v1.0", panel);
    version->setStyleSheet("color: #C0C0C0; font-family: 'Century Gothic'; font-size: 12px; border: none;");
    version->setAlignment(Qt::AlignCenter);
    layout->addWidget(version);

    return panel;
}

QPushButton* ProfileView::createMenuButton(const QString &text, const QString &iconPath, bool checked) {
    QPushButton *btn = new QPushButton(text, this);
    btn->setCheckable(true);
    btn->setChecked(checked);
    btn->setFixedHeight(50);
    btn->setFixedWidth(220);
    btn->setCursor(Qt::PointingHandCursor);

    btn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent; border: none; border-radius: 15px;"
        "   text-align: center; font-family: 'Century Gothic'; font-size: 15px; color: #666;"
        "}"
        "QPushButton:hover { background-color: rgba(248, 195, 205, 40); }"
        "QPushButton:checked { background-color: #F8C3CD; color: #333; font-weight: bold; }"
        );

    return btn;
}