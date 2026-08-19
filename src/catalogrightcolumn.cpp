#include "catalogrightcolumn.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include <QColor>

const QString SCROLL_STYLE =
    "QScrollBar:vertical { border: none; background: transparent; width: 6px; margin: 0px; }"
    "QScrollBar::handle:vertical { background: #CCCCCC; border-radius: 3px; min-height: 20px; }"
    "QScrollBar::handle:vertical:hover { background: #B3B3B3; }"
    "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }";

const QString TEXT_BROWSER_STYLE =
    "QTextBrowser { background: transparent; border: none; color: #333333; "
    "font-family: 'Century Gothic'; font-size: 13px; line-height: 1.4; padding-bottom: 8px; padding-right: 5px; }";

bool isDarkColor(const QString& hexColor) {
    QColor color(hexColor);
    if (!color.isValid()) return false;
    double luma = ((color.red() * 299) + (color.green() * 587) + (color.blue() * 114)) / 1000;
    return luma < 128;
}

CatalogRightColumn::CatalogRightColumn(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void CatalogRightColumn::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(30, 10, 40, 30);
    m_mainLayout->setSpacing(20);
    // Важно: убираем Qt::AlignTop, чтобы layout мог растягиваться

    // --- БЛОК ЦВЕТОВ (Растягивающийся контейнер) ---
    QWidget *colorsContainer = new QWidget();
    // Делаем контейнер растягивающимся, чтобы он забирал всё свободное место сверху
    colorsContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    m_colorsListLayout = new QVBoxLayout(colorsContainer);
    m_colorsListLayout->setContentsMargins(0, 0, 0, 0);
    m_colorsListLayout->setSpacing(12);
    // Выравниваем содержимое (кружки) по центру этого растянутого контейнера
    m_colorsListLayout->setAlignment(Qt::AlignVCenter);

    m_mainLayout->addWidget(colorsContainer);
    m_mainLayout->addSpacing(5);

    // --- БЛОК "ОСОБЕННОСТИ" ---
    m_featuresBlock = new QWidget();
    m_featuresBlock->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed); // Блок инфы фиксирован по высоте
    m_featuresBlock->setStyleSheet("QWidget#featuresBlock { background-color: #FCF0F4; border: 1px solid #E6B0AA; border-radius: 12px; }");
    m_featuresBlock->setObjectName("featuresBlock");

    QVBoxLayout *featuresLayout = new QVBoxLayout(m_featuresBlock);
    featuresLayout->setContentsMargins(15, 15, 5, 15);
    featuresLayout->setSpacing(8);

    QHBoxLayout *featTitleLayout = new QHBoxLayout();
    featTitleLayout->setSpacing(10);


    QLabel *featTitle = new QLabel("Особенности");
    featTitle->setStyleSheet("color: #8E3A4B; font-family: 'Century Gothic'; font-size: 14px; font-weight: bold; border: none; background: transparent;");

    featTitleLayout->addWidget(featTitle);
    featTitleLayout->addStretch();
    featuresLayout->addLayout(featTitleLayout);

    m_featuresText = new QTextBrowser();
    m_featuresText->setFixedHeight(85);
    m_featuresText->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_featuresText->setStyleSheet(TEXT_BROWSER_STYLE + SCROLL_STYLE);
    featuresLayout->addWidget(m_featuresText);

    m_mainLayout->addWidget(m_featuresBlock);

    // --- БЛОК "УХОД И ПАРАМЕТРЫ" ---
    m_careBlock = new QWidget();
    m_careBlock->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed); // Блок инфы фиксирован по высоте
    m_careBlock->setStyleSheet("QWidget#careBlock { background-color: #FCF0F4; border: 1px solid #E6B0AA; border-radius: 12px; }");
    m_careBlock->setObjectName("careBlock");

    QVBoxLayout *careLayout = new QVBoxLayout(m_careBlock);
    careLayout->setContentsMargins(15, 15, 5, 15);
    careLayout->setSpacing(8);

    QHBoxLayout *careTitleLayout = new QHBoxLayout();
    careTitleLayout->setSpacing(10);


    QLabel *careTitle = new QLabel("Уход и параметры");
    careTitle->setStyleSheet("color: #8E3A4B; font-family: 'Century Gothic'; font-size: 14px; font-weight: bold; border: none; background: transparent;");

    careTitleLayout->addWidget(careTitle);
    careTitleLayout->addStretch();
    careLayout->addLayout(careTitleLayout);

    m_careText = new QTextBrowser();
    m_careText->setFixedHeight(55);
    m_careText->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_careText->setStyleSheet(TEXT_BROWSER_STYLE + SCROLL_STYLE);
    careLayout->addWidget(m_careText);

    m_mainLayout->addWidget(m_careBlock);

    // Убрали m_mainLayout->addStretch(1), чтобы блоки прижались к низу, а верх растянулся
}

