#include "rulespage.h"
#include "errordialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QDebug>

RulesPage::RulesPage(QWidget *parent) : QWidget(parent), m_isEditing(false), m_isAddingNew(false) {
    setStyleSheet("background-color: #FFFFFF;");
    setupUI();
    loadItemsIntoCombos();
    loadRules();
}

void RulesPage::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ==========================================
    // ЛЕВАЯ ПАНЕЛЬ
    // ==========================================
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setFixedWidth(320);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(25, 25, 15, 25);
    leftLayout->setSpacing(15);

    QLabel *titleLabel = new QLabel("ПРАВИЛА СОЧЕТАНИЙ", leftPanel);
    titleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; color: #333; border: none; background: transparent;");
    leftLayout->addWidget(titleLabel);

    m_searchEdit = new QLineEdit(leftPanel);
    m_searchEdit->setPlaceholderText("Поиск...");
    m_searchEdit->setFixedHeight(38);
    m_searchEdit->setStyleSheet("QLineEdit { border: 1px solid #D86B7A; border-radius: 19px; padding-left: 15px; font-family: 'Century Gothic'; font-size: 13px; }");
    leftLayout->addWidget(m_searchEdit);

    m_rulesList = new QListWidget(leftPanel);
    m_rulesList->setFrameShape(QFrame::NoFrame);
    m_rulesList->setFocusPolicy(Qt::NoFocus);
    m_rulesList->setStyleSheet(
        "QListWidget { background: transparent; outline: none; border: none; } "
        "QListWidget::item { height: 65px; border-radius: 12px; margin-bottom: 5px; border: none; } "
        "QListWidget::item:hover { background-color: #FFF0F3; } "
        "QListWidget::item:selected { background-color: #FFF0F3; border: none; outline: none; } "
        "QScrollBar:vertical { border: none; background: transparent; width: 4px; margin: 0px; } "
        "QScrollBar::handle:vertical { background: #DCDCDC; border-radius: 2px; min-height: 30px; } "
        "QScrollBar::handle:vertical:hover { background: #D86B7A; } "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; } "
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        );
    leftLayout->addWidget(m_rulesList);

    m_btnAddRule = new QPushButton("+ Создать правило", leftPanel);
    m_btnAddRule->setFixedHeight(38);
    m_btnAddRule->setCursor(Qt::PointingHandCursor);
    m_btnAddRule->setStyleSheet("QPushButton { border: 1px solid #DCDCDC; border-radius: 12px; font-family: 'Century Gothic'; font-size: 12px; color: #555; background: white; } QPushButton:hover { background: #F5F5F5; }");
    leftLayout->addWidget(m_btnAddRule);

    QFrame *vSep = new QFrame(this);
    vSep->setFrameShape(QFrame::VLine);
    vSep->setStyleSheet("background-color: #E8E8E8; border: none; width: 1px; margin: 25px 0;");

    // ==========================================
    // ПРАВАЯ ПАНЕЛЬ
    // ==========================================
    m_rightPanel = new QWidget(this);
    QVBoxLayout *rightMainLayout = new QVBoxLayout(m_rightPanel);
    rightMainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainScroll = new QScrollArea(m_rightPanel);
    m_mainScroll->setWidgetResizable(true);
    m_mainScroll->setFrameShape(QFrame::NoFrame);
    m_mainScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_mainScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_mainScroll->setStyleSheet(
        "QScrollArea { background: transparent; border: none; } "
        "QScrollBar:vertical { border: none; background: transparent; width: 6px; margin: 0px; } "
        "QScrollBar::handle:vertical { background: #DCDCDC; border-radius: 3px; min-height: 30px; } "
        "QScrollBar::handle:vertical:hover { background: #D86B7A; } "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; background: none; } "
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        );

    m_scrollContent = new QWidget();
    m_scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout *rightLayout = new QVBoxLayout(m_scrollContent);
    rightLayout->setContentsMargins(20, 20, 20, 20);
    rightLayout->setSpacing(15);
    rightLayout->setAlignment(Qt::AlignTop);

    // --- КАРТОЧКА 1: ЗАГОЛОВОК ---
    m_headerCard = new QFrame();
    m_headerCard->setFixedHeight(80);
    m_headerCard->setStyleSheet("QFrame { background: white; border: 1px solid #F2F2F2; border-radius: 20px; }");
    QHBoxLayout *hL = new QHBoxLayout(m_headerCard);
    hL->setContentsMargins(25, 0, 25, 0);

    QVBoxLayout *titleV = new QVBoxLayout();
    titleV->setAlignment(Qt::AlignVCenter);
    m_ruleTitleLabel = new QLabel("Выберите правило", m_headerCard);
    m_ruleTitleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 20px; font-weight: bold; color: #333; border: none; background: transparent;");
    m_ruleIdLabel = new QLabel("ID: #0", m_headerCard);
    m_ruleIdLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; color: #BBB; border: none; background: transparent;");
    titleV->addWidget(m_ruleTitleLabel);
    titleV->addWidget(m_ruleIdLabel);
    hL->addLayout(titleV);
    hL->addStretch();

    m_btnEditRule = new QPushButton("Редактировать", m_headerCard);
    m_btnEditRule->setFixedSize(130, 36);
    m_btnEditRule->setCursor(Qt::PointingHandCursor);
    m_btnEditRule->setStyleSheet("QPushButton { border: 1.5px solid #EAEAEA; border-radius: 18px; font-family: 'Century Gothic'; font-size: 12px; font-weight: bold; color: #444; background: white; } QPushButton:hover { background: #F9F9F9; }");

    m_btnDeleteRule = new QPushButton("×", m_headerCard);
    m_btnDeleteRule->setFixedSize(36, 36);
    m_btnDeleteRule->setCursor(Qt::PointingHandCursor);
    m_btnDeleteRule->setStyleSheet(
        "QPushButton { "
        "  border: 1.5px solid #EAEAEA; "
        "  border-radius: 18px; "
        "  color: #D86B7A; "
        "  font-family: 'Arial'; "
        "  font-size: 22px; "
        "  background: white; "
        "  padding-bottom: 4px; " // Центрируем крестик
        "  text-align: center; "
        "} "
        "QPushButton:hover { background: #FFF0F3; border-color: #D86B7A; }"
        );

    hL->addWidget(m_btnEditRule);
    hL->addSpacing(10);
    hL->addWidget(m_btnDeleteRule);
    rightLayout->addWidget(m_headerCard);

    // --- КАРТОЧКА 2: ФОРМУЛА ---
    m_formulaCard = new QFrame();
    m_formulaCard->setStyleSheet("QFrame { background: white; border: 1px solid #F2F2F2; border-radius: 20px; }");
    QVBoxLayout *fL = new QVBoxLayout(m_formulaCard);
    fL->setContentsMargins(20, 20, 20, 25);

    QLabel *fTitle = new QLabel("ФОРМУЛА", m_formulaCard);
    fTitle->setAlignment(Qt::AlignCenter);
    fTitle->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; font-weight: bold; color: #CCC; letter-spacing: 3px; border: none; background: transparent;");
    fL->addWidget(fTitle); fL->addSpacing(15);

    QHBoxLayout *itemsH = new QHBoxLayout();
    auto createItemBox = [&](QLabel** img, QLabel** name, QComboBox** combo) {
        QVBoxLayout *v = new QVBoxLayout();
        v->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        *img = new QLabel();
        (*img)->setFixedSize(85, 85);
        (*img)->setAlignment(Qt::AlignCenter);
        (*img)->setStyleSheet("border: 1px solid #F5F5F5; border-radius: 42px; background: #FAFAFA;");
        *name = new QLabel("Предмет"); (*name)->setAlignment(Qt::AlignCenter);
        (*name)->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; color: #333; border: none; background: transparent; margin-top: 5px; margin-bottom: 5px;");
        *combo = new QComboBox();
        (*combo)->setFixedSize(180, 38);
        (*combo)->setStyleSheet("QComboBox { border: 1px solid #EAEAEA; border-radius: 10px; padding: 0 10px; font-family: 'Century Gothic'; font-size: 12px; color: #555; background: white; } QComboBox::drop-down { border: none; }");
        v->addWidget(*img, 0, Qt::AlignCenter);
        v->addWidget(*name);
        v->addWidget(*combo);
        return v;
    };

    QVBoxLayout *vA = createItemBox(&m_imgItemA, &m_nameItemA, &m_comboItemA);
    QLabel *plus = new QLabel("+"); plus->setStyleSheet("font-size: 24px; color: #E0E0E0; border: none; background: transparent; font-weight: 200;");
    QVBoxLayout *vB = createItemBox(&m_imgItemB, &m_nameItemB, &m_comboItemB);

    itemsH->addStretch(1);
    itemsH->addLayout(vA);
    itemsH->addSpacing(20);
    itemsH->addWidget(plus, 0, Qt::AlignCenter);
    itemsH->addSpacing(20);
    itemsH->addLayout(vB);
    itemsH->addStretch(1);

    fL->addLayout(itemsH);
    rightLayout->addWidget(m_formulaCard);

    // --- КАРТОЧКА 3: ПАРАМЕТРЫ ---
    m_paramsCard = new QFrame();
    m_paramsCard->setStyleSheet("QFrame { background: white; border: 1px solid #F2F2F2; border-radius: 20px; }");
    QVBoxLayout *pL = new QVBoxLayout(m_paramsCard);
    pL->setContentsMargins(25, 20, 25, 25);

    QHBoxLayout *sliderHeader = new QHBoxLayout();
    QLabel *lblComp = new QLabel("УРОВЕНЬ СОВМЕСТИМОСТИ (-10 ... 10)", m_paramsCard);
    lblComp->setStyleSheet("font-family: 'Century Gothic'; font-size: 10px; font-weight: bold; color: #999; border: none; background: transparent;");
    m_lblCompValue = new QLabel("0", m_paramsCard);
    m_lblCompValue->setStyleSheet("font-family: 'Century Gothic'; font-size: 22px; font-weight: bold; color: #F4CB45; border: none; background: transparent;");
    sliderHeader->addWidget(lblComp); sliderHeader->addStretch(); sliderHeader->addWidget(m_lblCompValue);

    m_sliderComp = new QSlider(Qt::Horizontal, m_paramsCard);
    m_sliderComp->setRange(-10, 10);
    m_sliderComp->setValue(0);
    m_sliderComp->setStyleSheet(
        "QSlider::groove:horizontal { background: #F5F5F5; height: 6px; border-radius: 3px; border: none;} "
        "QSlider::sub-page:horizontal { background: #E0E0E0; border-radius: 3px; } "
        "QSlider::handle:horizontal { background: white; border: 2px solid #CCC; width: 16px; height: 16px; margin: -5px 0; border-radius: 8px; }"
        );
    pL->addLayout(sliderHeader);
    pL->addWidget(m_sliderComp);
    pL->addSpacing(20);

    QLabel *lblDesc = new QLabel("ПОДРОБНОЕ ОПИСАНИЕ СОВМЕСТИМОСТИ", m_paramsCard);
    lblDesc->setStyleSheet("font-family: 'Century Gothic'; font-size: 10px; font-weight: bold; color: #999; border: none; background: transparent;");
    pL->addWidget(lblDesc);

    m_textEditFullDesc = new QTextEdit(m_paramsCard);
    m_textEditFullDesc->setFixedHeight(120);
    m_textEditFullDesc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_textEditFullDesc->setPlaceholderText("Опишите подробности совместимости этих цветов...");
    m_textEditFullDesc->setStyleSheet(
        "QTextEdit { border: 1px solid #EAEAEA; border-radius: 12px; padding: 10px; font-family: 'Century Gothic'; font-size: 13px; color: #333; background-color: white; } "
        "QScrollBar:vertical { border: none; background: transparent; width: 4px; margin: 0px;} "
        "QScrollBar::handle:vertical { background: #DCDCDC; border-radius: 2px; min-height: 20px; } "
        "QScrollBar::handle:vertical:hover { background: #D86B7A; } "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; background: none; } "
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        );
    pL->addWidget(m_textEditFullDesc);

    rightLayout->addWidget(m_paramsCard);
    rightLayout->addStretch(1);

    m_mainScroll->setWidget(m_scrollContent);
    rightMainLayout->addWidget(m_mainScroll);

    mainLayout->addWidget(leftPanel); mainLayout->addWidget(vSep); mainLayout->addWidget(m_rightPanel, 1);

    // Подключения
    connect(m_rulesList, &QListWidget::itemSelectionChanged, this, &RulesPage::onRuleSelected);
    connect(m_btnAddRule, &QPushButton::clicked, this, &RulesPage::onAddRuleClicked);
    connect(m_btnEditRule, &QPushButton::clicked, this, &RulesPage::onEditRuleClicked);
    connect(m_btnDeleteRule, &QPushButton::clicked, this, &RulesPage::onDeleteRuleClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &RulesPage::onSearchTextChanged);
    connect(m_comboItemA, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RulesPage::onItemAChanged);
    connect(m_comboItemB, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RulesPage::onItemBChanged);
    connect(m_sliderComp, &QSlider::valueChanged, this, &RulesPage::onSliderValueChanged);

    m_mainScroll->hide();
}

