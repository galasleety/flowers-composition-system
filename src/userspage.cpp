#include "userspage.h"
#include "errordialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QFrame>
#include <QIcon>
#include <QDebug>
#include <QScrollBar>
#include <QFont>
#include <QHeaderView>

UsersPage::UsersPage(QWidget *parent) : QWidget(parent), m_isEditing(false), m_currentUserId(-1) {
    setStyleSheet("background-color: #FFFFFF;");
    setupUI();
}

void UsersPage::setCurrentUser(int adminId) {
    m_currentUserId = adminId;
    loadUsers();
}

void UsersPage::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ==========================================
    // ЛЕВАЯ ЧАСТЬ
    // ==========================================
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setFixedWidth(290);

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(25, 25, 15, 25);
    leftLayout->setSpacing(15);

    QLabel *titleLabel = new QLabel("ПОЛЬЗОВАТЕЛИ", leftPanel);
    titleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; color: #333333; letter-spacing: 1px; border: none;");
    leftLayout->addWidget(titleLabel);

    m_searchEdit = new QLineEdit(leftPanel);
    m_searchEdit->setPlaceholderText("Поиск...");
    m_searchEdit->setFixedHeight(38);
    m_searchEdit->addAction(QIcon(":/sources/icons/search.png"), QLineEdit::LeadingPosition);
    m_searchEdit->setStyleSheet(
        "QLineEdit { border: 1px solid #D86B7A; border-radius: 19px; padding-left: 10px; padding-right: 15px; font-family: 'Century Gothic'; font-size: 13px; color: #333333; background-color: #FFFFFF; }"
        );
    connect(m_searchEdit, &QLineEdit::textChanged, this, &UsersPage::onSearchTextChanged);
    leftLayout->addWidget(m_searchEdit);

    m_usersList = new QListWidget(leftPanel);
    m_usersList->setFrameShape(QFrame::NoFrame);
    m_usersList->setFocusPolicy(Qt::NoFocus);
    m_usersList->setIconSize(QSize(30, 30));
    m_usersList->setStyleSheet(
        "QListWidget { background: transparent; outline: none; border: none; }"
        "QListWidget::item { height: 50px; border-radius: 12px; color: #555555; padding-left: 5px; }"
        "QListWidget::item:hover { background-color: #FFF0F3; color: #333333; }"
        "QListWidget::item:selected { background-color: #FFF0F3; color: #000000; }"
        "QScrollBar:vertical { border: none; background: transparent; width: 4px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #DCDCDC; border-radius: 2px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #D86B7A; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        );
    connect(m_usersList, &QListWidget::itemSelectionChanged, this, &UsersPage::onUserSelected);
    leftLayout->addWidget(m_usersList);

    QFrame *vSeparator = new QFrame(this);
    vSeparator->setFrameShape(QFrame::VLine);
    vSeparator->setStyleSheet("background-color: #E8E8E8; border: none; width: 1px; margin-top: 25px; margin-bottom: 25px;");

    // ==========================================
    // ПРАВАЯ ЧАСТЬ
    // ==========================================
    m_rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(m_rightPanel);
    rightLayout->setContentsMargins(20, 20, 30, 20);
    rightLayout->setAlignment(Qt::AlignTop);

    m_userInfoCard = new QFrame(m_rightPanel);
    m_userInfoCard->setFixedHeight(90);
    m_userInfoCard->setObjectName("userInfoCard");
    m_userInfoCard->setStyleSheet("QFrame#userInfoCard { border: 2px solid #DCDCDC; border-radius: 12px; background-color: transparent; }");

    QHBoxLayout *cardLayout = new QHBoxLayout(m_userInfoCard);
    cardLayout->setContentsMargins(15, 10, 15, 10);
    cardLayout->setSpacing(15);

    m_userIconLabel = new QLabel(m_userInfoCard);
    m_userIconLabel->setFixedSize(40, 40);
    m_userIconLabel->setAlignment(Qt::AlignCenter);
    m_userIconLabel->setStyleSheet("border: none;");

    m_textStack = new QStackedWidget(m_userInfoCard);
    m_textStack->setStyleSheet("background: transparent; border: none;");

    QWidget *viewWidget = new QWidget();
    QVBoxLayout *viewLayout = new QVBoxLayout(viewWidget);
    viewLayout->setContentsMargins(0, 0, 0, 0);
    viewLayout->setSpacing(2);
    viewLayout->setAlignment(Qt::AlignVCenter);

    m_userNameLabel = new QLabel("Имя", viewWidget);
    m_userNameLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 20px; font-weight: bold; color: #000000; border: none;");

    QHBoxLayout *viewRoleLayout = new QHBoxLayout();
    viewRoleLayout->setContentsMargins(0, 0, 0, 0);
    viewRoleLayout->setSpacing(10);
    m_userIdViewLabel = new QLabel("ID: #0", viewWidget);
    m_userIdViewLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; color: #888888; border: none; font-weight: bold;");
    m_userRoleViewLabel = new QLabel("user", viewWidget);
    m_userRoleViewLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; color: #888888; border: none; font-weight: bold;");
    viewRoleLayout->addWidget(m_userIdViewLabel);
    viewRoleLayout->addWidget(m_userRoleViewLabel);
    viewRoleLayout->addStretch();

    viewLayout->addWidget(m_userNameLabel);
    viewLayout->addLayout(viewRoleLayout);

    QWidget *editWidget = new QWidget();
    QVBoxLayout *editLayout = new QVBoxLayout(editWidget);
    editLayout->setContentsMargins(0, 0, 0, 0);
    editLayout->setSpacing(4);
    editLayout->setAlignment(Qt::AlignVCenter);

    m_nameEditField = new QLineEdit(editWidget);
    m_nameEditField->setFixedWidth(160);
    m_nameEditField->setFixedHeight(26);
    m_nameEditField->setStyleSheet("border: 1px solid #DCDCDC; border-radius: 6px; padding: 0 5px; font-family: 'Century Gothic'; font-size: 14px; font-weight: bold; background: #FFFFFF; color: #000;");

    QHBoxLayout *editRoleLayout = new QHBoxLayout();
    editRoleLayout->setContentsMargins(0, 0, 0, 0);
    editRoleLayout->setSpacing(10);
    m_userIdEditLabel = new QLabel("ID: #0", editWidget);
    m_userIdEditLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; color: #888888; border: none; font-weight: bold;");

    m_roleEditCombo = new QComboBox(editWidget);
    m_roleEditCombo->setFixedWidth(110);
    m_roleEditCombo->setFixedHeight(22);
    m_roleEditCombo->addItem("Пользователь", "user");
    m_roleEditCombo->addItem("Администратор", "admin");
    m_roleEditCombo->setStyleSheet("QComboBox { border: 1px solid #DCDCDC; border-radius: 4px; padding: 0 5px; font-family: 'Century Gothic'; font-size: 11px; background: #FFFFFF; color: #333; } QComboBox::drop-down { border: none; width: 15px; }");

    editRoleLayout->addWidget(m_userIdEditLabel);
    editRoleLayout->addWidget(m_roleEditCombo);
    editRoleLayout->addStretch();

    editLayout->addWidget(m_nameEditField);
    editLayout->addLayout(editRoleLayout);

    m_textStack->addWidget(viewWidget);
    m_textStack->addWidget(editWidget);

    m_btnEditUser = new QPushButton("Редактировать", m_userInfoCard);
    m_btnEditUser->setFixedWidth(125);
    m_btnEditUser->setFixedHeight(32);
    m_btnEditUser->setCursor(Qt::PointingHandCursor);
    m_btnEditUser->setStyleSheet("QPushButton { border: 2px solid #DCDCDC; border-radius: 16px; font-family: 'Century Gothic'; font-size: 12px; color: #333333; background: transparent; } QPushButton:hover { background-color: #F5F5F5; border-color: #BDBDBD; }");
    connect(m_btnEditUser, &QPushButton::clicked, this, &UsersPage::onEditUserClicked);

    m_btnDeleteUser = new QPushButton("Удалить", m_userInfoCard);
    m_btnDeleteUser->setFixedWidth(60);
    m_btnDeleteUser->setFixedHeight(32);
    m_btnDeleteUser->setCursor(Qt::PointingHandCursor);
    m_btnDeleteUser->setStyleSheet("QPushButton { border: none; font-family: 'Century Gothic'; font-size: 12px; color: #D86B7A; background: transparent; } QPushButton:hover { text-decoration: underline; }");
    connect(m_btnDeleteUser, &QPushButton::clicked, this, &UsersPage::onDeleteUserClicked);

    cardLayout->addWidget(m_userIconLabel);
    cardLayout->addWidget(m_textStack, 1);
    cardLayout->addStretch(0);
    cardLayout->addWidget(m_btnEditUser);
    cardLayout->addSpacing(5);
    cardLayout->addWidget(m_btnDeleteUser);

    rightLayout->addWidget(m_userInfoCard);

    // ==========================================
    // НИЖНЯЯ ЧАСТЬ: ВКЛАДКИ
    // ==========================================
    m_bottomContainer = new QWidget(m_rightPanel);
    QVBoxLayout *bottomLayout = new QVBoxLayout(m_bottomContainer);
    bottomLayout->setContentsMargins(0, 20, 0, 0);
    bottomLayout->setSpacing(15);

    QHBoxLayout *tabsMenuLayout = new QHBoxLayout();
    tabsMenuLayout->setSpacing(25);
    tabsMenuLayout->setAlignment(Qt::AlignLeft);

    m_tabsGroup = new QButtonGroup(this);
    m_tabsGroup->setExclusive(true);

    QString tabStyle =
        "QPushButton { background: transparent; border: none; color: #888888; font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; padding-bottom: 5px; }"
        "QPushButton:checked { color: #8B2C3E; border-bottom: 2px solid #8B2C3E; }";

    QPushButton *tabNotes = new QPushButton("Заметки", m_bottomContainer);
    tabNotes->setCheckable(true);
    tabNotes->setStyleSheet(tabStyle);
    tabNotes->setCursor(Qt::PointingHandCursor);

    QPushButton *tabBouquets = new QPushButton("Букеты", m_bottomContainer);
    tabBouquets->setCheckable(true);
    tabBouquets->setStyleSheet(tabStyle);
    tabBouquets->setCursor(Qt::PointingHandCursor);

    QPushButton *tabFavs = new QPushButton("Избранное", m_bottomContainer);
    tabFavs->setCheckable(true);
    tabFavs->setStyleSheet(tabStyle);
    tabFavs->setCursor(Qt::PointingHandCursor);

    m_tabsGroup->addButton(tabNotes, 0);
    m_tabsGroup->addButton(tabBouquets, 1);
    m_tabsGroup->addButton(tabFavs, 2);

    tabsMenuLayout->addWidget(tabNotes);
    tabsMenuLayout->addWidget(tabBouquets);
    tabsMenuLayout->addWidget(tabFavs);
    tabsMenuLayout->addStretch();

    bottomLayout->addLayout(tabsMenuLayout);

    m_lowerStack = new QStackedWidget(m_bottomContainer);

    m_lowerStack->addWidget(createNotesTab());
    m_lowerStack->addWidget(createBouquetsTab());
    m_lowerStack->addWidget(createFavoritesTab());

    bottomLayout->addWidget(m_lowerStack, 1);

    connect(m_tabsGroup, &QButtonGroup::idClicked, m_lowerStack, &QStackedWidget::setCurrentIndex);
    tabNotes->setChecked(true);

    rightLayout->addWidget(m_bottomContainer, 1);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(vSeparator);
    mainLayout->addWidget(m_rightPanel, 1);

    m_userInfoCard->hide();
    m_bottomContainer->hide();
}

