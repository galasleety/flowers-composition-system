#include "catalogview.h"
#include "catalogtopbar.h"
#include "catalogrightcolumn.h"
#include "catalogbottomgrid.h"
#include "catalogminicard.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QSizePolicy>
#include <QPixmap>
#include <QCoreApplication>
#include <QWindow>
#include <QScreen>
#include <QGridLayout>
#include <QDebug>
#include <QScrollArea>
#include <QScrollBar>

CatalogView::CatalogView(QWidget *parent) : QWidget(parent)
{
    m_currentUserId = -1; // Изначально пользователь не задан

    setStyleSheet("background-color: #FFFFFF;");
    setupUI();

    // Загрузка с задержкой для правильного просчета геометрии
    QTimer::singleShot(110, this, &CatalogView::loadMainCatalog);
}

void CatalogView::setCurrentUser(int userId) {
    m_currentUserId = userId;
    updateFavoriteButtonState();
}

void CatalogView::scrollToAndSelectItem(int itemId, const QString &itemType) {
    Q_UNUSED(itemType);
    int idx = m_slideItemIds.indexOf(itemId);
    if (idx != -1) displaySlide(idx);
}

void CatalogView::setupUI()
{
    // 1. Основной лейаут самого виджета CatalogView
    QVBoxLayout *globalLayout = new QVBoxLayout(this);
    globalLayout->setContentsMargins(0, 0, 0, 0);
    globalLayout->setSpacing(0);

    // 2. Создаем область прокрутки (Scroll Area)
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");
    // Делаем скроллбар аккуратным
    scrollArea->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical { border: none; background: #F5F5F5; width: 8px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #CCCCCC; border-radius: 4px; min-height: 20px; }"
        "QScrollBar::handle:vertical:hover { background: #B3B3B3; }"
        );

    // 3. Создаем внутренний виджет, который будет содержать ВСЕ элементы
    QWidget *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background-color: #FFFFFF;");
    QVBoxLayout *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(60, 15, 60, 40); // Отступы как на макете
    contentLayout->setSpacing(0);

    // --- А) ВЕРХНЯЯ ПАНЕЛЬ (ФИЛЬТРЫ) ---
    m_topBar = new CatalogTopBar(this);
    contentLayout->addWidget(m_topBar);
    contentLayout->addSpacing(25);

    // --- Б) СРЕДНЯЯ ЧАСТЬ (БОЛЬШАЯ КАРТОЧКА С КНОПКАМИ) ---
    QHBoxLayout *cardAreaLayout = new QHBoxLayout();
    cardAreaLayout->setSpacing(20);

    m_prevBtn = new QPushButton("‹");
    m_prevBtn->setFixedSize(50, 50);
    m_prevBtn->setCursor(Qt::PointingHandCursor);
    m_prevBtn->setStyleSheet(
        "QPushButton { background-color: #FFFFFF; border: 1px solid #CCCCCC; border-radius: 25px; "
        "color: #333333; font-size: 26px; font-weight: 300; padding-bottom: 4px; padding-right: 2px; }"
        "QPushButton:hover { border: 1px solid #8E3A4B; color: #8E3A4B; }"
        );
    connect(m_prevBtn, &QPushButton::clicked, this, &CatalogView::onPrevClicked);

    m_nextBtn = new QPushButton("›");
    m_nextBtn->setFixedSize(50, 50);
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    m_nextBtn->setStyleSheet(
        "QPushButton { background-color: #FFFFFF; border: 1px solid #CCCCCC; border-radius: 25px; "
        "color: #333333; font-size: 26px; font-weight: 300; padding-bottom: 4px; padding-left: 2px; }"
        "QPushButton:hover { border: 1px solid #8E3A4B; color: #8E3A4B; }"
        );
    connect(m_nextBtn, &QPushButton::clicked, this, &CatalogView::onNextClicked);

    cardAreaLayout->addStretch();
    cardAreaLayout->addWidget(m_prevBtn);

    QWidget *mainCard = createMainCard();
    mainCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    cardAreaLayout->addWidget(mainCard, 1);

    cardAreaLayout->addWidget(m_nextBtn);
    cardAreaLayout->addStretch();

    contentLayout->addLayout(cardAreaLayout);

    // --- В) НИЖНЯЯ ЧАСТЬ (ПОЛНЫЙ КАТАЛОГ С СЕТКОЙ) ---
    contentLayout->addSpacing(60);

    QLabel *fullCatalogTitle = new QLabel("Полный каталог");
    fullCatalogTitle->setAlignment(Qt::AlignCenter);
    fullCatalogTitle->setStyleSheet(
        "font-family: 'Georgia', serif; font-size: 38px; font-weight: bold; color: #111111; border: none;"
        );
    contentLayout->addWidget(fullCatalogTitle);

    QLabel *subTitle = new QLabel("Нажмите на интересующий вас элемент");
    subTitle->setAlignment(Qt::AlignCenter);
    subTitle->setStyleSheet(
        "font-family: 'Century Gothic'; font-size: 16px; color: #888888; border: none; margin-top: 8px;"
        );
    contentLayout->addWidget(subTitle);
    contentLayout->addSpacing(30);

    m_bottomGrid = new CatalogBottomGrid(this);
    contentLayout->addWidget(m_bottomGrid);

    connect(m_bottomGrid, &CatalogBottomGrid::itemSelected, this, [this, scrollArea](int itemId){
        this->scrollToAndSelectItem(itemId, "");
        scrollArea->ensureVisible(0, 0);
    });

    scrollArea->setWidget(scrollContent);
    globalLayout->addWidget(scrollArea);

    connect(m_topBar, &CatalogTopBar::filtersChanged, this, &CatalogView::loadMainCatalog);
}