void CatalogRightColumn::clearLayout(QLayout *layout)
{
    if (!layout) return;
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (QWidget *widget = item->widget()) {
            widget->hide();
            widget->setParent(nullptr);
            delete widget;
        }
        else if (QLayout *childLayout = item->layout()) {
            clearLayout(childLayout);
        }
        delete item;
    }
}

void CatalogRightColumn::loadItem(int itemId, int currentVariationId)
{
    loadColors(itemId, currentVariationId);
    loadInfoBlocks(itemId);
}

void CatalogRightColumn::loadColors(int itemId, int currentVariationId)
{
    clearLayout(m_colorsListLayout);

    QSqlQuery q;
    q.prepare(R"(
        SELECT V.variation_id, C.color_hex, C.color_name
        FROM Item_variation V
        LEFT JOIN Colors C ON V.color_id = C.color_id
        WHERE V.item_id = :id
        ORDER BY V.variation_id ASC
    )");
    q.bindValue(":id", itemId);

    if (!q.exec()) return;

    struct ColorData { int id; QString hex; QString name; };
    QList<ColorData> colors;
    while (q.next()) {
        colors.append({q.value(0).toInt(), q.value(1).toString(), q.value(2).toString()});
    }

    int totalColors = colors.size();
    if (totalColors == 0) return;

    int btnSize = 40;
    if (totalColors > 2) btnSize = 34;
    if (totalColors > 4) btnSize = 28;

    int borderRadius = btnSize / 2;

    for (int i = 0; i < totalColors; ++i) {
        int varId = colors[i].id;
        QString hex = colors[i].hex.isEmpty() ? "#CCCCCC" : colors[i].hex;
        QString name = colors[i].name;

        QHBoxLayout *rowLayout = new QHBoxLayout();
        rowLayout->setContentsMargins(4, 4, 4, 4);
        rowLayout->setSpacing(15);
        rowLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        QPushButton *colorBtn = new QPushButton();
        colorBtn->setFixedSize(btnSize, btnSize);
        colorBtn->setCursor(Qt::PointingHandCursor);

        bool isSelected = (varId == currentVariationId);

        QString borderStyle = isSelected ? "border: 1px solid #B0B0B0;" : "border: 1px solid #EAEAEA;";

        colorBtn->setStyleSheet(QString(
                                    "QPushButton { background-color: %1; border-radius: %2px; %3 }"
                                    "QPushButton:hover { border: 1px solid #8E3A4B; }"
                                    ).arg(hex).arg(borderRadius).arg(borderStyle));

        connect(colorBtn, &QPushButton::clicked, this, [this, varId]() {
            emit variationSelected(varId);
        });

        rowLayout->addWidget(colorBtn);

        if (isSelected && !name.isEmpty()) {
            QLabel *nameLbl = new QLabel(name);
            QString textColor = isDarkColor(hex) ? "#FFFFFF" : "#333333";

            nameLbl->setStyleSheet(QString(
                                       "background-color: %1; color: %2; "
                                       "border: 1px solid #EAEAEA; border-radius: 4px; padding: 4px 10px; "
                                       "font-family: 'Century Gothic'; font-size: 13px;"
                                       ).arg(hex, textColor));

            rowLayout->addWidget(nameLbl);
        }

        rowLayout->addStretch();
        m_colorsListLayout->addLayout(rowLayout);
    }
}