// ---------------------------------------------------------
// СТИЛИ (ДОБАВЛЕНО СОСТОЯНИЕ DISABLED ДЛЯ КНОПОК)
// ---------------------------------------------------------
QString getTableStyle() {
    return "QTableWidget { border: 1px solid #DCDCDC; border-radius: 8px; gridline-color: #DCDCDC; background-color: #FFFFFF; font-family: 'Century Gothic'; font-size: 13px; color: #333333; }"
           "QHeaderView::section { background-color: #F2F2F2; border: none; border-bottom: 1px solid #DCDCDC; border-right: 1px solid #DCDCDC; font-family: 'Century Gothic'; font-size: 12px; font-weight: bold; color: #555555; height: 35px; }"
           "QHeaderView::section:last { border-right: none; }"
           "QTableWidget::item { border-bottom: 1px solid #DCDCDC; }"
           "QTableWidget::item:selected { background-color: #FFF0F3; color: #000000; }"
           "QScrollBar:vertical { border: none; background: transparent; width: 6px; margin: 0px; }"
           "QScrollBar::handle:vertical { background: #DCDCDC; border-radius: 3px; min-height: 30px; }"
           "QScrollBar::handle:vertical:hover { background: #D86B7A; }"
           "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
           "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }";
}

QString getActionBtnStyle() {
    return "QPushButton { background: transparent; border: 1px solid #DCDCDC; border-radius: 12px; padding: 4px 15px; font-family: 'Century Gothic'; font-size: 12px; color: #555555; }"
           "QPushButton:hover:!disabled { background: #F5F5F5; border-color: #BDBDBD; }"
           "QPushButton:disabled { color: #CCCCCC; border-color: #EEEEEE; }"; // СЕРЫЙ ЦВЕТ ЕСЛИ НЕ АКТИВНО
}

