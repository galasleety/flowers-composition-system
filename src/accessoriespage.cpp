#include "accessoriespage.h"
#include "errordialog.h"
#include "addvariationdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QFrame>
#include <QIcon>
#include <QDebug>
#include <QScrollBar>
#include <QFont>
#include <QLineEdit>

AccessoriesPage::AccessoriesPage(QWidget *parent) : QWidget(parent), m_isEditing(false), m_isAddingNew(false) {
    setStyleSheet("background-color: #FFFFFF;");
    setupUI();
    loadAccessories();
}

void AccessoriesPage::setupUI() {
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

    QLabel *titleLabel = new QLabel("КАТАЛОГ АКСЕССУАРОВ", leftPanel);
    titleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; color: #333;");
    leftLayout->addWidget(titleLabel);

    m_searchEdit = new QLineEdit(leftPanel);
    m_searchEdit->setPlaceholderText("Поиск...");
    m_searchEdit->setFixedHeight(38);
    m_searchEdit->setStyleSheet("QLineEdit { border: 1px solid #D86B7A; border-radius: 19px; padding-left: 15px; font-family: 'Century Gothic'; }");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AccessoriesPage::onSearchTextChanged);
    leftLayout->addWidget(m_searchEdit);

    m_accessoriesList = new QListWidget(leftPanel);
    m_accessoriesList->setFrameShape(QFrame::NoFrame);
    m_accessoriesList->setFocusPolicy(Qt::NoFocus);
    m_accessoriesList->setIconSize(QSize(30, 30));
    m_accessoriesList->setStyleSheet(
        "QListWidget { background: transparent; outline: none; border: none; } "
        "QListWidget::item { height: 50px; border-radius: 12px; color: #555555; padding-left: 5px; font-family: 'Century Gothic'; font-size: 15px; } "
        "QListWidget::item:hover { background-color: #FFF0F3; color: #333333; } "
        "QListWidget::item:selected { background-color: #FFF0F3; color: #000000; font-weight: bold; border: none; outline: none; } "
        "QScrollBar:vertical { border: none; background: transparent; width: 4px; margin: 0px; } "
        "QScrollBar::handle:vertical { background: #DCDCDC; border-radius: 2px; min-height: 30px; } "
        "QScrollBar::handle:vertical:hover { background: #D86B7A; } "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; } "
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        );
    connect(m_accessoriesList, &QListWidget::itemSelectionChanged, this, &AccessoriesPage::onAccessorySelected);
    leftLayout->addWidget(m_accessoriesList);

    m_btnAddAccessory = new QPushButton("+ Добавить аксессуар", leftPanel);
    m_btnAddAccessory->setFixedHeight(38);
    m_btnAddAccessory->setCursor(Qt::PointingHandCursor);
    m_btnAddAccessory->setStyleSheet("QPushButton { border: 1px solid #DCDCDC; border-radius: 12px; font-family: 'Century Gothic'; font-size: 12px; color: #555; } QPushButton:hover { background: #F5F5F5; }");
    connect(m_btnAddAccessory, &QPushButton::clicked, this, &AccessoriesPage::onAddAccessoryClicked);
    leftLayout->addWidget(m_btnAddAccessory);

    QFrame *vSeparator = new QFrame(this);
    vSeparator->setFrameShape(QFrame::VLine);
    vSeparator->setStyleSheet("background-color: #E8E8E8; border: none; width: 1px; margin: 25px 0;");

    // ==========================================
    // ПРАВАЯ ЧАСТЬ: ОБЩИЙ СКРОЛЛ
    // ==========================================
    m_rightPanel = new QWidget(this);
    QVBoxLayout *rightMainLayout = new QVBoxLayout(m_rightPanel);
    rightMainLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *mainScroll = new QScrollArea(m_rightPanel);
    mainScroll->setWidgetResizable(true);
    mainScroll->setFrameShape(QFrame::NoFrame);
    mainScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainScroll->setStyleSheet(
        "QScrollArea { background: transparent; } "
        "QScrollBar:vertical { border: none; background: transparent; width: 6px; margin: 0px; } "
        "QScrollBar::handle:vertical { background: #DCDCDC; border-radius: 3px; min-height: 30px; } "
        "QScrollBar::handle:vertical:hover { background: #D86B7A; } "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; } "
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        );

    QWidget *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout *rightLayout = new QVBoxLayout(scrollContent);
    rightLayout->setContentsMargins(30, 20, 30, 30);
    rightLayout->setAlignment(Qt::AlignTop);

    // --- 1. ШАПКА КАРТОЧКИ ---
    m_infoCard = new QFrame(scrollContent);
    m_infoCard->setFixedHeight(220);
    m_infoCard->setObjectName("infoCard");
    m_infoCard->setStyleSheet("QFrame#infoCard { border: 2px solid #DCDCDC; border-radius: 15px; background-color: white; }");

    QHBoxLayout *cardLayout = new QHBoxLayout(m_infoCard);
    cardLayout->setContentsMargins(20, 20, 20, 20);
    cardLayout->setSpacing(25);

    m_iconLabel = new QLabel(m_infoCard);
    m_iconLabel->setFixedSize(140, 140);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet("border: 1px solid #EEE; border-radius: 12px; background: #FFFFFF;");

    m_textStack = new QStackedWidget(m_infoCard);
    m_textStack->setStyleSheet("background: transparent; border: none;");

    QWidget *vW = new QWidget();
    QVBoxLayout *vL = new QVBoxLayout(vW); vL->setContentsMargins(0,0,0,0); vL->setSpacing(8);
    m_nameLabel = new QLabel("Название аксессуара", vW);
    m_nameLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 22px; font-weight: bold; color: #000; border: none;");
    m_descLabel = new QLabel("Описание...", vW);
    m_descLabel->setWordWrap(true);
    m_descLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; color: #555; border: none; line-height: 1.3;");
    m_idLabel = new QLabel("ID: #0", vW);
    m_idLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; color: #AAA; font-weight: bold; border: none;");
    vL->addWidget(m_nameLabel); vL->addWidget(m_descLabel, 1); vL->addWidget(m_idLabel);

    QWidget *eW = new QWidget();
    QVBoxLayout *eL = new QVBoxLayout(eW); eL->setContentsMargins(0,0,0,0); eL->setSpacing(10);

    m_nameEditField = new QLineEdit(eW);
    m_nameEditField->setMinimumWidth(180);
    m_nameEditField->setPlaceholderText("Название аксессуара...");
    m_nameEditField->setStyleSheet("border: 1px solid #DCDCDC; border-radius: 6px; font-family: 'Century Gothic'; font-weight: bold; padding: 5px;");

    m_descEditField = new QTextEdit(eW);
    m_descEditField->setMinimumWidth(180);
    m_descEditField->setPlaceholderText("Описание аксессуара...");
    m_descEditField->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_descEditField->setStyleSheet("QTextEdit { border: 1px solid #DCDCDC; border-radius: 6px; font-family: 'Century Gothic'; font-size: 12px; padding: 5px; background: white; }");

    eL->addWidget(m_nameEditField); eL->addWidget(m_descEditField, 1);
    m_textStack->addWidget(vW); m_textStack->addWidget(eW);

    QVBoxLayout *btnContainer = new QVBoxLayout();
    btnContainer->setSpacing(10);
    btnContainer->setAlignment(Qt::AlignTop);

    m_btnEditAccessory = new QPushButton("Редактировать", m_infoCard);
    m_btnEditAccessory->setFixedSize(125, 36);
    m_btnEditAccessory->setCursor(Qt::PointingHandCursor);
    m_btnEditAccessory->setStyleSheet("QPushButton { border: 2px solid #DCDCDC; border-radius: 18px; font-family: 'Century Gothic'; font-size: 12px; } QPushButton:hover { background: #F5F5F5; }");
    connect(m_btnEditAccessory, &QPushButton::clicked, this, &AccessoriesPage::onEditAccessoryClicked);

    m_btnDeleteAccessory = new QPushButton("Удалить", m_infoCard);
    m_btnDeleteAccessory->setFixedSize(60, 36);
    m_btnDeleteAccessory->setCursor(Qt::PointingHandCursor);
    m_btnDeleteAccessory->setStyleSheet("QPushButton { border: none; color: #D86B7A; font-family: 'Century Gothic'; font-size: 12px; } QPushButton:hover { text-decoration: underline; }");
    connect(m_btnDeleteAccessory, &QPushButton::clicked, this, &AccessoriesPage::onDeleteAccessoryClicked);

    btnContainer->addWidget(m_btnEditAccessory);
    btnContainer->addWidget(m_btnDeleteAccessory, 0, Qt::AlignCenter);

    cardLayout->addWidget(m_iconLabel);
    cardLayout->addWidget(m_textStack, 1);
    cardLayout->addLayout(btnContainer);
    rightLayout->addWidget(m_infoCard);

    // --- 2. СЕТКА ХАРАКТЕРИСТИК ---
    rightLayout->addSpacing(15);
    QLabel *charTitle = new QLabel("ХАРАКТЕРИСТИКИ", scrollContent);
    charTitle->setAlignment(Qt::AlignCenter);
    charTitle->setObjectName("charTitle");
    charTitle->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; font-weight: bold; color: #AAA; letter-spacing: 2px;");
    rightLayout->addWidget(charTitle);

    m_detailsStack = new QStackedWidget(scrollContent);

    // Сетка ПРОСМОТРА
    QWidget *viewGridWidget = new QWidget();
    QGridLayout *viewGrid = new QGridLayout(viewGridWidget);
    viewGrid->setSpacing(20);
    viewGrid->setContentsMargins(20, 15, 20, 15);

    auto addViewStat = [&](const QString &title, QLabel* &valLabel, int r, int c) {
        QVBoxLayout *v = new QVBoxLayout();
        QLabel *t = new QLabel(title);
        t->setStyleSheet("font-family: 'Century Gothic'; font-size: 10px; font-weight: bold; color: #333; text-transform: uppercase;");
        t->setAlignment(Qt::AlignCenter);
        valLabel = new QLabel("-");
        valLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 14px; color: #555;");
        valLabel->setAlignment(Qt::AlignCenter);
        v->addWidget(t);
        v->addWidget(valLabel);
        viewGrid->addLayout(v, r, c);
    };

    addViewStat("Кастомизация", m_valCustomizable, 0, 0);
    addViewStat("Способ крепления", m_valAttachment, 0, 1);
    addViewStat("Долговечность", m_valDurability, 1, 0);
    addViewStat("Чувств. к теплу", m_valHeat, 1, 1);
    addViewStat("Особенности дизайна", m_valNotes, 2, 0); // В третьем ряду

    // Сетка РЕДАКТИРОВАНИЯ
    QWidget *editGridWidget = new QWidget();
    QGridLayout *editGrid = new QGridLayout(editGridWidget);
    editGrid->setSpacing(20);
    editGrid->setContentsMargins(20, 15, 20, 15);

    QString comboStyle = "QComboBox { border: 1px solid #DCDCDC; border-radius: 6px; font-family: 'Century Gothic'; font-size: 12px; background: white; padding: 0 10px; } "
                         "QComboBox::drop-down { border: none; width: 25px; } "
                         "QComboBox QAbstractItemView { border: 1px solid #DCDCDC; selection-background-color: #FFF0F3; selection-color: black; } "
                         "QLineEdit { background: transparent; border: none; color: #333; }";

    auto addEditStatCombo = [&](const QString &title, QComboBox* combo, int r, int c) {
        QVBoxLayout *v = new QVBoxLayout();
        QLabel *t = new QLabel(title);
        t->setStyleSheet("font-family: 'Century Gothic'; font-size: 10px; font-weight: bold; color: #333; text-transform: uppercase;");
        t->setAlignment(Qt::AlignCenter);
        combo->setMinimumWidth(160);
        combo->setFixedHeight(28);
        combo->setStyleSheet(comboStyle);

        combo->setEditable(true);
        combo->lineEdit()->setReadOnly(true);
        combo->lineEdit()->setAlignment(Qt::AlignCenter);

        v->addWidget(t);
        v->addWidget(combo);
        editGrid->addLayout(v, r, c);
    };

    m_comboCustomizable = new QComboBox();
    m_comboCustomizable->addItem("Нет", 0);
    m_comboCustomizable->addItem("Да", 1);

    m_comboAttachment = new QComboBox();
    m_comboAttachment->addItems({"Без крепления", "Шпажка", "Проволока", "Клей", "Лента", "Прищепка", "Зажим"});

    m_comboDurability = new QComboBox();
    m_comboDurability->addItems({"Одноразовый", "Многоразовый", "Хрупкий", "Прочный", "Очень прочный"});

    m_comboHeat = new QComboBox();
    m_comboHeat->addItems({"Нет", "Низкая", "Средняя", "Высокая", "Плавится"});

    m_comboNotes = new QComboBox();
    m_comboNotes->addItems({"Без особенностей", "Ручная работа", "Сложная форма", "Глиттер/Блестки", "Минимализм", "Винтаж", "Тематический"});

    addEditStatCombo("Кастомизация", m_comboCustomizable, 0, 0);
    addEditStatCombo("Способ крепления", m_comboAttachment, 0, 1);
    addEditStatCombo("Долговечность", m_comboDurability, 1, 0);
    addEditStatCombo("Чувств. к теплу", m_comboHeat, 1, 1);
    addEditStatCombo("Особенности дизайна", m_comboNotes, 2, 0); // Третий ряд

    m_detailsStack->addWidget(viewGridWidget);
    m_detailsStack->addWidget(editGridWidget);
    rightLayout->addWidget(m_detailsStack);

    // --- 3. ВАРИАЦИИ ---
    rightLayout->addSpacing(25);

    QHBoxLayout *varHeaderLayout = new QHBoxLayout();
    m_varTitle = new QLabel("ВАРИАЦИИ АКСЕССУАРА", scrollContent);
    m_varTitle->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; font-weight: bold; color: #AAA; letter-spacing: 2px;");

    m_btnAddVariation = new QPushButton("+ Добавить вариацию", scrollContent);
    m_btnAddVariation->setFixedSize(140, 26);
    m_btnAddVariation->setCursor(Qt::PointingHandCursor);
    m_btnAddVariation->setStyleSheet("QPushButton { border: 1px solid #DCDCDC; border-radius: 13px; font-family: 'Century Gothic'; font-size: 11px; color: #555; background: white; } QPushButton:hover { background: #F5F5F5; }");
    connect(m_btnAddVariation, &QPushButton::clicked, this, &AccessoriesPage::onAddVariationClicked);

    varHeaderLayout->addWidget(m_varTitle);
    varHeaderLayout->addStretch();
    varHeaderLayout->addWidget(m_btnAddVariation);
    rightLayout->addLayout(varHeaderLayout);

    m_varScrollArea = new QScrollArea(scrollContent);
    m_varScrollArea->setFixedHeight(160);
    m_varScrollArea->setWidgetResizable(true);
    m_varScrollArea->setFrameShape(QFrame::NoFrame);
    m_varScrollArea->setStyleSheet(
        "QScrollArea { background: transparent; } "
        "QScrollBar:horizontal { height: 6px; background: transparent; } "
        "QScrollBar::handle:horizontal { background: #DCDCDC; border-radius: 3px; } "
        "QScrollBar::handle:horizontal:hover { background: #D86B7A; }"
        );

    m_variationsContainer = new QWidget();
    m_variationsContainer->setStyleSheet("background: transparent;");
    m_variationsLayout = new QHBoxLayout(m_variationsContainer);
    m_variationsLayout->setContentsMargins(0, 5, 0, 5);
    m_variationsLayout->setSpacing(15);
    m_variationsLayout->setAlignment(Qt::AlignLeft);

    m_varScrollArea->setWidget(m_variationsContainer);
    rightLayout->addWidget(m_varScrollArea);

    rightLayout->addStretch(1);

    mainScroll->setWidget(scrollContent);
    rightMainLayout->addWidget(mainScroll);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(vSeparator);
    mainLayout->addWidget(m_rightPanel, 1);

    scrollContent->hide();
}