void RulesPage::showOverlay() {
    m_overlay = new QWidget(this->window());
    m_overlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);");
    m_overlay->resize(this->window()->size());
    m_overlay->move(0, 0);
    m_overlay->show();
}

void RulesPage::hideOverlay() {
    if (m_overlay) {
        m_overlay->deleteLater();
        m_overlay = nullptr;
    }
}

QWidget* RulesPage::createRuleItemWidget(const QString &imgA, const QString &imgB, const QString &title, const QString &desc, int compLevel) {
    QWidget *w = new QWidget();
    w->setStyleSheet("background: transparent;");
    QHBoxLayout *h = new QHBoxLayout(w);
    h->setContentsMargins(10, 8, 10, 8); h->setSpacing(10);

    auto createSmallCircle = [&](const QString &path) {
        QLabel *l = new QLabel(); l->setFixedSize(36, 36);
        l->setAlignment(Qt::AlignCenter);
        l->setStyleSheet("border-radius: 18px; background: #FAFAFA; border: 1px solid #EEE;");
        if(!path.isEmpty()) {
            l->setPixmap(getRoundedPixmap(path, 36));
            l->setStyleSheet("border-radius: 18px; border: 1px solid #EEE; background: transparent;");
        }
        return l;
    };

    h->addWidget(createSmallCircle(imgA));
    QLabel *p = new QLabel("+"); p->setStyleSheet("color: #DDD; font-size: 10px;");
    h->addWidget(p);
    h->addWidget(createSmallCircle(imgB));

    QVBoxLayout *tV = new QVBoxLayout(); tV->setSpacing(2);
    QLabel *nt = new QLabel(title); nt->setStyleSheet("font-family: 'Century Gothic'; font-weight: bold; font-size: 13px; color: #333;");
    QLabel *dt = new QLabel(desc); dt->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; color: #999;");
    tV->addWidget(nt); tV->addWidget(dt);

    h->addLayout(tV); h->addStretch();

    QLabel *dot = new QLabel();
    dot->setFixedSize(6, 6);
    QString dotColor;
    if (compLevel < 0) dotColor = "#D86B7A";
    else if (compLevel == 0) dotColor = "#F4CB45";
    else dotColor = "#66CDAA";
    dot->setStyleSheet(QString("background: %1; border-radius: 3px;").arg(dotColor));

    h->addWidget(dot);
    return w;
}

