#include "catalogbottomgrid.h"
#include "catalogminicard.h"
#include <QSqlQuery>
#include <QLabel>
#include <QHBoxLayout>

CatalogBottomGrid::CatalogBottomGrid(QWidget *parent) : QWidget(parent)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 20, 0, 20);
    m_mainLayout->setSpacing(30);
    refresh(); // Первоначальная загрузка
}

void CatalogBottomGrid::highlightItem(int itemId) {
    QList<CatalogMiniCard*> cards = this->findChildren<CatalogMiniCard*>();
    for (CatalogMiniCard* card : cards) {
        // Заменили card->itemId() на card->getId()
        card->setSelected(card->getId() == itemId);
    }
}

void CatalogBottomGrid::refresh(const QString &category, const QString &searchText) {
    // Очищаем старые карточки
    QLayoutItem *child;
    while ((child = m_mainLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    QString searchCondition = "";
    if (!searchText.isEmpty()) {
        searchCondition = " AND name LIKE '%" + searchText + "%'";
    }

    bool showAll = category.isEmpty() || category == "Все" || category == "Все категории";

    // ВЫЗОВЫ С QICON: Укажи здесь СВОИ пути к иконкам вместо моих заглушек
    if (showAll || category == "Основные цветы") {
        addCategorySection("Основные цветы",
                           QIcon(":/sources/icons/flower_main.png"), // <-- Твой путь к иконке цветов
                           "SELECT item_id, name FROM Items WHERE item_id IN (SELECT item_id FROM Flower_details WHERE subtype='main') " + searchCondition);
    }
    if (showAll || category == "Стаффаж") {
        addCategorySection("Стаффаж",
                           QIcon(":/sources/icons/staffage.png"), // <-- Твой путь к иконке стаффажа
                           "SELECT item_id, name FROM Items WHERE item_id IN (SELECT item_id FROM Flower_details WHERE subtype='filler') " + searchCondition);
    }
    if (showAll || category == "Упаковка") {
        addCategorySection("Упаковка",
                           QIcon(":/sources/icons/packaging.png"), // <-- Твой путь к иконке упаковки
                           "SELECT item_id, name FROM Items WHERE item_type='packaging' " + searchCondition);
    }
    if (showAll || category == "Аксессуары") {
        addCategorySection("Аксессуары",
                           QIcon(":/sources/icons/accessory.png"), // <-- Твой путь к иконке аксессуаров
                           "SELECT item_id, name FROM Items WHERE item_type='accessory' " + searchCondition);
    }
}

void CatalogBottomGrid::addCategorySection(const QString &title, const QIcon &icon, const QString &sqlQuery) {
    QSqlQuery q(sqlQuery);
    if (!q.next()) return; // Если элементов нет, пропускаем секцию

    // --- ИСПРАВЛЕНО: ЗАГОЛОВОК КАТЕГОРИИ ПО ЦЕНТРУ С ИКОНКОЙ ---
    QWidget *headerWidget = new QWidget(this);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setAlignment(Qt::AlignCenter); // ВЫРАВНИВАЕМ СТРОГО ПО ЦЕНТРУ
    headerLayout->setContentsMargins(0, 10, 0, 10);
    headerLayout->setSpacing(10); // Отступ между иконкой и текстом

    QLabel *iconLabel = new QLabel(headerWidget);
    iconLabel->setFixedSize(24, 24); // Размер иконки категории
    if (!icon.isNull()) {
        iconLabel->setPixmap(icon.pixmap(24, 24)); // Устанавливаем картинку из QIcon
    }

    QLabel *titleLabel = new QLabel(title, headerWidget);
    titleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 20px; color: #444; font-weight: bold;");

    // Компенсатор справа, чтобы заголовок был идеально по центру (уравновешивает иконку)
    QWidget *rightSpacer = new QWidget(headerWidget);
    rightSpacer->setFixedSize(24, 24);

    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(rightSpacer);

    m_mainLayout->addWidget(headerWidget);
    // -----------------------------------------------------------

    // --- СЕТКА ДЛЯ МИНИ-КАРТОЧЕК ---
    QWidget *gridContainer = new QWidget(this);
    QGridLayout *gridLayout = new QGridLayout(gridContainer);
    gridLayout->setSpacing(20);
    gridLayout->setAlignment(Qt::AlignLeft);

    int column = 0, row = 0;
    do {
        int id = q.value(0).toInt();
        QString name = q.value(1).toString();

        QSqlQuery imgQ;
        imgQ.prepare("SELECT image FROM Item_variation WHERE item_id = :id LIMIT 1");
        imgQ.bindValue(":id", id);
        QString imgPath = (imgQ.exec() && imgQ.next()) ? imgQ.value(0).toString() : "";

        CatalogMiniCard *card = new CatalogMiniCard(id, name, imgPath, false); // Режим каталога (с точками)
        connect(card, &CatalogMiniCard::clicked, this, &CatalogBottomGrid::itemSelected);

        gridLayout->addWidget(card, row, column);
        column++;
        if (column > 5) { column = 0; row++; } // 6 карточек в ряд
    } while (q.next());

    m_mainLayout->addWidget(gridContainer);
}