void AccessoriesPage::loadAccessories() {
    m_accessoriesList->clear();
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return;

    QSqlQuery q(db);
    if (q.exec("SELECT item_id, name FROM Items WHERE item_type = 'accessory' ORDER BY name")) {
        while (q.next()) {
            QListWidgetItem *item = new QListWidgetItem("   " + q.value(1).toString());
            item->setData(Qt::UserRole, q.value(0).toInt());
            item->setIcon(QIcon(":/sources/icons/ribbon.png")); // Замени на свою иконку аксессуаров
            item->setFont(QFont("Century Gothic", 11));
            m_accessoriesList->addItem(item);
        }
    }
}

// ==========================================
// ЛОГИКА ВЗАИМОДЕЙСТВИЯ
// ==========================================

void AccessoriesPage::onAccessorySelected() {
    QListWidgetItem *item = m_accessoriesList->currentItem();
    if (!item) {
        m_rightPanel->findChild<QScrollArea*>()->widget()->hide();
        return;
    }

    m_rightPanel->findChild<QScrollArea*>()->widget()->show();

    m_isEditing = false;
    m_isAddingNew = false;
    m_btnDeleteAccessory->show();

    m_varScrollArea->show();
    m_varTitle->show();
    m_btnAddVariation->show();

    m_textStack->setCurrentIndex(0);
    m_detailsStack->setCurrentIndex(0);
    m_btnEditAccessory->setText("Редактировать");
    m_btnEditAccessory->setStyleSheet("QPushButton { border: 2px solid #DCDCDC; border-radius: 18px; font-family: 'Century Gothic'; font-size: 12px; color: #333; } QPushButton:hover { background: #F5F5F5; }");

    int id = item->data(Qt::UserRole).toInt();
    QSqlQuery q;

    q.prepare("SELECT i.name, i.description, "
              "(SELECT image FROM Item_variation WHERE item_id = i.item_id LIMIT 1), "
              "ad.is_customizable, ad.attachment_method, ad.durability, ad.heat_sensitivity, ad.design_notes "
              "FROM Items i LEFT JOIN Accessories_details ad ON i.item_id = ad.item_id "
              "WHERE i.item_id = :id");
    q.bindValue(":id", id);

    if (q.exec() && q.next()) {
        QString name = q.value(0).toString();
        QString desc = q.value(1).toString();
        QString imgPath = q.value(2).toString();

        m_nameLabel->setText(name);
        m_descLabel->setText(desc.isEmpty() ? "Описание отсутствует." : desc);
        m_idLabel->setText(QString("ID: #%1").arg(id));
        m_nameEditField->setText(name);
        m_descEditField->setPlainText(desc);

        int isCustom = q.value(3).toInt();
        QString attachment = q.value(4).toString();
        QString durability = q.value(5).toString();
        QString heat = q.value(6).toString();
        QString notes = q.value(7).toString();

        m_valCustomizable->setText(isCustom == 1 ? "Да" : "Нет");
        m_valAttachment->setText(attachment.isEmpty() ? "-" : attachment);
        m_valDurability->setText(durability.isEmpty() ? "-" : durability);
        m_valHeat->setText(heat.isEmpty() ? "-" : heat);
        m_valNotes->setText(notes.isEmpty() ? "-" : notes);

        m_comboCustomizable->setCurrentIndex(isCustom == 1 ? 1 : 0);
        m_comboAttachment->setCurrentText(attachment);
        m_comboDurability->setCurrentText(durability);
        m_comboHeat->setCurrentText(heat);
        m_comboNotes->setCurrentText(notes);

        m_iconLabel->setStyleSheet("border: 1px solid #EEE; border-radius: 12px; background: #FFFFFF;");

        if (!imgPath.isEmpty()) {
            QPixmap pix(imgPath);
            if (!pix.isNull()) {
                qreal dpr = this->devicePixelRatioF();
                QPixmap scaledPix = pix.scaled(m_iconLabel->size() * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                scaledPix.setDevicePixelRatio(dpr);
                m_iconLabel->setPixmap(scaledPix);
            } else {
                m_iconLabel->setText("No Img");
                m_iconLabel->setStyleSheet("border: 1px solid #EEE; border-radius: 12px; background: #FFFFFF; color: #AAA; font-family: 'Century Gothic';");
            }
        } else {
            m_iconLabel->setText("No Img");
            m_iconLabel->setStyleSheet("border: 1px solid #EEE; border-radius: 12px; background: #FFFFFF; color: #AAA; font-family: 'Century Gothic';");
        }

        loadVariations(id);
    }
}

void AccessoriesPage::onAddAccessoryClicked() {
    m_accessoriesList->clearSelection();
    m_isEditing = true;
    m_isAddingNew = true;

    m_pendingVariations.clear();

    m_rightPanel->findChild<QScrollArea*>()->widget()->show();
    m_infoCard->show();
    m_detailsStack->show();

    QLabel *charTitle = m_rightPanel->findChild<QLabel*>("charTitle");
    if (charTitle) charTitle->show();

    m_varScrollArea->show();
    m_varTitle->show();
    m_btnAddVariation->show();
    renderPendingVariations();

    m_nameEditField->clear();
    m_descEditField->clear();

    m_comboCustomizable->setCurrentIndex(0);
    m_comboAttachment->setCurrentIndex(0);
    m_comboDurability->setCurrentIndex(0);
    m_comboHeat->setCurrentIndex(0);
    m_comboNotes->setCurrentIndex(0);

    m_iconLabel->clear();
    m_iconLabel->setText("Новое\nфото");
    m_iconLabel->setStyleSheet("border: 1px dashed #AAA; border-radius: 12px; background: #FFFFFF; color: #AAA; font-family: 'Century Gothic'; font-weight: bold;");

    m_textStack->setCurrentIndex(1);
    m_detailsStack->setCurrentIndex(1);

    m_btnEditAccessory->setText("Сохранить");
    m_btnEditAccessory->setStyleSheet("QPushButton { border: none; border-radius: 18px; color: white; background: #D86B7A; font-family: 'Century Gothic'; font-weight: bold; }");
    m_btnDeleteAccessory->hide();
}

void AccessoriesPage::onEditAccessoryClicked() {
    if (!m_isEditing) {
        m_isEditing = true;
        m_isAddingNew = false;
        m_textStack->setCurrentIndex(1);
        m_detailsStack->setCurrentIndex(1);
        m_btnEditAccessory->setText("Сохранить");
        m_btnEditAccessory->setStyleSheet("QPushButton { border: none; border-radius: 18px; color: white; background: #D86B7A; font-family: 'Century Gothic'; font-weight: bold; }");
    } else {
        QString newName = m_nameEditField->text().trimmed();
        QString newDesc = m_descEditField->toPlainText().trimmed();

        if (newName.isEmpty() || newDesc.isEmpty()) {
            QWidget *overlay = new QWidget(this->window());
            overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
            overlay->resize(this->window()->size());
            overlay->move(0, 0);
            overlay->show();

            ErrorDialog err("Ошибка", "Заполните название и описание аксессуара!", this->window());
            err.move(this->window()->frameGeometry().center() - err.rect().center());
            err.exec();

            overlay->deleteLater();
            return;
        }

        QSqlDatabase db = QSqlDatabase::database();
        db.transaction();

        if (m_isAddingNew) {
            QSqlQuery qItems(db);
            qItems.prepare("INSERT INTO Items (name, description, item_type) VALUES (:n, :d, 'accessory')");
            qItems.bindValue(":n", newName);
            qItems.bindValue(":d", newDesc);

            if (qItems.exec()) {
                int newId = qItems.lastInsertId().toInt();

                QSqlQuery qDetails(db);
                qDetails.prepare("INSERT INTO Accessories_details (item_id, is_customizable, attachment_method, durability, heat_sensitivity, design_notes) "
                                 "VALUES (:id, :cust, :att, :dur, :heat, :notes)");
                qDetails.bindValue(":id", newId);
                qDetails.bindValue(":cust", m_comboCustomizable->currentData().toInt());
                qDetails.bindValue(":att", m_comboAttachment->currentText());
                qDetails.bindValue(":dur", m_comboDurability->currentText());
                qDetails.bindValue(":heat", m_comboHeat->currentText());
                qDetails.bindValue(":notes", m_comboNotes->currentText());
                qDetails.exec();

                for (const auto& var : m_pendingVariations) {
                    QSqlQuery qVar(db);
                    qVar.prepare("INSERT INTO Item_variation (item_id, color_id, image) VALUES (:iid, :cid, :img)");
                    qVar.bindValue(":iid", newId);
                    qVar.bindValue(":cid", var.colorId);
                    qVar.bindValue(":img", var.imagePath);
                    qVar.exec();
                }
                m_pendingVariations.clear();

                db.commit();

                loadAccessories();
                for(int i = 0; i < m_accessoriesList->count(); ++i) {
                    if (m_accessoriesList->item(i)->data(Qt::UserRole).toInt() == newId) {
                        m_accessoriesList->setCurrentRow(i);
                        break;
                    }
                }
            } else {
                db.rollback();
            }
        } else {
            QListWidgetItem *item = m_accessoriesList->currentItem();
            if (!item) { db.rollback(); return; }

            int id = item->data(Qt::UserRole).toInt();

            QSqlQuery qItems(db);
            qItems.prepare("UPDATE Items SET name = :n, description = :d WHERE item_id = :id");
            qItems.bindValue(":n", newName);
            qItems.bindValue(":d", newDesc);
            qItems.bindValue(":id", id);

            QSqlQuery qDetails(db);
            qDetails.prepare("UPDATE Accessories_details SET is_customizable = :cust, attachment_method = :att, "
                             "durability = :dur, heat_sensitivity = :heat, design_notes = :notes WHERE item_id = :id");
            qDetails.bindValue(":cust", m_comboCustomizable->currentData().toInt());
            qDetails.bindValue(":att", m_comboAttachment->currentText());
            qDetails.bindValue(":dur", m_comboDurability->currentText());
            qDetails.bindValue(":heat", m_comboHeat->currentText());
            qDetails.bindValue(":notes", m_comboNotes->currentText());
            qDetails.bindValue(":id", id);

            if (qItems.exec() && qDetails.exec()) {
                db.commit();
                m_nameLabel->setText(newName);
                m_descLabel->setText(newDesc);
                item->setText("   " + newName);
                onAccessorySelected();
            } else {
                db.rollback();
                qDebug() << "Ошибка при сохранении:" << qItems.lastError().text() << qDetails.lastError().text();
            }
        }
    }
}

// ==========================================
// ЛОГИКА ВАРИАЦИЙ И УДАЛЕНИЯ
// ==========================================

void AccessoriesPage::renderPendingVariations() {
    QLayoutItem *child;
    while ((child = m_variationsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    for (int i = 0; i < m_pendingVariations.size(); ++i) {
        const auto& var = m_pendingVariations[i];

        QFrame *card = new QFrame(m_variationsContainer);
        card->setFixedSize(110, 130);
        card->setStyleSheet("QFrame { border: 1px solid #E8E8E8; border-radius: 12px; background: white; }");

        QVBoxLayout *cL = new QVBoxLayout(card);
        cL->setContentsMargins(10, 10, 10, 5);
        cL->setSpacing(5);

        QLabel *imgLbl = new QLabel(card);
        imgLbl->setFixedSize(90, 70);
        imgLbl->setAlignment(Qt::AlignCenter);
        imgLbl->setStyleSheet("border: none; background: #FFFFFF; border-radius: 8px;");

        if (!var.imagePath.isEmpty()) {
            QPixmap p(var.imagePath);
            if (!p.isNull()) {
                qreal dpr = this->devicePixelRatioF();
                QPixmap scaled = p.scaled(imgLbl->size() * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                scaled.setDevicePixelRatio(dpr);
                imgLbl->setPixmap(scaled);
            } else { imgLbl->setText("No Img"); }
        } else { imgLbl->setText("No Img"); }

        QLabel *colorLbl = new QLabel(var.colorName, card);
        colorLbl->setAlignment(Qt::AlignCenter);
        colorLbl->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; font-weight: bold; color: #333; border: none;");

        QPushButton *btnDel = new QPushButton("Удалить", card);
        btnDel->setCursor(Qt::PointingHandCursor);
        btnDel->setStyleSheet("QPushButton { border: none; font-family: 'Century Gothic'; font-size: 10px; color: #D86B7A; } QPushButton:hover { text-decoration: underline; }");

        connect(btnDel, &QPushButton::clicked, this, [this, i]() {
            ConfirmDialog d("Удаление", "Удалить эту вариацию?", this->window());
            if (d.exec() == QDialog::Accepted) {
                m_pendingVariations.removeAt(i);
                renderPendingVariations();
            }
        });

        cL->addWidget(imgLbl);
        cL->addWidget(colorLbl);
        cL->addWidget(btnDel);
        m_variationsLayout->addWidget(card);
    }
}

void AccessoriesPage::loadVariations(int itemId) {
    QLayoutItem *child;
    while ((child = m_variationsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    QSqlQuery q;
    q.prepare("SELECT v.variation_id, c.color_name, v.image "
              "FROM Item_variation v "
              "LEFT JOIN Colors c ON v.color_id = c.color_id "
              "WHERE v.item_id = :id");
    q.bindValue(":id", itemId);

    if (q.exec()) {
        while (q.next()) {
            int varId = q.value(0).toInt();
            QString colorName = q.value(1).toString();
            QString imgPath = q.value(2).toString();

            QFrame *card = new QFrame(m_variationsContainer);
            card->setFixedSize(110, 130);
            card->setStyleSheet("QFrame { border: 1px solid #E8E8E8; border-radius: 12px; background: white; }");

            QVBoxLayout *cL = new QVBoxLayout(card);
            cL->setContentsMargins(10, 10, 10, 5);
            cL->setSpacing(5);

            QLabel *imgLbl = new QLabel(card);
            imgLbl->setFixedSize(90, 70);
            imgLbl->setAlignment(Qt::AlignCenter);
            imgLbl->setStyleSheet("border: none; background: #FFFFFF; border-radius: 8px;");

            if (!imgPath.isEmpty()) {
                QPixmap p(imgPath);
                if (!p.isNull()) {
                    qreal dpr = this->devicePixelRatioF();
                    QPixmap scaled = p.scaled(imgLbl->size() * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    scaled.setDevicePixelRatio(dpr);
                    imgLbl->setPixmap(scaled);
                } else { imgLbl->setText("No Img"); }
            } else { imgLbl->setText("No Img"); }

            QLabel *colorLbl = new QLabel(colorName.isEmpty() ? "Неизвестно" : colorName, card);
            colorLbl->setAlignment(Qt::AlignCenter);
            colorLbl->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; font-weight: bold; color: #333; border: none;");

            QPushButton *btnDel = new QPushButton("Удалить", card);
            btnDel->setCursor(Qt::PointingHandCursor);
            btnDel->setStyleSheet("QPushButton { border: none; font-family: 'Century Gothic'; font-size: 10px; color: #D86B7A; } QPushButton:hover { text-decoration: underline; }");

            connect(btnDel, &QPushButton::clicked, this, [this, varId]() {
                onDeleteVariationClicked(varId);
            });

            cL->addWidget(imgLbl);
            cL->addWidget(colorLbl);
            cL->addWidget(btnDel);

            m_variationsLayout->addWidget(card);
        }
    }
}

void AccessoriesPage::onAddVariationClicked() {
    int id = -1;

    if (!m_isAddingNew) {
        QListWidgetItem *item = m_accessoriesList->currentItem();
        if (!item) return;
        id = item->data(Qt::UserRole).toInt();
    }

    QWidget *overlay = new QWidget(this->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->resize(this->window()->size());
    overlay->move(0, 0);
    overlay->show();

    AddVariationDialog dialog(id, this->window());
    dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());

    dialog.exec();
    overlay->deleteLater();

    if (dialog.isSaved()) {
        if (m_isAddingNew) {
            int colorId = dialog.getColorId();
            QString colorName = "Неизвестно";

            QSqlQuery q;
            q.prepare("SELECT color_name FROM Colors WHERE color_id = :id");
            q.bindValue(":id", colorId);
            if (q.exec() && q.next()) colorName = q.value(0).toString();

            m_pendingVariations.append({colorId, colorName, dialog.getImagePath()});
            renderPendingVariations();
        } else {
            loadVariations(id);
            onAccessorySelected();
        }
    }
}

void AccessoriesPage::onDeleteVariationClicked(int variationId) {
    QWidget *overlay = new QWidget(this->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->resize(this->window()->size());
    overlay->move(0, 0);
    overlay->show();

    ConfirmDialog dialog("Удаление", "Удалить эту вариацию аксессуара?", this->window());
    dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());

    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery q;
        q.prepare("DELETE FROM Item_variation WHERE variation_id = :vid");
        q.bindValue(":vid", variationId);
        if (q.exec()) {
            int currentItemId = m_accessoriesList->currentItem()->data(Qt::UserRole).toInt();
            loadVariations(currentItemId);
            onAccessorySelected();
        }
    }
    overlay->deleteLater();
}

void AccessoriesPage::onDeleteAccessoryClicked() {
    QListWidgetItem *item = m_accessoriesList->currentItem();
    if (!item) return;

    QWidget *overlay = new QWidget(this->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->resize(this->window()->size());
    overlay->move(0, 0);
    overlay->show();

    ConfirmDialog dialog("Удаление", "Удалить этот аксессуар из каталога навсегда?", this->window());
    dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());

    if (dialog.exec() == QDialog::Accepted) {
        int id = item->data(Qt::UserRole).toInt();
        QSqlQuery q;
        q.prepare("DELETE FROM Items WHERE item_id = :id");
        q.bindValue(":id", id);
        if (q.exec()) loadAccessories();
    }
    overlay->deleteLater();
}

void AccessoriesPage::onSearchTextChanged(const QString &text) {
    for(int i = 0; i < m_accessoriesList->count(); ++i) {
        m_accessoriesList->item(i)->setHidden(!m_accessoriesList->item(i)->text().contains(text, Qt::CaseInsensitive));
    }
}