QWidget* UsersPage::createNotesTab() {
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *title = new QLabel("ЗАМЕТКИ", tab);
    title->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; color: #333333; text-transform: uppercase;");

    m_btnDeleteNote = new QPushButton("Удалить", tab);

    m_btnDeleteNote->setStyleSheet(getActionBtnStyle());
    m_btnDeleteNote->setCursor(Qt::PointingHandCursor);

    m_btnDeleteNote->setEnabled(false); // ИЗНАЧАЛЬНО ВЫКЛЮЧЕНА

    connect(m_btnDeleteNote, &QPushButton::clicked, this, &UsersPage::onDeleteNoteClicked);

    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(m_btnDeleteNote);
    layout->addLayout(headerLayout);

    QLabel *hint = new QLabel("Дважды кликните по ячейке, чтобы открыть редактирование. Сохранение при нажатии Enter.", tab);
    hint->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; font-style: italic; color: #888888;");
    layout->addWidget(hint);

    m_notesTable = new QTableWidget(0, 2, tab);
    m_notesTable->setHorizontalHeaderLabels({"ТЕМА", "ТЕКСТ"});
    m_notesTable->horizontalHeader()->setStretchLastSection(true);
    m_notesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_notesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_notesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // ВАЖНО: Разрешаем редактирование по двойному клику
    m_notesTable->setEditTriggers(QAbstractItemView::DoubleClicked);

    m_notesTable->setFocusPolicy(Qt::NoFocus);
    m_notesTable->setShowGrid(true);
    m_notesTable->setFrameShape(QFrame::NoFrame);
    m_notesTable->setStyleSheet(getTableStyle());
    m_notesTable->verticalHeader()->setVisible(false);

    // Включаем кнопку удаления только если выбрана строка
    connect(m_notesTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        m_btnDeleteNote->setEnabled(m_notesTable->selectedItems().count() > 0);
    });
    // Подключаем сохранение в базу после редактирования
    connect(m_notesTable, &QTableWidget::itemChanged, this, &UsersPage::onNoteItemChanged);

    layout->addWidget(m_notesTable, 1);
    return tab;
}