QWidget* CatalogView::createMainCard()
{
    QFrame *card = new QFrame(this);
    card->setObjectName("mainCatalogCard");
    card->setMinimumHeight(500);
    card->setMaximumWidth(1200);
    card->setStyleSheet("QFrame#mainCatalogCard { background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 20px; }");

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(25);
    shadow->setColor(QColor(0, 0, 0, 15));
    shadow->setOffset(0, 8);
    card->setGraphicsEffect(shadow);

    QGridLayout *cardLayout = new QGridLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    // --- 1. ЛЕВАЯ КОЛОНКА ---
    QWidget *leftWrapper = new QWidget();
    QVBoxLayout *leftCol = new QVBoxLayout(leftWrapper);
    leftWrapper->setContentsMargins(40, 25, 20, 30);
    leftCol->setAlignment(Qt::AlignTop);
    leftCol->setSpacing(0);

    m_titleLabel = new QLabel("Загрузка...");
    m_titleLabel->setStyleSheet("font-family: 'Georgia', serif; font-size: 42px; font-weight: bold; color: #111111; border: none; line-height: 1.1;");
    m_titleLabel->setWordWrap(true);

    m_descLabel = new QLabel("");
    m_descLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 18px; color: #666666; border: none; line-height: 1.4; margin-top: 10px;");
    m_descLabel->setWordWrap(true);

    QWidget *tagsWidget = new QWidget();
    m_tagsLayout = new QHBoxLayout(tagsWidget);
    m_tagsLayout->setContentsMargins(0, 15, 0, 0);
    m_tagsLayout->setSpacing(10);
    m_tagsLayout->setAlignment(Qt::AlignLeft);

    leftCol->addWidget(m_titleLabel);
    leftCol->addWidget(m_descLabel);
    leftCol->addWidget(tagsWidget);

    leftCol->addStretch(1);

    // --- МОДЕРНИЗИРОВАННАЯ КНОПКА ИЗБРАННОГО С ДВУМЯ СОСТОЯНИЯМИ QICON ---
    m_favoriteBtn = new QPushButton();
    m_favoriteBtn->setFixedSize(44, 44);
    m_favoriteBtn->setCursor(Qt::PointingHandCursor);
    m_favoriteBtn->setIconSize(QSize(24, 24));
    m_favoriteBtn->setCheckable(true); // Кнопка становится триггером переключения

    QIcon dynamicLikeIcon;
    QPixmap emptyPix(":/sources/icons/free-icon-favorite-15397171.svg");
    QPixmap filledPix(":/sources/icons/heart_full.svg");

    // Принудительно сглаживаем текстуры высокого разрешения при добавлении в QIcon
    if (!emptyPix.isNull()) {
        dynamicLikeIcon.addPixmap(emptyPix.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation), QIcon::Normal, QIcon::Off);
    }
    if (!filledPix.isNull()) {
        dynamicLikeIcon.addPixmap(filledPix.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation), QIcon::Normal, QIcon::On);
    }
    m_favoriteBtn->setIcon(dynamicLikeIcon);

    m_favoriteBtn->setStyleSheet(
        "QPushButton { background-color: #FFFFFF; border: 1px solid #CCCCCC; border-radius: 22px; }"
        "QPushButton:hover { border: 1px solid #8E3A4B; }"
        );
    connect(m_favoriteBtn, &QPushButton::clicked, this, &CatalogView::onFavoriteClicked);

    QHBoxLayout *favLayout = new QHBoxLayout();
    favLayout->addStretch();
    favLayout->addWidget(m_favoriteBtn);
    leftCol->addLayout(favLayout);

    leftCol->addSpacing(15);

    QFrame *hLine = new QFrame();
    hLine->setFixedHeight(1);
    hLine->setStyleSheet("background-color: #DCDCDC; border: none; margin-bottom: 15px;");
    leftCol->addWidget(hLine);

    m_variationsCountLabel = new QLabel("00/00");
    m_variationsCountLabel->setAlignment(Qt::AlignCenter);
    m_variationsCountLabel->setStyleSheet("font-family: 'Georgia', serif; font-size: 22px; font-weight: bold; color: #111111; border: none;");
    leftCol->addWidget(m_variationsCountLabel);

    // --- 2. ПУСТАЯ ЦЕНТРАЛЬНАЯ КОЛОНКА ---
    QWidget *centerSpacer = new QWidget();

    // --- 3. ПРАВАЯ КОЛОНКА ---
    m_rightColumn = new CatalogRightColumn(this);
    connect(m_rightColumn, &CatalogRightColumn::variationSelected, this, &CatalogView::onVariationSelected);

    // --- 4. ИЗОБРАЖЕНИЕ-ОВЕРЛЕЙ ---
    m_imageLabel = new QLabel();
    m_imageLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_imageLabel->setStyleSheet("border: none; background: transparent; padding: 0px; margin: 0px;");

    cardLayout->addWidget(leftWrapper, 0, 0);
    cardLayout->addWidget(centerSpacer, 0, 1);
    cardLayout->addWidget(m_rightColumn, 0, 2);
    cardLayout->addWidget(m_imageLabel, 0, 0, 1, 3);

    cardLayout->setColumnStretch(0, 35);
    cardLayout->setColumnStretch(1, 32);
    cardLayout->setColumnStretch(2, 33);

    return card;
}

