#include "flowerspage.h"
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

FlowersPage::FlowersPage(QWidget *parent) : QWidget(parent), m_isEditing(false), m_isAddingNew(false) {
    setStyleSheet("background-color: #FFFFFF;");
    setupUI();
    loadFlowers();
}

void FlowersPage::setupUI() {
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

    QLabel *titleLabel = new QLabel("КАТАЛОГ ЦВЕТОВ", leftPanel);
    titleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; color: #333;");
    leftLayout->addWidget(titleLabel);

    m_searchEdit = new QLineEdit(leftPanel);
    m_searchEdit->setPlaceholderText("Поиск...");
    m_searchEdit->setFixedHeight(38);
    m_searchEdit->setStyleSheet("QLineEdit { border: 1px solid #D86B7A; border-radius: 19px; padding-left: 15px; font-family: 'Century Gothic'; }");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &FlowersPage::onSearchTextChanged);
    leftLayout->addWidget(m_searchEdit);

    m_flowersList = new QListWidget(leftPanel);
    m_flowersList->setFrameShape(QFrame::NoFrame);
    m_flowersList->setFocusPolicy(Qt::NoFocus);
    m_flowersList->setIconSize(QSize(30, 30));
    m_flowersList->setStyleSheet(
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
    connect(m_flowersList, &QListWidget::itemSelectionChanged, this, &FlowersPage::onFlowerSelected);
    leftLayout->addWidget(m_flowersList);

    m_btnAddFlower = new QPushButton("+ Добавить цветок", leftPanel);
    m_btnAddFlower->setFixedHeight(38);
    m_btnAddFlower->setCursor(Qt::PointingHandCursor);
    m_btnAddFlower->setStyleSheet("QPushButton { border: 1px solid #DCDCDC; border-radius: 12px; font-family: 'Century Gothic'; font-size: 12px; color: #555; } QPushButton:hover { background: #F5F5F5; }");
    connect(m_btnAddFlower, &QPushButton::clicked, this, &FlowersPage::onAddFlowerClicked);
    leftLayout->addWidget(m_btnAddFlower);

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
    scrollContent->setObjectName("mainScrollContent"); // ВАЖНО: Имя для поиска
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

    m_flowerIconLabel = new QLabel(m_infoCard);
    m_flowerIconLabel->setFixedSize(140, 140);
    m_flowerIconLabel->setAlignment(Qt::AlignCenter);
    m_flowerIconLabel->setStyleSheet("border: 1px solid #EEE; border-radius: 12px; background: #FFFFFF;");

    m_textStack = new QStackedWidget(m_infoCard);
    m_textStack->setStyleSheet("background: transparent; border: none;");

    QWidget *vW = new QWidget();
    QVBoxLayout *vL = new QVBoxLayout(vW); vL->setContentsMargins(0,0,0,0); vL->setSpacing(8);
    m_flowerNameLabel = new QLabel("Название", vW);
    m_flowerNameLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 22px; font-weight: bold; color: #000; border: none;");
    m_flowerDescLabel = new QLabel("Описание...", vW);
    m_flowerDescLabel->setWordWrap(true);
    m_flowerDescLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; color: #555; border: none; line-height: 1.3;");
    m_flowerIdLabel = new QLabel("ID: #0", vW);
    m_flowerIdLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; color: #AAA; font-weight: bold; border: none;");
    vL->addWidget(m_flowerNameLabel); vL->addWidget(m_flowerDescLabel, 1); vL->addWidget(m_flowerIdLabel);

    QWidget *eW = new QWidget();
    QVBoxLayout *eL = new QVBoxLayout(eW); eL->setContentsMargins(0,0,0,0); eL->setSpacing(10);

    m_nameEditField = new QLineEdit(eW);
    m_nameEditField->setMinimumWidth(180);
    m_nameEditField->setPlaceholderText("Название цветка...");
    m_nameEditField->setStyleSheet("border: 1px solid #DCDCDC; border-radius: 6px; font-family: 'Century Gothic'; font-weight: bold; padding: 5px;");

    m_descEditField = new QTextEdit(eW);
    m_descEditField->setMinimumWidth(180);
    m_descEditField->setPlaceholderText("Описание цветка...");
    m_descEditField->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_descEditField->setStyleSheet("QTextEdit { border: 1px solid #DCDCDC; border-radius: 6px; font-family: 'Century Gothic'; font-size: 12px; padding: 5px; background: white; }");

    eL->addWidget(m_nameEditField); eL->addWidget(m_descEditField, 1);
    m_textStack->addWidget(vW); m_textStack->addWidget(eW);

    QVBoxLayout *btnContainer = new QVBoxLayout();
    btnContainer->setSpacing(10);
    btnContainer->setAlignment(Qt::AlignTop);

    m_btnEditFlower = new QPushButton("Редактировать", m_infoCard);
    m_btnEditFlower->setFixedSize(125, 36);
    m_btnEditFlower->setCursor(Qt::PointingHandCursor);
    m_btnEditFlower->setStyleSheet("QPushButton { border: 2px solid #DCDCDC; border-radius: 18px; font-family: 'Century Gothic'; font-size: 12px; } QPushButton:hover { background: #F5F5F5; }");
    connect(m_btnEditFlower, &QPushButton::clicked, this, &FlowersPage::onEditFlowerClicked);

    m_btnDeleteFlower = new QPushButton("Удалить", m_infoCard);
    m_btnDeleteFlower->setFixedSize(60, 36);
    m_btnDeleteFlower->setCursor(Qt::PointingHandCursor);
    m_btnDeleteFlower->setStyleSheet("QPushButton { border: none; color: #D86B7A; font-family: 'Century Gothic'; font-size: 12px; } QPushButton:hover { text-decoration: underline; }");
    connect(m_btnDeleteFlower, &QPushButton::clicked, this, &FlowersPage::onDeleteFlowerClicked);

    btnContainer->addWidget(m_btnEditFlower);
    btnContainer->addWidget(m_btnDeleteFlower, 0, Qt::AlignCenter);

    cardLayout->addWidget(m_flowerIconLabel);
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

    addViewStat("Размер бутона", m_valBudSize, 0, 0);
    addViewStat("Аромат", m_valScent, 0, 1);
    addViewStat("Длина стебля (см)", m_valStem, 1, 0);
    addViewStat("Уровень пыльцы", m_valPollen, 1, 1);
    addViewStat("Стойкость (дней)", m_valLifetime, 2, 0);
    addViewStat("Ядовитость", m_valPoison, 2, 1);

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

    m_editBudSize = new QComboBox();
    m_editBudSize->addItems({"Микро", "Мелкий", "Средний", "Крупный", "Гигантский", "Пионовидный"});

    m_editScent = new QComboBox();
    m_editScent->addItems({"Нет", "Слабый", "Средний", "Сильный", "Специфический"});

    m_editStem = new QComboBox();
    m_editStem->addItems({"30", "40", "50", "60", "70", "80", "90", "100"});

    m_editPollen = new QComboBox();
    m_editPollen->addItems({"Нет", "Низкий", "Средний", "Высокий"});

    m_editLifetime = new QComboBox();
    m_editLifetime->addItems({"5", "7", "10", "14", "20", "30"});

    m_comboPoison = new QComboBox();
    m_comboPoison->addItem("Нет", 0);
    m_comboPoison->addItem("Да", 1);

    addEditStatCombo("Размер бутона", m_editBudSize, 0, 0);
    addEditStatCombo("Аромат", m_editScent, 0, 1);
    addEditStatCombo("Длина стебля (см)", m_editStem, 1, 0);
    addEditStatCombo("Уровень пыльцы", m_editPollen, 1, 1);
    addEditStatCombo("Стойкость (дней)", m_editLifetime, 2, 0);
    addEditStatCombo("Ядовитость", m_comboPoison, 2, 1);

    m_detailsStack->addWidget(viewGridWidget);
    m_detailsStack->addWidget(editGridWidget);
    rightLayout->addWidget(m_detailsStack);

    // --- 3. ВАРИАЦИИ ---
    rightLayout->addSpacing(25);

    QHBoxLayout *varHeaderLayout = new QHBoxLayout();
    m_varTitle = new QLabel("ВАРИАЦИИ ЦВЕТКА", scrollContent);
    m_varTitle->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; font-weight: bold; color: #AAA; letter-spacing: 2px;");

    m_btnAddVariation = new QPushButton("+ Добавить вариацию", scrollContent);
    m_btnAddVariation->setFixedSize(140, 26);
    m_btnAddVariation->setCursor(Qt::PointingHandCursor);
    m_btnAddVariation->setStyleSheet("QPushButton { border: 1px solid #DCDCDC; border-radius: 13px; font-family: 'Century Gothic'; font-size: 11px; color: #555; background: white; } QPushButton:hover { background: #F5F5F5; }");
    connect(m_btnAddVariation, &QPushButton::clicked, this, &FlowersPage::onAddVariationClicked);

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

    scrollContent->hide(); // Прячем внутренности со старта
}