QWidget* UsersPage::createBouquetsTab() {
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *title = new QLabel("СОХРАНЕННЫЕ БУКЕТЫ", tab);
    title->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; color: #333333; text-transform: uppercase;");

    m_btnDeleteBouquet = new QPushButton("Удалить", tab);
    m_btnDeleteBouquet->setStyleSheet(getActionBtnStyle());
    m_btnDeleteBouquet->setCursor(Qt::PointingHandCursor);
    m_btnDeleteBouquet->setEnabled(false); // ИЗНАЧАЛЬНО ВЫКЛЮЧЕНА
    connect(m_btnDeleteBouquet, &QPushButton::clicked, this, &UsersPage::onDeleteBouquetClicked);

    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(m_btnDeleteBouquet);
    layout->addLayout(headerLayout);

    QLabel *hint = new QLabel("Дважды кликните по названию букета, чтобы переименовать его.", tab);
    hint->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; font-style: italic; color: #888888;");
    layout->addWidget(hint);

    m_bouquetsTable = new QTableWidget(0, 2, tab);
    m_bouquetsTable->setHorizontalHeaderLabels({"НАЗВАНИЕ БУКЕТА", "ДАТА СОЗДАНИЯ"});
    m_bouquetsTable->horizontalHeader()->setStretchLastSection(true);
    m_bouquetsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_bouquetsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_bouquetsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // ВАЖНО: Разрешаем редактирование по двойному клику
    m_bouquetsTable->setEditTriggers(QAbstractItemView::DoubleClicked);

    m_bouquetsTable->setFocusPolicy(Qt::NoFocus);
    m_bouquetsTable->setShowGrid(true);
    m_bouquetsTable->setFrameShape(QFrame::NoFrame);
    m_bouquetsTable->setStyleSheet(getTableStyle());
    m_bouquetsTable->verticalHeader()->setVisible(false);

    connect(m_bouquetsTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        m_btnDeleteBouquet->setEnabled(m_bouquetsTable->selectedItems().count() > 0);
    });
    connect(m_bouquetsTable, &QTableWidget::itemChanged, this, &UsersPage::onBouquetItemChanged);

    layout->addWidget(m_bouquetsTable, 1);
    return tab;
}