void CatalogRightColumn::loadInfoBlocks(int itemId)
{
    QString itemType = "";
    QSqlQuery typeQ;
    typeQ.prepare("SELECT item_type FROM Items WHERE item_id = :id");
    typeQ.bindValue(":id", itemId);
    if (typeQ.exec() && typeQ.next()) itemType = typeQ.value(0).toString();

    QString htmlStyle = "<style>p { margin: 0 0 5px 0; padding: 0; } b { color: #4A4A4A; font-weight: 600; }</style>";
    QString featuresStr = htmlStyle;
    QString careStr = htmlStyle;

    QSqlQuery q;
    if (itemType == "flower") {
        q.prepare("SELECT bud_size, scent_intensity, lifetime, care_complexity, stem_length_cm, is_poisonous FROM Flower_details WHERE item_id = :id");
        q.bindValue(":id", itemId);
        if (q.exec() && q.next()) {
            featuresStr += "<p><b>Размер бутона:</b> " + q.value(0).toString() + "</p>";
            featuresStr += "<p><b>Длина стебля:</b> до " + q.value(4).toString() + " см</p>";
            featuresStr += "<p><b>Аромат:</b> " + q.value(1).toString() + "</p>";
            featuresStr += "<p><b>Ядовитость:</b> " + q.value(5).toString() + " из 10</p>";

            careStr += "<p><b>Стойкость:</b> " + q.value(2).toString() + " дней в вазе</p>";
            careStr += "<p><b>Сложность ухода:</b> " + q.value(3).toString() + " из 10</p>";
        }
    }
    else if (itemType == "packaging") {
        q.prepare("SELECT material, is_waterproof, breathability, shock_absorption FROM Packaging_details WHERE item_id = :id");
        q.bindValue(":id", itemId);
        if (q.exec() && q.next()) {
            featuresStr += "<p><b>Материал:</b> " + q.value(0).toString() + "</p>";
            featuresStr += "<p><b>Воздухопроницаемость:</b> " + q.value(2).toString() + "</p>";

            careStr += "<p><b>Водостойкость:</b> " + QString(q.value(1).toInt() == 1 ? "Да (не раскисает в воде)" : "Нет (беречь от влаги)") + "</p>";
            careStr += "<p><b>Защита при падении:</b> " + q.value(3).toString() + "</p>";
        }
    }
    else if (itemType == "accessory") {
        q.prepare("SELECT attachment_method, durability, heat_sensitivity, design_notes, is_customizable FROM Accessories_details WHERE item_id = :id");
        q.bindValue(":id", itemId);
        if (q.exec() && q.next()) {
            featuresStr += "<p><b>Способ крепления:</b> " + q.value(0).toString() + "</p>";
            featuresStr += "<p><b>Кастомизация:</b> " + QString(q.value(4).toInt() == 1 ? "Возможна (нанесение текста/лого)" : "Не предусмотрена") + "</p>";
            featuresStr += "<p><b>Дизайн:</b> " + q.value(3).toString() + "</p>";

            careStr += "<p><b>Износостойкость:</b> " + q.value(1).toString() + "</p>";
            careStr += "<p><b>Термочувствительность:</b> " + q.value(2).toString() + "</p>";
        }
    }

    QSqlQuery tipsQ;
    tipsQ.prepare("SELECT instruction_steps, is_critical FROM Technique_Tips WHERE item_id = :id");
    tipsQ.bindValue(":id", itemId);
    if (tipsQ.exec() && tipsQ.next()) {
        bool isCritical = tipsQ.value(1).toInt() == 1;
        QString tipText = tipsQ.value(0).toString();

        if (isCritical) {
            careStr += "<p><b>Совет флориста:</b> <i><span style='color:#2C3E50;'>" + tipText + "</span></i></p>";
        } else {
            careStr += "<p><b>Совет флориста:</b> " + tipText + "</p>";
        }
    }

    featuresStr += "<div style='height: 8px;'></div>";
    careStr += "<div style='height: 8px;'></div>";

    if (featuresStr == htmlStyle + "<div style='height: 8px;'></div>") {
        m_featuresBlock->hide();
    } else {
        m_featuresText->setHtml(featuresStr);
        m_featuresBlock->show();
    }

    if (careStr == htmlStyle + "<div style='height: 8px;'></div>") {
        m_careBlock->hide();
    } else {
        m_careText->setHtml(careStr);
        m_careBlock->show();
    }
}