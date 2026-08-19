#include "favoritespanel.h"
#include "errordialog.h"
#include "catalogminicard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QSqlQuery>
#include <QSqlError>
#include <QComboBox>
#include <QPushButton>
#include <QDebug>

FavoritesPanel::FavoritesPanel(QWidget *parent) : QWidget(parent), m_userId(-1) {
    setupUI();
}

void FavoritesPanel::setCurrentUser(int userId) {
    m_userId = userId;
    loadFavorites();
}

void FavoritesPanel::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 10, 0, 0);
    mainLayout->setSpacing(25);

    // --- ВЕРХНЯЯ ПАНЕЛЬ ---
    QWidget *topBar = new QWidget();
    QHBoxLayout *barLayout = new QHBoxLayout(topBar);
    barLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *title = new QLabel("Мои избранные", this);
    title->setStyleSheet("font-family: 'Century Gothic'; font-size: 26px; font-weight: bold; color: #333;");

    m_countLabel = new QLabel("0 элементов", this);
    m_countLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 14px; color: #999; margin-top: 10px;");

    m_sortCombo = new QComboBox();
    m_sortCombo->addItems({"Сначала новые", "По названию"});
    m_sortCombo->setFixedSize(190, 40);
    m_sortCombo->setStyleSheet(
        "QComboBox { border: 1px solid #EEE; border-radius: 12px; padding-left: 15px; background: #F9F9F9; font-family: 'Century Gothic'; }"
        "QComboBox::drop-down { border: none; } "
        "QComboBox::down-arrow { image: url(:/icons/down.png); width: 10px; }"
        );

    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        loadFavorites();
    });

    QPushButton *btnCatalog = new QPushButton("Перейти в каталог", this);
    btnCatalog->setFixedSize(180, 40);
    btnCatalog->setCursor(Qt::PointingHandCursor);
    btnCatalog->setStyleSheet(
        "QPushButton { border: 1px solid #F8C3CD; border-radius: 12px; color: #D86B7A; font-family: 'Century Gothic'; font-weight: bold; background: white; } "
        "QPushButton:hover { background: #FFF0F3; }"
        );

    connect(btnCatalog, &QPushButton::clicked, this, &FavoritesPanel::goToCatalogRequested);

    barLayout->addWidget(title);
    barLayout->addWidget(m_countLabel);
    barLayout->addStretch();
    barLayout->addWidget(m_sortCombo);
    barLayout->addWidget(btnCatalog);
    mainLayout->addWidget(topBar);

    // --- СЕТКА ---
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background: transparent; border: none;");

    QWidget *container = new QWidget();
    container->setStyleSheet("background: transparent;");

    m_favoritesGrid = new QGridLayout(container);
    m_favoritesGrid->setSpacing(25);
    m_favoritesGrid->setContentsMargins(0, 10, 0, 0);
    m_favoritesGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    scroll->setWidget(container);
    mainLayout->addWidget(scroll, 1);
}

void FavoritesPanel::loadFavorites() {
    if (m_userId <= 0) {
        qDebug() << "[FAV] Ошибка: m_userId не установлен!";
        return;
    }

    // 1. Очистка сетки
    QLayoutItem *item;
    while ((item = m_favoritesGrid->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // 2. Формируем запрос
    QString queryStr = "SELECT v.variation_id, i.name, v.image FROM Item_variation v "
                       "JOIN Items i ON v.item_id = i.item_id "
                       "JOIN Favorites f ON v.variation_id = f.variation_id "
                       "WHERE f.user_id = :uid ";

    if (m_sortCombo->currentIndex() == 0) {
        queryStr += "ORDER BY f.liked_at DESC";
    } else {
        queryStr += "ORDER BY i.name ASC";
    }

    QSqlQuery q;
    q.prepare(queryStr);
    q.bindValue(":uid", m_userId);

    int count = 0;
    if (q.exec()) {
        int row = 0, col = 0;
        while (q.next()) {
            int variationId = q.value(0).toInt();

            CatalogMiniCard *card = new CatalogMiniCard(variationId, q.value(1).toString(), q.value(2).toString(), true, this);

            // --- ИСПРАВЛЕНО: КНОПКА УДАЛЕНИЯ С ГРАФИЧЕСКОЙ ИКОНКОЙ ---
            QPushButton *btnRemove = new QPushButton(card);
            btnRemove->setFixedSize(30, 30);
            btnRemove->setCursor(Qt::PointingHandCursor);
            btnRemove->move(160 - 38, 6); // Аккуратное смещение к правому верхнему углу карточки

            // Устанавливаем иконку вместо текста "×"
            btnRemove->setIcon(QIcon(":/sources/icons/free-icon-trash-can-542673.svg"));
            btnRemove->setIconSize(QSize(16, 16));

            btnRemove->setStyleSheet(
                "QPushButton { "
                "   background-color: rgba(255, 255, 255, 220); "
                "   border-radius: 15px; "
                "   border: 1px solid #F0F0F0; "
                "} "
                "QPushButton:hover { background-color: #FFF0F3; border-color: #F8C3CD; }"
                );

            connect(btnRemove, &QPushButton::clicked, this, [this, variationId]() {
                onRemoveRequested(variationId);
            });

            m_favoritesGrid->addWidget(card, row, col);

            if (++col > 4) { col = 0; row++; }
            count++;
        }
    } else {
        qDebug() << "[FAV] Ошибка выполнения запроса:" << q.lastError().text();
    }

    if (count == 0) {
        QLabel *emptyLabel = new QLabel("В избранном пока ничего нет", this);
        emptyLabel->setStyleSheet("color: #CCCCCC; font-family: 'Century Gothic'; font-size: 18px; padding-top: 100px;");
        emptyLabel->setAlignment(Qt::AlignCenter);
        m_favoritesGrid->addWidget(emptyLabel, 0, 0, 1, 4, Qt::AlignCenter);
    }

    m_countLabel->setText(QString("%1 элементов").arg(count));
}

void FavoritesPanel::onRemoveRequested(int id) {
    if (m_userId <= 0) return;

    ConfirmDialog dialog("Удаление", "Вы действительно хотите удалить этот компонент из избранного?", this->window());
    dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());

    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery q;
        q.prepare("DELETE FROM Favorites WHERE user_id = :uid AND variation_id = :vid");
        q.bindValue(":uid", m_userId);
        q.bindValue(":vid", id);

        if (q.exec()) {
            loadFavorites();
        }
    }
}