QWidget* UsersPage::createFavoritesTab() {
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *title = new QLabel("ИЗБРАННОЕ", tab);
    title->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; color: #333333; text-transform: uppercase;");

    m_btnDeleteFav = new QPushButton("Удалить", tab);
    m_btnDeleteFav->setStyleSheet(getActionBtnStyle());
    m_btnDeleteFav->setCursor(Qt::PointingHandCursor);
    m_btnDeleteFav->setEnabled(false); // ИЗНАЧАЛЬНО ВЫКЛЮЧЕНА
    connect(m_btnDeleteFav, &QPushButton::clicked, this, &UsersPage::onDeleteFavClicked);

    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(m_btnDeleteFav);
    layout->addLayout(headerLayout);

    QLabel *hint = new QLabel("Избранные материалы (только чтение)", tab);
    hint->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; font-style: italic; color: #888888;");
    layout->addWidget(hint);

    m_favsTable = new QTableWidget(0, 2, tab);
    m_favsTable->setHorizontalHeaderLabels({"НАИМЕНОВАНИЕ", "ТИП КОМПОНЕНТА"});
    m_favsTable->horizontalHeader()->setStretchLastSection(true);
    m_favsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_favsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_favsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Избранное из каталога логичнее не редактировать отсюда
    m_favsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_favsTable->setFocusPolicy(Qt::NoFocus);
    m_favsTable->setShowGrid(true);
    m_favsTable->setFrameShape(QFrame::NoFrame);
    m_favsTable->setStyleSheet(getTableStyle());
    m_favsTable->verticalHeader()->setVisible(false);

    connect(m_favsTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        m_btnDeleteFav->setEnabled(m_favsTable->selectedItems().count() > 0);
    });

    layout->addWidget(m_favsTable, 1);
    return tab;
}

// ---------------------------------------------------------
// ЗАГРУЗКА ДАННЫХ В ТАБЛИЦЫ (Блокируем сигналы редактирования при загрузке!)
// ---------------------------------------------------------
void UsersPage::loadUsers() {
    m_usersList->blockSignals(true);
    m_usersList->clear();
    QString searchText = m_searchEdit->text().trimmed();

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return;

    QSqlQuery q(db);
    q.prepare("SELECT id, username, role FROM users WHERE id != :currId");
    q.bindValue(":currId", m_currentUserId);

    if (!q.exec()) return;

    QFont itemFont("Century Gothic", 11);

    while (q.next()) {
        int id = q.value(0).toInt();
        QString name = q.value(1).toString();
        QString role = q.value(2).toString().trimmed().toLower();

        if (!searchText.isEmpty() && !name.contains(searchText, Qt::CaseInsensitive)) continue;

        QListWidgetItem *item = new QListWidgetItem("   " + name);
        item->setData(Qt::UserRole, id);
        item->setFont(itemFont);

        if (role == "admin") {
            item->setIcon(QIcon(":/sources/icons/admin.svg"));
        } else {
            item->setIcon(QIcon(":/sources/icons/user.svg"));
        }

        m_usersList->addItem(item);
    }

    m_usersList->blockSignals(false);

    if (m_usersList->count() > 0) {
        m_usersList->setCurrentRow(0);
    } else {
        m_userInfoCard->hide();
        m_bottomContainer->hide();
    }
}