void RulesPage::loadRules() {
    m_rulesList->clear();
    QSqlQuery q("SELECT cr.rule_id, iA.name, iB.name, cr.description, "
                "(SELECT image FROM Item_variation WHERE item_id = cr.item_a_id LIMIT 1), "
                "(SELECT image FROM Item_variation WHERE item_id = cr.item_b_id LIMIT 1), "
                "cr.compatibility_level "
                "FROM CompatibilityRules cr "
                "JOIN Items iA ON cr.item_a_id = iA.item_id "
                "JOIN Items iB ON cr.item_b_id = iB.item_id");

    while(q.next()){
        QListWidgetItem *item = new QListWidgetItem(m_rulesList);
        item->setData(Qt::UserRole, q.value(0).toInt());
        item->setData(Qt::UserRole + 1, q.value(1).toString() + " " + q.value(2).toString());
        item->setSizeHint(QSize(0, 65));

        QString fullName = q.value(1).toString() + " + " + q.value(2).toString();
        QFontMetrics fm(QFont("Century Gothic", 10, QFont::Bold));
        QString elidedName = fm.elidedText(fullName, Qt::ElideRight, 120);

        m_rulesList->setItemWidget(item, createRuleItemWidget(q.value(4).toString(), q.value(5).toString(), elidedName, q.value(3).toString(), q.value(6).toInt()));
    }
}

