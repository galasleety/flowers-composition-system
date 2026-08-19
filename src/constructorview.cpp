#include "constructorview.h"
#include "constructorfilterdialog.h"
#include "catalogminicard.h"
#include "detailscolumn.h"
#include "vasecolumn.h"
#include "successdialog.h"
#include "savebouquetdialog.h"

#include <QLabel>
#include <QFrame>
#include <QScrollBar>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QBuffer>
#include <QMessageBox>

ConstructorView::ConstructorView(QWidget *parent)
    : QWidget(parent), m_currentUserId(-1), m_filtersActive(false)
{
    m_searchEdit = nullptr;
    m_btnCatalog = nullptr;
    m_btnFavorites = nullptr;
    m_btnFilters = nullptr;
    m_categoryGroup = nullptr;
    m_componentsGrid = nullptr;

    setStyleSheet("background-color: #FFFFFF;");
    m_activeSettings = FilterSettings();

    setupUI();
}

void ConstructorView::setCurrentUser(int userId) {
    m_currentUserId = userId;
    loadComponentsGrid();
}

void ConstructorView::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);

    // Создаем три основные области конструктора
    QWidget *leftCol = createLeftColumn();
    QWidget *centerCol = createCenterColumn();
    m_detailsColumn = new DetailsColumn(this);

    mainLayout->addWidget(leftCol, 0);   // Левая: палитра компонентов
    mainLayout->addWidget(centerCol, 1);  // Центр: холст (предпросмотр)
    mainLayout->addWidget(m_detailsColumn, 0); // Правая: анализ и состав

    // --- УСТАНОВКА СВЯЗЕЙ ---
    connect(m_vaseColumn, &VaseColumn::compositionChanged,
            m_detailsColumn, &DetailsColumn::updateAnalysis);

    connect(m_vaseColumn, &VaseColumn::saveRequested, this, &ConstructorView::onSaveBouquetRequested);

    if (m_btnCatalog) m_btnCatalog->setChecked(true);
}