void UsersPage::loadUserNotes(int userId) {
    m_notesTable->blockSignals(true); // БЛОКИРУЕМ СИГНАЛ РЕДАКТИРОВАНИЯ!
    m_notesTable->setRowCount(0);
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery q(db);
    q.prepare("SELECT note_id, title, content FROM UserNotes WHERE user_id = :uid ORDER BY note_id DESC");
    q.bindValue(":uid", userId);

    if (q.exec()) {
        int row = 0;
        while (q.next()) {
            m_notesTable->insertRow(row);

            QTableWidgetItem *itemTitle = new QTableWidgetItem(q.value(1).toString());
            QTableWidgetItem *itemContent = new QTableWidgetItem(q.value(2).toString());

            itemTitle->setTextAlignment(Qt::AlignCenter);
            itemContent->setTextAlignment(Qt::AlignCenter);
            itemTitle->setData(Qt::UserRole, q.value(0).toInt()); // Сохраняем note_id

            m_notesTable->setItem(row, 0, itemTitle);
            m_notesTable->setItem(row, 1, itemContent);
            row++;
        }
    }
    m_btnDeleteNote->setEnabled(false);
    m_notesTable->blockSignals(false); // Включаем обратно
}

void UsersPage::loadUserBouquets(int userId) {
    m_bouquetsTable->blockSignals(true);
    m_bouquetsTable->setRowCount(0);
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery q(db);
    q.prepare("SELECT bouquet_id, bouquet_name, created_at FROM SavedBouquets WHERE user_id = :uid ORDER BY bouquet_id DESC");
    q.bindValue(":uid", userId);

    if (q.exec()) {
        int row = 0;
        while (q.next()) {
            m_bouquetsTable->insertRow(row);
            QTableWidgetItem *itemName = new QTableWidgetItem(q.value(1).toString());
            QTableWidgetItem *itemDate = new QTableWidgetItem(q.value(2).toString());

            // Дату запрещаем редактировать
            itemDate->setFlags(itemDate->flags() & ~Qt::ItemIsEditable);

            itemName->setTextAlignment(Qt::AlignCenter);
            itemDate->setTextAlignment(Qt::AlignCenter);
            itemName->setData(Qt::UserRole, q.value(0).toInt()); // Сохраняем bouquet_id

            m_bouquetsTable->setItem(row, 0, itemName);
            m_bouquetsTable->setItem(row, 1, itemDate);
            row++;
        }
    }
    m_btnDeleteBouquet->setEnabled(false);
    m_bouquetsTable->blockSignals(false);
}

void UsersPage::loadUserFavorites(int userId) {
    m_favsTable->setRowCount(0);
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery q(db);
    q.prepare("SELECT f.favorite_id, i.name, i.item_type FROM Favorites f "
              "JOIN Item_variation v ON f.variation_id = v.variation_id "
              "JOIN Items i ON v.item_id = i.item_id "
              "WHERE f.user_id = :uid");
    q.bindValue(":uid", userId);

    if (q.exec()) {
        int row = 0;
        while (q.next()) {
            m_favsTable->insertRow(row);
            QTableWidgetItem *itemName = new QTableWidgetItem(q.value(1).toString());

            QString rawType = q.value(2).toString();
            QString typeStr = "Неизвестно";
            if (rawType == "flower") typeStr = "Цветок";
            else if (rawType == "accessory") typeStr = "Аксессуар";
            else if (rawType == "packaging") typeStr = "Упаковка";

            QTableWidgetItem *itemType = new QTableWidgetItem(typeStr);
            itemName->setTextAlignment(Qt::AlignCenter);
            itemType->setTextAlignment(Qt::AlignCenter);
            itemName->setData(Qt::UserRole, q.value(0).toInt());

            m_favsTable->setItem(row, 0, itemName);
            m_favsTable->setItem(row, 1, itemType);
            row++;
        }
    }
    m_btnDeleteFav->setEnabled(false);
}