void RulesPage::onRuleSelected() {
    QListWidgetItem *li = m_rulesList->currentItem();
    if(!li) { m_mainScroll->hide(); return; }
    m_mainScroll->show();
    m_isEditing = false;

    QSqlQuery q;
    q.prepare("SELECT item_a_id, item_b_id, description, compatibility_level FROM CompatibilityRules WHERE rule_id = :id");
    q.bindValue(":id", li->data(Qt::UserRole).toInt());

    if(q.exec() && q.next()){
        m_comboItemA->blockSignals(true); m_comboItemB->blockSignals(true);
        m_comboItemA->setCurrentIndex(m_comboItemA->findData(q.value(0).toInt()));
        m_comboItemB->setCurrentIndex(m_comboItemB->findData(q.value(1).toInt()));
        m_comboItemA->blockSignals(false); m_comboItemB->blockSignals(false);

        m_textEditFullDesc->setPlainText(q.value(2).toString());

        int compVal = q.value(3).toInt();
        m_sliderComp->setValue(compVal);
        onSliderValueChanged(compVal);

        m_ruleTitleLabel->setText(m_comboItemA->currentText() + " + " + m_comboItemB->currentText());
        m_ruleIdLabel->setText("ID: #" + li->data(Qt::UserRole).toString());

        updateItemImage(q.value(0).toInt(), m_imgItemA, 85);
        updateItemImage(q.value(1).toInt(), m_imgItemB, 85);

        m_comboItemA->setEnabled(false); m_comboItemB->setEnabled(false);
        m_sliderComp->setEnabled(false); m_textEditFullDesc->setEnabled(false);

        m_btnEditRule->setText("Редактировать");
        m_btnEditRule->setStyleSheet("QPushButton { border: 1.5px solid #EAEAEA; border-radius: 18px; font-family: 'Century Gothic'; font-size: 12px; font-weight: bold; color: #444; background: white; } QPushButton:hover { background: #F9F9F9; }");
    }
}