void CatalogView::updateImage()
{
    if (m_currentVariationId <= 0 || !m_imageLabel) return;

    m_imageLabel->clear();

    QString itemType = "";
    QSqlQuery typeQuery;
    typeQuery.prepare("SELECT item_type FROM Items WHERE item_id = (SELECT item_id FROM Item_variation WHERE variation_id = :vid)");
    typeQuery.bindValue(":vid", m_currentVariationId);
    if (typeQuery.exec() && typeQuery.next()) {
        itemType = typeQuery.value(0).toString();
    }

    if (itemType == "flower") {
        m_imageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    } else {
        m_imageLabel->setAlignment(Qt::AlignCenter);
    }

    QSqlQuery q;
    q.prepare("SELECT image FROM Item_variation WHERE variation_id = :id");
    q.bindValue(":id", m_currentVariationId);

    if (q.exec() && q.next()) {
        QString imagePath = q.value(0).toString();
        if (!imagePath.isEmpty()) {
            QPixmap pix(imagePath);
            if (!pix.isNull()) {
                qreal dpr = this->devicePixelRatioF();

                QFrame *card = findChild<QFrame*>("mainCatalogCard");
                int cardH = card ? card->height() : 480;
                int cardW = card ? card->width() : 1200;

                int targetH = cardH;
                int targetW = cardW * 0.45;

                QPixmap scaledPix = pix.scaled(
                    QSize(targetW, targetH) * dpr,
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                    );

                scaledPix.setDevicePixelRatio(dpr);
                m_imageLabel->setPixmap(scaledPix);
            }
        }
    }
}

void CatalogView::onVariationSelected(int variationId)
{
    m_currentVariationId = variationId;
    m_currentVariationIndex = m_itemVariationIds.indexOf(variationId) + 1;

    updatePaginationLabel();
    updateFavoriteButtonState();
    updateImage();

    if (!m_slideItemIds.isEmpty()) {
        m_rightColumn->loadItem(m_slideItemIds.at(m_currentSlideIndex), m_currentVariationId);
    }
}