QWidget* ConstructorView::createLeftColumn() {
    QWidget *panel = new QWidget(this);
    panel->setFixedWidth(370);
    panel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 10, 0);
    layout->setSpacing(15);
    layout->setAlignment(Qt::AlignTop);

    QLabel *title = new QLabel("Компоненты", panel);
    title->setStyleSheet("font-family: 'Century Gothic'; font-size: 16px; color: #444; font-weight: normal;");

    m_searchEdit = new QLineEdit(panel);
    m_searchEdit->setPlaceholderText("Поиск по палитре...");
    m_searchEdit->setFixedHeight(40);
    m_searchEdit->setStyleSheet("QLineEdit { background: white; border: 1px solid #E0E0E0; border-radius: 20px; padding: 0 15px 0 35px; font-family: 'Century Gothic'; font-size: 13px; }");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ConstructorView::onSearchChanged);

    QHBoxLayout *sLay = new QHBoxLayout();
    QString bStyle = "QPushButton { background: white; border: 1px solid #E0E0E0; border-radius: 20px; height: 40px; font-family: 'Century Gothic'; font-size: 14px; color: #555; } "
                     "QPushButton:checked { background: #F8C3CD; color: #111; border: none; font-weight: bold; }";

    m_btnCatalog = new QPushButton("Каталог", panel);
    m_btnCatalog->setCheckable(true);
    m_btnCatalog->setStyleSheet(bStyle);

    m_btnFavorites = new QPushButton("Избранное", panel);
    m_btnFavorites->setCheckable(true);
    m_btnFavorites->setStyleSheet(bStyle);

    m_sourceGroup = new QButtonGroup(this);
    m_sourceGroup->addButton(m_btnCatalog, 0);
    m_sourceGroup->addButton(m_btnFavorites, 1);
    connect(m_sourceGroup, &QButtonGroup::idToggled, this, &ConstructorView::onSourceToggled);

    sLay->addWidget(m_btnCatalog);
    sLay->addWidget(m_btnFavorites);

    m_btnFilters = new QPushButton("Настроить фильтры", panel);
    m_btnFilters->setFixedHeight(40);
    m_btnFilters->setStyleSheet("QPushButton { background: white; border: 1px solid #E0E0E0; border-radius: 20px; font-family: 'Century Gothic'; font-size: 14px; }");
    connect(m_btnFilters, &QPushButton::clicked, this, &ConstructorView::onFiltersClicked);

    QWidget *catW = new QWidget(panel);
    QHBoxLayout *catL = new QHBoxLayout(catW);
    catL->setContentsMargins(0,0,0,0);
    m_categoryGroup = new QButtonGroup(this);
    QStringList cats = {"Основные", "Стаффаж", "Упаковка", "Аксессуары"};
    for(int i=0; i<4; ++i) {
        QPushButton *b = new QPushButton(cats[i], catW);
        b->setCheckable(true);
        b->setStyleSheet("QPushButton { border: none; color: #888; padding: 5px; font-family: 'Century Gothic'; font-size: 13px; } QPushButton:checked { color: #333; font-weight: bold; border-bottom: 2px solid #F8C3CD; }");
        m_categoryGroup->addButton(b, i);
        catL->addWidget(b);
        if(i==0) b->setChecked(true);
    }
    connect(m_categoryGroup, &QButtonGroup::idToggled, this, &ConstructorView::onCategoryToggled);

    QScrollArea *scroll = new QScrollArea(this);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("background: transparent; border: none;");

    QString scrollStyle =
        "QScrollBar:vertical { border: none; background: #F5F5F5; width: 8px; margin: 0px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #BDBDBD; border-radius: 4px; min-height: 25px; }"
        "QScrollBar::handle:vertical:hover { background: #A0A0A0; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; border: none; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }";
    scroll->verticalScrollBar()->setStyleSheet(scrollStyle);

    QWidget *container = new QWidget();
    m_componentsGrid = new QGridLayout(container);
    m_componentsGrid->setContentsMargins(0, 10, 10, 10);
    m_componentsGrid->setSpacing(15);
    m_componentsGrid->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    scroll->setWidget(container);

    layout->addWidget(title);
    layout->addWidget(m_searchEdit);
    layout->addLayout(sLay);
    layout->addWidget(m_btnFilters);
    layout->addWidget(catW);
    layout->addWidget(scroll, 1);

    return panel;
}

QWidget* ConstructorView::createCenterColumn() {
    m_vaseColumn = new VaseColumn(this);
    return m_vaseColumn;
}

void ConstructorView::onSaveBouquetRequested() {
    if (m_currentUserId <= 0) return;

    QMap<int, int> components = m_vaseColumn->getComponentCounts();
    if (components.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Букет пуст. Добавьте элементы.");
        return;
    }

    AnalysisData data;
    data.harmony = m_detailsColumn->getHarmonyValue();
    data.harmonyDesc = m_detailsColumn->getHarmonyDesc();
    data.compatibility = m_detailsColumn->getCompatibilityValue();
    data.compatibilityDesc = m_detailsColumn->getCompatibilityDesc();
    data.style = m_detailsColumn->getStyleValue();
    data.styleDesc = m_detailsColumn->getStyleDesc();
    data.life = m_detailsColumn->getLifeValue();
    data.lifeDesc = m_detailsColumn->getLifeDesc();
    data.scent = m_detailsColumn->getScentValue();
    data.scentDesc = m_detailsColumn->getScentDesc();
    data.tox = m_detailsColumn->getToxValue();
    data.toxDesc = m_detailsColumn->getToxDesc();

    QWidget *overlay = new QWidget(this->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->setGeometry(this->window()->rect());
    overlay->show();

    SaveBouquetDialog dialog(data, this);
    dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());

    if (dialog.exec() == QDialog::Accepted) {
        QSqlDatabase db = QSqlDatabase::database();
        if (db.transaction()) {
            QSqlQuery q;
            q.prepare("INSERT INTO SavedBouquets (user_id, bouquet_name, image_data) VALUES (:uid, :name, :img)");
            q.bindValue(":uid", m_currentUserId);
            q.bindValue(":name", dialog.getBouquetName());
            q.bindValue(":img", m_vaseColumn->getSnapshot());

            if (q.exec()) {
                int bid = q.lastInsertId().toInt();
                bool compError = false;

                q.prepare("INSERT INTO BouquetComponent (bouquet_id, variation_id, quantity) VALUES (:bid, :vid, :qty)");
                for (auto it = components.begin(); it != components.end(); ++it) {
                    q.bindValue(":bid", bid);
                    q.bindValue(":vid", it.key());
                    q.bindValue(":qty", it.value());
                    if (!q.exec()) {
                        compError = true;
                        break;
                    }
                }

                if (!compError && db.commit()) {
                    SuccessDialog success("Ваш букет успешно сохранен!", this);
                    success.move(this->window()->frameGeometry().center() - success.rect().center());
                    success.exec();
                } else {
                    db.rollback();
                    QMessageBox::critical(this, "Ошибка", "Не удалось сохранить состав букета.");
                }
            } else {
                db.rollback();
                QMessageBox::critical(this, "Ошибка", "Ошибка при записи в базу данных.");
            }
        }
    }
    overlay->deleteLater();
}