// ТОЛЬКО ЦВЕТЫ И СТАФФАЖ
void RulesPage::loadItemsIntoCombos() {
    m_comboItemA->clear(); m_comboItemB->clear();
    QSqlQuery q("SELECT item_id, name FROM Items WHERE item_type IN ('flower', 'staffage') ORDER BY name");
    while(q.next()){
        m_comboItemA->addItem(q.value(1).toString(), q.value(0).toInt());
        m_comboItemB->addItem(q.value(1).toString(), q.value(0).toInt());
    }
}

QPixmap RulesPage::getRoundedPixmap(const QString &path, int size) {
    QPixmap src(path);
    if(src.isNull()) return QPixmap();

    int physicalSize = size * 2;
    QPixmap target(physicalSize, physicalSize);
    target.fill(Qt::transparent);

    QPainter p(&target);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath pathObj;
    pathObj.addEllipse(0, 0, physicalSize, physicalSize);
    p.setClipPath(pathObj);

    QPixmap scaledSrc = src.scaled(physicalSize, physicalSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    int xOffset = (physicalSize - scaledSrc.width()) / 2;
    int yOffset = (physicalSize - scaledSrc.height()) / 2;
    p.drawPixmap(xOffset, yOffset, scaledSrc);

    target.setDevicePixelRatio(2.0);
    return target;
}

void RulesPage::updateItemImage(int id, QLabel* l, int s) {
    if (!l) return;
    QSqlQuery q; q.prepare("SELECT image FROM Item_variation WHERE item_id = :id LIMIT 1");
    q.bindValue(":id", id);
    if(q.exec() && q.next()) {
        l->setPixmap(getRoundedPixmap(q.value(0).toString(), s));
    } else {
        l->clear();
    }
}

void RulesPage::onItemAChanged(int) {
    updateItemImage(m_comboItemA->currentData().toInt(), m_imgItemA, 85);
    if(m_nameItemA) m_nameItemA->setText(m_comboItemA->currentText());
}

void RulesPage::onItemBChanged(int) {
    updateItemImage(m_comboItemB->currentData().toInt(), m_imgItemB, 85);
    if(m_nameItemB) m_nameItemB->setText(m_comboItemB->currentText());
}

void RulesPage::onSliderValueChanged(int v) {
    if(!m_lblCompValue) return;
    m_lblCompValue->setText(QString::number(v));

    if (v < 0) {
        m_lblCompValue->setStyleSheet("font-family: 'Century Gothic'; font-size: 22px; font-weight: bold; color: #D86B7A; border: none;");
    } else if (v == 0) {
        m_lblCompValue->setStyleSheet("font-family: 'Century Gothic'; font-size: 22px; font-weight: bold; color: #F4CB45; border: none;");
    } else {
        m_lblCompValue->setStyleSheet("font-family: 'Century Gothic'; font-size: 22px; font-weight: bold; color: #66CDAA; border: none;");
    }
}

void RulesPage::onSearchTextChanged(const QString &t) {
    for(int i=0; i<m_rulesList->count(); ++i) {
        m_rulesList->item(i)->setHidden(!m_rulesList->item(i)->data(Qt::UserRole+1).toString().contains(t, Qt::CaseInsensitive));
    }
}

void RulesPage::onEditRuleClicked() {
    if (!m_comboItemA || !m_comboItemB || !m_rulesList) return;

    if (!m_isEditing) {
        m_isEditing = true;
        m_comboItemA->setEnabled(true);
        m_comboItemB->setEnabled(true);
        m_sliderComp->setEnabled(true);
        m_textEditFullDesc->setEnabled(true);

        m_btnEditRule->setText("Сохранить");
        m_btnEditRule->setStyleSheet(
            "QPushButton { "
            "  border: 1.5px solid #D86B7A; "
            "  border-radius: 18px; "
            "  color: #D86B7A; "
            "  background: white; "
            "  font-family: 'Century Gothic'; "
            "  font-size: 12px; "
            "  font-weight: bold; "
            "} "
            "QPushButton:hover { background: #FFF0F3; }"
            );
    } else {
        int itemA = m_comboItemA->currentData().toInt();
        int itemB = m_comboItemB->currentData().toInt();

        if (itemA == itemB) {
            showOverlay();
            ErrorDialog e("Ошибка", "Элементы должны быть разными!", this->window());
            e.exec();
            hideOverlay();
            return;
        }

        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isOpen()) return;

        QSqlQuery q;
        if (m_isAddingNew) {
            q.prepare("INSERT INTO CompatibilityRules (item_a_id, item_b_id, description, compatibility_level) "
                      "VALUES (:a, :b, :desc, :comp)");
        } else {
            if (!m_rulesList->currentItem()) return;
            q.prepare("UPDATE CompatibilityRules SET item_a_id = :a, item_b_id = :b, "
                      "description = :desc, compatibility_level = :comp WHERE rule_id = :id");
            q.bindValue(":id", m_rulesList->currentItem()->data(Qt::UserRole).toInt());
        }

        q.bindValue(":a", itemA);
        q.bindValue(":b", itemB);
        q.bindValue(":desc", m_textEditFullDesc->toPlainText().trimmed());
        q.bindValue(":comp", m_sliderComp->value());

        if (q.exec()) {
            int savedId = m_isAddingNew ? q.lastInsertId().toInt() : m_rulesList->currentItem()->data(Qt::UserRole).toInt();

            m_isAddingNew = false;
            m_isEditing = false;

            loadRules();
            for (int i = 0; i < m_rulesList->count(); ++i) {
                if (m_rulesList->item(i)->data(Qt::UserRole).toInt() == savedId) {
                    m_rulesList->setCurrentRow(i);
                    break;
                }
            }
        }
    }
}