// ---------------------------------------------------------
// ОБРАБОТЧИКИ
// ---------------------------------------------------------
void UsersPage::onUserSelected() {
    QListWidgetItem *item = m_usersList->currentItem();
    if (!item) {
        m_userInfoCard->hide();
        m_bottomContainer->hide();
        return;
    }

    m_userInfoCard->show();
    m_bottomContainer->show();

    m_isEditing = false;
    m_textStack->setCurrentIndex(0);

    m_btnEditUser->setText("Редактировать");
    m_btnEditUser->setStyleSheet("QPushButton { border: 2px solid #DCDCDC; border-radius: 16px; font-family: 'Century Gothic'; font-size: 12px; color: #333333; background: transparent; } QPushButton:hover { background-color: #F5F5F5; border-color: #BDBDBD; }");

    QFont boldFont("Century Gothic", 11);
    boldFont.setBold(true);
    for(int i = 0; i < m_usersList->count(); ++i) {
        QFont f("Century Gothic", 11);
        m_usersList->item(i)->setFont(m_usersList->item(i) == item ? boldFont : f);
    }

    int userId = item->data(Qt::UserRole).toInt();

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery q(db);
    q.prepare("SELECT username, role FROM users WHERE id = :id");
    q.bindValue(":id", userId);

    if (q.exec() && q.next()) {
        QString name = q.value(0).toString();
        QString role = q.value(1).toString().trimmed().toLower();

        m_userNameLabel->setText(name);
        m_userIdViewLabel->setText(QString("ID: #%1").arg(userId));
        m_userRoleViewLabel->setText(role);

        m_nameEditField->setText(name);
        m_userIdEditLabel->setText(QString("ID: #%1").arg(userId));

        if (role == "admin") {
            m_roleEditCombo->setCurrentIndex(1);
            m_userIconLabel->setPixmap(QIcon(":/sources/icons/admin_user.png").pixmap(45, 45));
        } else {
            m_roleEditCombo->setCurrentIndex(0);
            m_userIconLabel->setPixmap(QIcon(":/sources/icons/normal_user.png").pixmap(45, 45));
        }

        loadUserNotes(userId);
        loadUserBouquets(userId);
        loadUserFavorites(userId);
    }
}

void UsersPage::onEditUserClicked() {
    QListWidgetItem *item = m_usersList->currentItem();
    if (!item) return;
    int userId = item->data(Qt::UserRole).toInt();

    if (!m_isEditing) {
        m_isEditing = true;
        m_textStack->setCurrentIndex(1);
        m_btnEditUser->setText("Сохранить");
        m_btnEditUser->setStyleSheet("QPushButton { border: none; border-radius: 16px; font-family: 'Century Gothic'; font-size: 12px; color: white; background: #D86B7A; font-weight: bold; } QPushButton:hover { background-color: #C75A69; }");
    } else {
        QString newName = m_nameEditField->text().trimmed();
        QString newRole = m_roleEditCombo->currentData().toString();

        if (!newName.isEmpty()) {
            QSqlDatabase db = QSqlDatabase::database();
            QSqlQuery q(db);
            q.prepare("UPDATE users SET username = :name, role = :role WHERE id = :id");
            q.bindValue(":name", newName);
            q.bindValue(":role", newRole);
            q.bindValue(":id", userId);

            if (q.exec()) {
                m_userNameLabel->setText(newName);
                m_userRoleViewLabel->setText(newRole);
                if (newRole == "admin") {
                    m_userIconLabel->setPixmap(QIcon(":/sources/icons/admin_user.png").pixmap(45, 45));
                    item->setIcon(QIcon(":/sources/icons/admin_user.png"));
                } else {
                    m_userIconLabel->setPixmap(QIcon(":/sources/icons/normal_user.png").pixmap(45, 45));
                    item->setIcon(QIcon(":/sources/icons/normal_user.png"));
                }
                item->setText("   " + newName);
            }
        }
        m_isEditing = false;
        m_textStack->setCurrentIndex(0);
        m_btnEditUser->setText("Редактировать");
        m_btnEditUser->setStyleSheet("QPushButton { border: 2px solid #DCDCDC; border-radius: 16px; font-family: 'Century Gothic'; font-size: 12px; color: #333333; background: transparent; } QPushButton:hover { background-color: #F5F5F5; border-color: #BDBDBD; }");
    }
}

void UsersPage::onDeleteUserClicked() {
    QListWidgetItem *item = m_usersList->currentItem();
    if (!item) return;

    int userId = item->data(Qt::UserRole).toInt();

    QWidget *overlay = new QWidget(this->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->setGeometry(this->window()->rect());
    overlay->show();

    ConfirmDialog dialog("Удаление пользователя", "Вы действительно хотите удалить этого пользователя и все связанные с ним данные?", this->window());
    dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());

    if (dialog.exec() == QDialog::Accepted) {
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery q(db);
        q.prepare("DELETE FROM users WHERE id = :id");
        q.bindValue(":id", userId);
        if (q.exec()) loadUsers();
    }
    overlay->deleteLater();
}