void CatalogView::clearLayout(QLayout *layout) {
    if (!layout) return;
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (QWidget *widget = item->widget()) {
            widget->hide();
            widget->setParent(nullptr);
            delete widget;
        }
        delete item;
    }
}

void CatalogView::loadMainCatalog() {
    int rememberedItemId = -1;
    if (!m_slideItemIds.isEmpty() && m_currentSlideIndex >= 0 && m_currentSlideIndex < m_slideItemIds.count()) {
        rememberedItemId = m_slideItemIds.at(m_currentSlideIndex);
    }
    m_slideItemIds.clear();
    m_currentSlideIndex = 0;
    QString category = m_topBar->currentCategory();
    QString searchText = m_topBar->currentSearchText();
    QString sortText = m_topBar->currentSortText();

    if (m_bottomGrid) {
        m_bottomGrid->refresh(category, searchText);
    }

    QString queryStr = "SELECT DISTINCT I.item_id, I.name FROM Items I WHERE I.name NOT LIKE '%(Перед)%' ";

    if (category == "Основные цветы") queryStr += "AND I.item_id IN (SELECT item_id FROM Flower_details WHERE subtype = 'main') ";
    else if (category == "Стаффаж") queryStr += "AND I.item_id IN (SELECT item_id FROM Flower_details WHERE subtype = 'filler') ";
    else if (category == "Упаковка") queryStr += "AND I.item_type = 'packaging' ";
    else if (category == "Аксессуары") queryStr += "AND I.item_type = 'accessory' ";

    if (!searchText.isEmpty()) queryStr += "AND I.name LIKE '%" + searchText + "%' ";
    if (sortText.contains("Я-А")) queryStr += "ORDER BY I.name DESC";
    else queryStr += "ORDER BY I.name ASC";

    QSqlQuery q;
    if (q.exec(queryStr)) {
        while (q.next()) m_slideItemIds.append(q.value(0).toInt());
    }
    if (!m_slideItemIds.isEmpty()) {
        int targetIndex = 0;
        if (rememberedItemId != -1) {
            targetIndex = m_slideItemIds.indexOf(rememberedItemId);
            if (targetIndex == -1) targetIndex = 0;
        }
        displaySlide(targetIndex);
    } else {
        m_titleLabel->setText("Ничего не найдено");
        m_descLabel->setText("Попробуйте изменить фильтры.");
        clearLayout(m_tagsLayout);
        m_variationsCountLabel->setText("00/00");
        m_imageLabel->clear();
    }
}

void CatalogView::displaySlide(int index) {
    if (m_slideItemIds.isEmpty() || index < 0 || index >= m_slideItemIds.count()) return;
    m_currentSlideIndex = index;
    int itemId = m_slideItemIds.at(index);
    QSqlQuery q;
    q.prepare("SELECT name, description, item_type FROM Items WHERE item_id = :id");
    q.bindValue(":id", itemId);
    QString itemType = "";
    if (q.exec() && q.next()) {
        m_titleLabel->setText(q.value(0).toString());
        m_descLabel->setText(q.value(1).toString());
        itemType = q.value(2).toString();
    }
    clearLayout(m_tagsLayout);
    auto addTag = [this](const QString &text) {
        QLabel *tag = new QLabel(text);
        tag->setStyleSheet("background-color: #F9F9F9; border: 1px solid #E0E0E0; border-radius: 6px; padding: 5px 12px; color: #777777; font-family: 'Century Gothic'; font-size: 13px; font-weight: bold;");
        m_tagsLayout->addWidget(tag);
    };
    if (itemType == "flower") {
        addTag("Цветок");
        QSqlQuery fq;
        fq.prepare("SELECT subtype FROM Flower_details WHERE item_id = :id");
        fq.bindValue(":id", itemId);
        if (fq.exec() && fq.next()) {
            if (fq.value(0).toString() == "main") addTag("Основной");
            else addTag("Стаффаж");
        }
    } else if (itemType == "packaging") addTag("Упаковка");
    else if (itemType == "accessory") addTag("Аксессуар");

    m_itemVariationIds.clear();
    QSqlQuery vq;
    vq.prepare("SELECT variation_id FROM Item_variation WHERE item_id = :id ORDER BY variation_id ASC");
    vq.bindValue(":id", itemId);
    if (vq.exec()) {
        while (vq.next()) m_itemVariationIds.append(vq.value(0).toInt());
    }

    m_totalVariations = m_itemVariationIds.count();
    if (m_totalVariations > 0) {
        m_currentVariationIndex = 1;
        m_currentVariationId = m_itemVariationIds.first();
    } else {
        m_currentVariationIndex = 0;
        m_currentVariationId = -1;
    }

    updatePaginationLabel();
    updateFavoriteButtonState();

    if (m_rightColumn) {
        m_rightColumn->loadItem(itemId, m_currentVariationId);
    }

    QCoreApplication::processEvents();
    updateImage();

    if (m_bottomGrid) {
        m_bottomGrid->highlightItem(itemId);
    }
}