void ConstructorView::loadComponentsGrid() {
    if (!m_componentsGrid || !m_categoryGroup || !m_searchEdit || !m_btnFavorites) return;

    QLayoutItem *item;
    while ((item = m_componentsGrid->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    int catIdx = m_categoryGroup->checkedId();

    // ДОБАВЛЯЕМ v.image_back
    QString queryStr = "SELECT v.variation_id, i.name, v.image, v.image_back FROM Item_variation v JOIN Items i ON v.item_id = i.item_id ";
    QStringList conditions;

    if (m_btnFavorites->isChecked()) {
        queryStr += " JOIN Favorites f ON v.variation_id = f.variation_id ";
        conditions << QString("f.user_id = %1").arg(m_currentUserId);
    }

    if (catIdx <= 1) {
        queryStr += " LEFT JOIN Flower_details d ON i.item_id = d.item_id ";
        conditions << (catIdx == 0 ? "d.subtype = 'main'" : "d.subtype = 'filler'");
    } else if (catIdx == 2) {
        queryStr += " LEFT JOIN Packaging_details p ON i.item_id = p.item_id ";
        conditions << "i.item_type = 'packaging'";
    } else if (catIdx == 3) {
        queryStr += " LEFT JOIN Accessories_details a ON i.item_id = a.item_id ";
        conditions << "i.item_type = 'accessory'";
    }

    // =========================================================
    // ТВОИ РОДНЫЕ ФИЛЬТРЫ И ПОИСК
    // =========================================================
    QString searchRaw = m_searchEdit->text().trimmed();
    if (!searchRaw.isEmpty()) {
        conditions << "(i.name LIKE :searchLow OR i.name LIKE :searchUp OR i.name LIKE :searchCap)";
    }

    if (m_filtersActive) {
        if (m_activeSettings.style != "Все") conditions << QString("i.style = '%1'").arg(m_activeSettings.style);
        if (m_activeSettings.color != "Все") conditions << QString("v.color_id = (SELECT color_id FROM Colors WHERE color_name='%1')").arg(m_activeSettings.color);

        if (catIdx <= 1) {
            if (m_activeSettings.type != "Все") {
                QString st = (m_activeSettings.type == "Основные") ? "main" : "filler";
                conditions << QString("d.subtype = '%1'").arg(st);
            }
            if (m_activeSettings.pollen != "Все") conditions << QString("d.pollen_level = '%1'").arg(m_activeSettings.pollen);
            if (m_activeSettings.scent != "Все") conditions << QString("d.scent_intensity = '%1'").arg(m_activeSettings.scent);
            if (m_activeSettings.bud != "Все") conditions << QString("d.bud_size = '%1'").arg(m_activeSettings.bud);
            if (m_activeSettings.stem != "Все") conditions << QString("d.stem_length_cm = %1").arg(m_activeSettings.stem);
            if (m_activeSettings.durability != "Все") conditions << QString("d.lifetime = %1").arg(m_activeSettings.durability);
            if (m_activeSettings.care != "Все") conditions << QString("d.care_complexity = %1").arg(m_activeSettings.care);
            if (m_activeSettings.safety != "Все") conditions << QString("d.is_poisonous = %1").arg(m_activeSettings.safety == "Да" ? 0 : 1);
        } else if (catIdx == 2) {
            if (m_activeSettings.material != "Все") conditions << QString("p.material = '%1'").arg(m_activeSettings.material);
            if (m_activeSettings.waterproof != "Все") conditions << QString("p.is_waterproof = %1").arg(m_activeSettings.waterproof == "Да" ? 1 : 0);
        } else if (catIdx == 3) {
            if (m_activeSettings.attachment != "Все") conditions << QString("a.attachment_method = '%1'").arg(m_activeSettings.attachment);
            if (m_activeSettings.customizable != "Все") conditions << QString("a.is_customizable = %1").arg(m_activeSettings.customizable == "Да" ? 1 : 0);
        }
    }

    if (!conditions.isEmpty()) queryStr += " WHERE " + conditions.join(" AND ");

    QSqlQuery q;
    q.prepare(queryStr);

    if (!searchRaw.isEmpty()) {
        QString lower = searchRaw.toLower();
        QString upper = searchRaw.toUpper();
        QString cap = lower;
        if (!cap.isEmpty()) {
            cap[0] = cap[0].toUpper();
        }

        q.bindValue(":searchLow", "%" + lower + "%");
        q.bindValue(":searchUp", "%" + upper + "%");
        q.bindValue(":searchCap", "%" + cap + "%");
    }

    q.exec();

    int row = 0, col = 0;
    while (q.next()) {
        int varId = q.value(0).toInt();
        QString name = q.value(1).toString();
        QString imgFront = q.value(2).toString().trimmed();
        QString imgBack = q.value(3).toString().trimmed();

        CatalogMiniCard *card = new CatalogMiniCard(varId, name, imgFront, true, this);

        card->setDoubleImage(imgBack);
        card->setProperty("imgBack", imgBack);

        // ИСПРАВЛЕННОЕ ПОДКЛЮЧЕНИЕ: Теперь работает идеально и не крашит сборку
        if (m_vaseColumn) {
            connect(card, &CatalogMiniCard::itemClicked, this, [this, card](int id, const QString &front) {
                QString back = card->property("imgBack").toString();
                m_vaseColumn->addItemToCenter(id, front, back);
            });
        }

        m_componentsGrid->addWidget(card, row, col);
        if (++col > 1) { col = 0; row++; }
    }
}

void ConstructorView::onFiltersClicked() {
    if (m_filtersActive) {
        m_filtersActive = false;
        m_activeSettings = FilterSettings();
        m_btnFilters->setText("Настроить фильтры");
        m_btnFilters->setStyleSheet("QPushButton { background: white; border: 1px solid #E0E0E0; border-radius: 20px; font-family: 'Century Gothic'; font-size: 14px; color: #555; }");
        loadComponentsGrid();
    } else {
        ConstructorFilterDialog dialog(m_categoryGroup->checkedId(), m_activeSettings, this);
        if (dialog.exec() == QDialog::Accepted) {
            m_filtersActive = true;
            m_activeSettings = dialog.getSettings();
            m_btnFilters->setText("Сбросить фильтры");
            m_btnFilters->setStyleSheet("QPushButton { background: #FDFDFD; color: #8E3A4B; border: 2px solid #8E3A4B; border-radius: 20px; font-weight: bold; font-family: 'Century Gothic'; font-size: 14px; }");
            loadComponentsGrid();
        }
    }
}

void ConstructorView::onSourceToggled(int, bool checked) { if(checked) loadComponentsGrid(); }
void ConstructorView::onSearchChanged(const QString&) { loadComponentsGrid(); }
void ConstructorView::onCategoryToggled(int, bool checked) {
    if(checked) {
        m_filtersActive = false;
        m_activeSettings = FilterSettings();
        if(m_btnFilters) {
            m_btnFilters->setText("Настроить фильтры");
            m_btnFilters->setStyleSheet("QPushButton { background: white; border: 1px solid #E0E0E0; border-radius: 20px; font-family: 'Century Gothic'; font-size: 14px; color: #555; }");
        }
        loadComponentsGrid();
    }
}