void UsersPage::onSearchTextChanged(const QString &text) {
    Q_UNUSED(text);
    loadUsers();
}

// ---------------------------------------------------------
// СОХРАНЕНИЕ ПРИ РЕДАКТИРОВАНИИ ТАБЛИЦ (ДВОЙНОЙ КЛИК)
// ---------------------------------------------------------
void UsersPage::onNoteItemChanged(QTableWidgetItem *item) {
    int row = item->row();
    int col = item->column();
    int noteId = m_notesTable->item(row, 0)->data(Qt::UserRole).toInt();
    QString newValue = item->text().trimmed();

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery q(db);
    if (col == 0) { // Изменили тему
        q.prepare("UPDATE UserNotes SET title = :val WHERE note_id = :nid");
    } else { // Изменили текст
        q.prepare("UPDATE UserNotes SET content = :val WHERE note_id = :nid");
    }
    q.bindValue(":val", newValue);
    q.bindValue(":nid", noteId);

    if (!q.exec()) qDebug() << "Ошибка автосохранения заметки:" << q.lastError().text();
}

void UsersPage::onBouquetItemChanged(QTableWidgetItem *item) {
    int row = item->row();
    int col = item->column();
    if (col != 0) return; // Редактировать можно только имя букета

    int bouquetId = m_bouquetsTable->item(row, 0)->data(Qt::UserRole).toInt();
    QString newName = item->text().trimmed();

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery q(db);
    q.prepare("UPDATE SavedBouquets SET bouquet_name = :val WHERE bouquet_id = :bid");
    q.bindValue(":val", newName);
    q.bindValue(":bid", bouquetId);

    if (!q.exec()) qDebug() << "Ошибка автосохранения букета:" << q.lastError().text();
}

// ---------------------------------------------------------
// УДАЛЕНИЕ ИЗ ТАБЛИЦ
// ---------------------------------------------------------
void UsersPage::onDeleteNoteClicked() {
    int row = m_notesTable->currentRow();
    if (row < 0) return;
    int noteId = m_notesTable->item(row, 0)->data(Qt::UserRole).toInt();

    QWidget *overlay = new QWidget(this->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->setGeometry(this->window()->rect());
    overlay->show();

    ConfirmDialog dialog("Удаление", "Удалить выбранную заметку?", this->window());
    dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());

    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery q;
        q.prepare("DELETE FROM UserNotes WHERE note_id = :nid");
        q.bindValue(":nid", noteId);
        if (q.exec()) m_notesTable->removeRow(row);
    }
    overlay->deleteLater();
}

void UsersPage::onDeleteBouquetClicked() {
    int row = m_bouquetsTable->currentRow();
    if (row < 0) return;
    int bouquetId = m_bouquetsTable->item(row, 0)->data(Qt::UserRole).toInt();

    QWidget *overlay = new QWidget(this->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->setGeometry(this->window()->rect());
    overlay->show();

    ConfirmDialog dialog("Удаление", "Удалить этот букет?", this->window());
    dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());

    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery q;
        q.prepare("DELETE FROM SavedBouquets WHERE bouquet_id = :bid");
        q.bindValue(":bid", bouquetId);
        if (q.exec()) m_bouquetsTable->removeRow(row);
    }
    overlay->deleteLater();
}

void UsersPage::onDeleteFavClicked() {
    int row = m_favsTable->currentRow();
    if (row < 0) return;
    int favId = m_favsTable->item(row, 0)->data(Qt::UserRole).toInt();

    QWidget *overlay = new QWidget(this->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->setGeometry(this->window()->rect());
    overlay->show();

    ConfirmDialog dialog("Удаление", "Удалить элемент из избранного?", this->window());
    dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());

    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery q;
        q.prepare("DELETE FROM Favorites WHERE favorite_id = :fid");
        q.bindValue(":fid", favId);
        if (q.exec()) m_favsTable->removeRow(row);
    }
    overlay->deleteLater();
}