void CatalogView::updatePaginationLabel() {
    QString currentStr = QString("%1").arg(m_currentVariationIndex, 2, 10, QChar('0'));
    QString totalStr = QString("%1").arg(m_totalVariations, 2, 10, QChar('0'));
    m_variationsCountLabel->setText(currentStr + "/" + totalStr);
}

// --- УМНОЕ ОБНОВЛЕНИЕ СОСТОЯНИЯ ЧЕРЕЗ СТАТУС CHECKED У QICON ---
void CatalogView::updateFavoriteButtonState() {
    if (m_currentUserId <= 0 || m_currentVariationId <= 0) {
        m_favoriteBtn->setChecked(false); // Отжимаем кнопку -> QIcon показывает heart_empty
        m_favoriteBtn->setStyleSheet(
            "QPushButton { background-color: #FFFFFF; border: 1px solid #CCCCCC; border-radius: 22px; }"
            "QPushButton:hover { border: 1px solid #8E3A4B; }"
            );
        return;
    }

    QSqlQuery q;
    q.prepare("SELECT 1 FROM Favorites WHERE user_id = :u AND variation_id = :v");
    q.bindValue(":u", m_currentUserId);
    q.bindValue(":v", m_currentVariationId);

    if (q.exec() && q.next()) {
        m_favoriteBtn->setChecked(true); // Зажимаем кнопку -> QIcon переключается на heart_full
        m_favoriteBtn->setStyleSheet(
            "QPushButton { background-color: #FFFFFF; border: 1px solid #8E3A4B; border-radius: 22px; }"
            );
    } else {
        m_favoriteBtn->setChecked(false); // Отжимаем кнопку -> QIcon возвращается на heart_empty
        m_favoriteBtn->setStyleSheet(
            "QPushButton { background-color: #FFFFFF; border: 1px solid #CCCCCC; border-radius: 22px; }"
            "QPushButton:hover { border: 1px solid #8E3A4B; }"
            );
    }
}

void CatalogView::onFavoriteClicked() {
    if (m_currentUserId <= 0) {
        qDebug() << "ОШИБКА: Пользователь не авторизован (m_currentUserId <= 0). Элемент не добавлен.";
        return;
    }
    if (m_currentVariationId <= 0) return;

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT 1 FROM Favorites WHERE user_id = :u AND variation_id = :v");
    checkQuery.bindValue(":u", m_currentUserId);
    checkQuery.bindValue(":v", m_currentVariationId);

    if (checkQuery.exec() && checkQuery.next()) {
        QSqlQuery delQuery;
        delQuery.prepare("DELETE FROM Favorites WHERE user_id = :u AND variation_id = :v");
        delQuery.bindValue(":u", m_currentUserId);
        delQuery.bindValue(":v", m_currentVariationId);
        delQuery.exec();
    } else {
        QSqlQuery insQuery;
        insQuery.prepare("INSERT INTO Favorites (user_id, variation_id) VALUES (:u, :v)");
        insQuery.bindValue(":u", m_currentUserId);
        insQuery.bindValue(":v", m_currentVariationId);
        insQuery.exec();
    }
    updateFavoriteButtonState();
}

void CatalogView::onNextClicked() {
    if (m_slideItemIds.isEmpty()) return;
    int nextIdx = m_currentSlideIndex + 1;
    if (nextIdx >= m_slideItemIds.count()) nextIdx = 0;
    displaySlide(nextIdx);
}

void CatalogView::onPrevClicked() {
    if (m_slideItemIds.isEmpty()) return;
    int prevIdx = m_currentSlideIndex - 1;
    if (prevIdx < 0) prevIdx = m_slideItemIds.count() - 1;
    displaySlide(prevIdx);
}