void FlowersPage::loadFlowers() {
    m_flowersList->clear();
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return;

    QSqlQuery q(db);
    if (q.exec("SELECT item_id, name FROM Items WHERE item_type = 'flower' ORDER BY name")) {
        while (q.next()) {
            QListWidgetItem *item = new QListWidgetItem("   " + q.value(1).toString());
            item->setData(Qt::UserRole, q.value(0).toInt());
            item->setIcon(QIcon(":/sources/icons/flower_icon.png"));
            item->setFont(QFont("Century Gothic", 11));
            m_flowersList->addItem(item);
        }
    }
}

// ==========================================
// ЛОГИКА ВЗАИМОДЕЙСТВИЯ
// ==========================================

void FlowersPage::onFlowerSelected() {
    QListWidgetItem *item = m_flowersList->currentItem();

    QScrollArea *scrollArea = m_rightPanel->findChild<QScrollArea*>();
    QWidget *scrollContent = m_rightPanel->findChild<QWidget*>("mainScrollContent");

    if (!item) {
        if (scrollContent) scrollContent->hide();
        if (scrollArea) scrollArea->hide();
        return;
    }

    if (scrollArea) scrollArea->show();
    if (scrollContent) scrollContent->show();

    m_isEditing = false;
    m_isAddingNew = false;
    m_btnDeleteFlower->show();

    m_varScrollArea->show();
    m_varTitle->show();
    m_btnAddVariation->show();

    m_textStack->setCurrentIndex(0);
    m_detailsStack->setCurrentIndex(0);
    m_btnEditFlower->setText("Редактировать");
    m_btnEditFlower->setStyleSheet("QPushButton { border: 2px solid #DCDCDC; border-radius: 18px; font-family: 'Century Gothic'; font-size: 12px; color: #333; } QPushButton:hover { background: #F5F5F5; }");

    int id = item->data(Qt::UserRole).toInt();
    QSqlQuery q;

    q.prepare("SELECT i.name, i.description, "
              "(SELECT image FROM Item_variation WHERE item_id = i.item_id LIMIT 1), "
              "fd.bud_size, fd.scent_intensity, fd.stem_length_cm, "
              "fd.pollen_level, fd.lifetime, fd.is_poisonous "
              "FROM Items i LEFT JOIN Flower_details fd ON i.item_id = fd.item_id "
              "WHERE i.item_id = :id");
    q.bindValue(":id", id);

    if (q.exec() && q.next()) {
        QString name = q.value(0).toString();
        QString desc = q.value(1).toString();
        QString imgPath = q.value(2).toString();

        m_flowerNameLabel->setText(name);
        m_flowerDescLabel->setText(desc.isEmpty() ? "Описание отсутствует." : desc);
        m_flowerIdLabel->setText(QString("ID: #%1").arg(id));
        m_nameEditField->setText(name);
        m_descEditField->setPlainText(desc);

        QString budSize = q.value(3).toString();
        QString scent = q.value(4).toString();
        QString stem = q.value(5).toString();
        QString pollen = q.value(6).toString();
        QString lifetime = q.value(7).toString();
        int isPoison = q.value(8).toInt();

        m_valBudSize->setText(budSize.isEmpty() ? "-" : budSize);
        m_valScent->setText(scent.isEmpty() ? "-" : scent);
        m_valStem->setText(stem.isEmpty() ? "-" : stem);
        m_valPollen->setText(pollen.isEmpty() ? "-" : pollen);
        m_valLifetime->setText(lifetime.isEmpty() ? "-" : lifetime);
        m_valPoison->setText(isPoison == 1 ? "Да" : "Нет");

        m_editBudSize->setCurrentText(budSize);
        m_editScent->setCurrentText(scent);
        m_editStem->setCurrentText(stem);
        m_editPollen->setCurrentText(pollen);
        m_editLifetime->setCurrentText(lifetime);
        m_comboPoison->setCurrentIndex(isPoison == 1 ? 1 : 0);

        m_flowerIconLabel->setStyleSheet("border: 1px solid #EEE; border-radius: 12px; background: #FFFFFF;");

        if (!imgPath.isEmpty()) {
            QPixmap pix(imgPath);
            if (!pix.isNull()) {
                qreal dpr = this->devicePixelRatioF();
                QPixmap scaledPix = pix.scaled(m_flowerIconLabel->size() * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                scaledPix.setDevicePixelRatio(dpr);
                m_flowerIconLabel->setPixmap(scaledPix);
            } else {
                m_flowerIconLabel->setText("No Img");
                m_flowerIconLabel->setStyleSheet("border: 1px solid #EEE; border-radius: 12px; background: #FFFFFF; color: #AAA; font-family: 'Century Gothic';");
            }
        } else {
            m_flowerIconLabel->setText("No Img");
            m_flowerIconLabel->setStyleSheet("border: 1px solid #EEE; border-radius: 12px; background: #FFFFFF; color: #AAA; font-family: 'Century Gothic';");
        }

        loadVariations(id);
    }
}

void FlowersPage::onAddFlowerClicked() {
    m_flowersList->clearSelection();
    m_isEditing = true;
    m_isAddingNew = true;

    m_pendingVariations.clear();

    QScrollArea *scrollArea = m_rightPanel->findChild<QScrollArea*>();
    QWidget *scrollContent = m_rightPanel->findChild<QWidget*>("mainScrollContent");

    if (scrollArea) scrollArea->show();
    if (scrollContent) scrollContent->show();

    m_infoCard->show();
    m_detailsStack->show();
    m_varScrollArea->show();
    m_varTitle->show();
    m_btnAddVariation->show();

    QLabel *charTitle = m_rightPanel->findChild<QLabel*>("charTitle");
    if (charTitle) charTitle->show();

    renderPendingVariations();

    m_nameEditField->clear();
    m_descEditField->clear();

    m_editBudSize->setCurrentIndex(0);
    m_editScent->setCurrentIndex(0);
    m_editStem->setCurrentIndex(0);
    m_editPollen->setCurrentIndex(0);
    m_editLifetime->setCurrentIndex(0);
    m_comboPoison->setCurrentIndex(0);

    m_flowerIconLabel->clear();
    m_flowerIconLabel->setText("Новое\nфото");
    m_flowerIconLabel->setStyleSheet("border: 1px dashed #AAA; border-radius: 12px; background: #FFFFFF; color: #AAA; font-family: 'Century Gothic'; font-weight: bold;");

    m_textStack->setCurrentIndex(1);
    m_detailsStack->setCurrentIndex(1);

    m_btnEditFlower->setText("Сохранить");
    m_btnEditFlower->setStyleSheet("QPushButton { border: none; border-radius: 18px; color: white; background: #D86B7A; font-family: 'Century Gothic'; font-weight: bold; }");
    m_btnDeleteFlower->hide();
}

void FlowersPage::onEditFlowerClicked() {
    if (!m_isEditing) {
        m_isEditing = true;
        m_isAddingNew = false;
        m_textStack->setCurrentIndex(1);
        m_detailsStack->setCurrentIndex(1);
        m_btnEditFlower->setText("Сохранить");
        m_btnEditFlower->setStyleSheet("QPushButton { border: none; border-radius: 18px; color: white; background: #D86B7A; font-family: 'Century Gothic'; font-weight: bold; }");
    } else {
        QString newName = m_nameEditField->text().trimmed();
        QString newDesc = m_descEditField->toPlainText().trimmed();

        if (newName.isEmpty() || newDesc.isEmpty()) {
            QWidget *overlay = new QWidget(this->window());
            overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
            overlay->resize(this->window()->size());
            overlay->move(0, 0);
            overlay->show();

            ErrorDialog err("Ошибка", "Заполните название и описание цветка!", this->window());
            err.move(this->window()->frameGeometry().center() - err.rect().center());
            err.exec();

            overlay->deleteLater();
            return;
        }

        QSqlDatabase db = QSqlDatabase::database();
        db.transaction();

        if (m_isAddingNew) {
            QSqlQuery qItems(db);
            qItems.prepare("INSERT INTO Items (name, description, item_type) VALUES (:n, :d, 'flower')");
            qItems.bindValue(":n", newName);
            qItems.bindValue(":d", newDesc);

            if (qItems.exec()) {
                int newId = qItems.lastInsertId().toInt();

                QSqlQuery qDetails(db);
                qDetails.prepare("INSERT INTO Flower_details (item_id, bud_size, scent_intensity, stem_length_cm, pollen_level, lifetime, is_poisonous) "
                                 "VALUES (:id, :b, :sc, :st, :p, :l, :pois)");
                qDetails.bindValue(":id", newId);
                qDetails.bindValue(":b", m_editBudSize->currentText());
                qDetails.bindValue(":sc", m_editScent->currentText());
                qDetails.bindValue(":st", m_editStem->currentText().toInt());
                qDetails.bindValue(":p", m_editPollen->currentText());
                qDetails.bindValue(":l", m_editLifetime->currentText().toInt());
                qDetails.bindValue(":pois", m_comboPoison->currentData().toInt());
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

                loadFlowers();
                for(int i = 0; i < m_flowersList->count(); ++i) {
                    if (m_flowersList->item(i)->data(Qt::UserRole).toInt() == newId) {
                        m_flowersList->setCurrentRow(i);
                        break;
                    }
                }
            } else {
                db.rollback();
            }
        } else {
            QListWidgetItem *item = m_flowersList->currentItem();
            if (!item) { db.rollback(); return; }

            int id = item->data(Qt::UserRole).toInt();

            QSqlQuery qItems(db);
            qItems.prepare("UPDATE Items SET name = :n, description = :d WHERE item_id = :id");
            qItems.bindValue(":n", newName);
            qItems.bindValue(":d", newDesc);
            qItems.bindValue(":id", id);

            QSqlQuery qDetails(db);
            qDetails.prepare("UPDATE Flower_details SET bud_size = :b, scent_intensity = :sc, stem_length_cm = :st, "
                             "pollen_level = :p, lifetime = :l, is_poisonous = :pois WHERE item_id = :id");
            qDetails.bindValue(":b", m_editBudSize->currentText());
            qDetails.bindValue(":sc", m_editScent->currentText());
            qDetails.bindValue(":st", m_editStem->currentText().toInt());
            qDetails.bindValue(":p", m_editPollen->currentText());
            qDetails.bindValue(":l", m_editLifetime->currentText().toInt());
            qDetails.bindValue(":pois", m_comboPoison->currentData().toInt());
            qDetails.bindValue(":id", id);

            if (qItems.exec() && qDetails.exec()) {
                db.commit();
                m_flowerNameLabel->setText(newName);
                m_flowerDescLabel->setText(newDesc);
                item->setText("   " + newName);
                onFlowerSelected();
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

void FlowersPage::renderPendingVariations() {
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

void FlowersPage::loadVariations(int itemId) {
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

void FlowersPage::onAddVariationClicked() {
    int id = -1;

    if (!m_isAddingNew) {
        QListWidgetItem *item = m_flowersList->currentItem();
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
            onFlowerSelected();
        }
    }
}

void FlowersPage::onDeleteVariationClicked(int variationId) {
    QWidget *overlay = new QWidget(this->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->resize(this->window()->size());
    overlay->move(0, 0);
    overlay->show();

    ConfirmDialog dialog("Удаление", "Удалить эту вариацию?", this->window());
    dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());

    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery q;
        q.prepare("DELETE FROM Item_variation WHERE variation_id = :vid");
        q.bindValue(":vid", variationId);
        if (q.exec()) {
            int currentItemId = m_flowersList->currentItem()->data(Qt::UserRole).toInt();
            loadVariations(currentItemId);
            onFlowerSelected();
        }
    }
    overlay->deleteLater();
}

void FlowersPage::onDeleteFlowerClicked() {
    QListWidgetItem *item = m_flowersList->currentItem();
    if (!item) return;

    QWidget *overlay = new QWidget(this->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->resize(this->window()->size());
    overlay->move(0, 0);
    overlay->show();

    ConfirmDialog dialog("Удаление", "Удалить этот цветок из каталога навсегда?", this->window());
    dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());

    if (dialog.exec() == QDialog::Accepted) {
        int id = item->data(Qt::UserRole).toInt();
        QSqlQuery q;
        q.prepare("DELETE FROM Items WHERE item_id = :id");
        q.bindValue(":id", id);

        if (q.exec()) {
            m_flowersList->clearSelection();
            loadFlowers();
        }
    }
    overlay->deleteLater();
}

void FlowersPage::onSearchTextChanged(const QString &text) {
    for(int i = 0; i < m_flowersList->count(); ++i) {
        m_flowersList->item(i)->setHidden(!m_flowersList->item(i)->text().contains(text, Qt::CaseInsensitive));
    }
}