void RulesPage::onDeleteRuleClicked() {
    QListWidgetItem *item = m_rulesList->currentItem();
    if (!item) return;

    showOverlay();
    ConfirmDialog d("Удаление", "Удалить это правило навсегда?", this->window());
    if (d.exec() == QDialog::Accepted) {
        QSqlQuery q;
        q.prepare("DELETE FROM CompatibilityRules WHERE rule_id = :id");
        q.bindValue(":id", item->data(Qt::UserRole).toInt());

        if (q.exec()) {
            m_rulesList->blockSignals(true);
            delete m_rulesList->takeItem(m_rulesList->currentRow());
            m_rulesList->clearSelection();
            m_rulesList->blockSignals(false);

            m_mainScroll->hide();
            loadRules();
        }
    }
    hideOverlay();
}

void RulesPage::onAddRuleClicked() {
    m_rulesList->clearSelection();
    m_isAddingNew = true;
    m_isEditing = true;

    m_mainScroll->show();

    m_ruleTitleLabel->setText("Новое правило");
    m_ruleIdLabel->setText("ID: NEW");

    m_comboItemA->setEnabled(true);
    m_comboItemB->setEnabled(true);
    m_sliderComp->setEnabled(true);
    m_textEditFullDesc->setEnabled(true);

    if (m_comboItemA->count() > 0) m_comboItemA->setCurrentIndex(0);
    if (m_comboItemB->count() > 0) m_comboItemB->setCurrentIndex(0);

    m_sliderComp->setValue(0);
    onSliderValueChanged(0);
    m_textEditFullDesc->clear();

    m_btnEditRule->setText("Сохранить");
    m_btnEditRule->setStyleSheet(
        "QPushButton { "
        "  border: 1.5px solid #D86B7A; "
        "  border-radius: 18px; "
        "  color: #D86B7A; "
        "  background: white; "
        "  font-family: 'Century Gothic'; "
        "  font-size: 12px; "
        "  font-weight: bold; "
        "} "
        "QPushButton:hover { background: #FFF0F3; }"
        );
}