#include "constructorfilterdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPainter>
#include <QSqlQuery>
#include <QSqlError>
#include <QFrame>

ConstructorFilterDialog::ConstructorFilterDialog(int catId, const FilterSettings &current, QWidget *parent)
    : QDialog(parent), m_catId(catId), m_settings(current)
{
    // Безопасное окно без системных рамок
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // ФИКСИРУЕМ ТОЛЬКО ШИРИНУ. Высота будет считаться автоматически!
    setFixedWidth(540);

    setupUI();
    loadDynamicData();

    // Заставляем окно "обжать" все элементы по высоте, убирая пустое пространство
    adjustSize();
}

// Строгое центрирование окна поверх родительского виджета при появлении
void ConstructorFilterDialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    if (parentWidget()) {
        QRect parentRect = parentWidget()->window()->geometry();
        // Рассчитываем центр с учетом новой, динамической высоты окна
        move(parentRect.center() - rect().center());
    }
}

void ConstructorFilterDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(35, 20, 35, 25);
    mainLayout->setSpacing(15);

    // --- ШАПКА ---
    QHBoxLayout *header = new QHBoxLayout();
    QLabel *title = new QLabel("Фильтры", this);
    title->setStyleSheet("font-family: 'Century Gothic'; font-size: 20px; font-weight: bold; color: #111;");

    QPushButton *closeBtn = new QPushButton("✕", this);
    closeBtn->setFixedSize(30, 30);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("background: transparent; font-size: 20px; color: #777; border: none; font-weight: bold;");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    header->addStretch();
    header->addWidget(title);
    header->addStretch();
    header->addWidget(closeBtn);
    mainLayout->addLayout(header);

    // Лямбда для создания строк
    auto addRow = [&](QGridLayout *g, const QString &lblText, const QString &key, int r, int c) {
        QLabel *l = new QLabel(lblText, this);
        l->setStyleSheet("font-family: 'Century Gothic'; font-size: 14px; color: #444;");
        g->addWidget(l, r, c * 2, Qt::AlignRight | Qt::AlignVCenter);
        g->addWidget(createStyledCombo(key), r, c * 2 + 1);
    };

    // --- ОБЩИЕ ФИЛЬТРЫ ---
    QGridLayout *topGrid = new QGridLayout();
    topGrid->setSpacing(15);
    addRow(topGrid, "Стиль:", "style", 0, 0);
    addRow(topGrid, "Цвет:", "color", 0, 1);
    mainLayout->addLayout(topGrid);

    QFrame *line1 = new QFrame(this);
    line1->setFixedHeight(1);
    line1->setStyleSheet("background: #CCC; border: none;");
    mainLayout->addWidget(line1);

    // --- ДИНАМИЧЕСКИЕ ФИЛЬТРЫ ---
    // Создаем только те поля, которые нужны для текущей категории
    QGridLayout *dynGrid = new QGridLayout();
    dynGrid->setSpacing(12);
    dynGrid->setHorizontalSpacing(20);

    if (m_catId <= 1) { // Цветы и Стаффаж
        addRow(dynGrid, "Тип:", "type", 0, 0);             addRow(dynGrid, "Безопасн.:", "safety", 0, 1);
        addRow(dynGrid, "Пыльца:", "pollen", 1, 0);        addRow(dynGrid, "Аромат:", "scent", 1, 1);
        addRow(dynGrid, "Бутон:", "bud", 2, 0);            addRow(dynGrid, "Стебель:", "stem", 2, 1);
        addRow(dynGrid, "Стойкость:", "durability", 3, 0); addRow(dynGrid, "Уход:", "care", 3, 1);
    } else if (m_catId == 2) { // Упаковка
        addRow(dynGrid, "Материал:", "material", 0, 0);    addRow(dynGrid, "Водостойк.:", "waterproof", 0, 1);
    } else if (m_catId == 3) { // Аксессуары
        addRow(dynGrid, "Кастом:", "customizable", 0, 0);  addRow(dynGrid, "Крепление:", "attachment", 0, 1);
    }
    mainLayout->addLayout(dynGrid);

    // Пружина (addStretch) УБРАНА отсюда, чтобы окно не создавало пустое пространство внизу!

    QFrame *line2 = new QFrame(this);
    line2->setFixedHeight(1);
    line2->setStyleSheet("background: #CCC; border: none;");
    mainLayout->addWidget(line2);

    // --- КНОПКИ ВНИЗУ ---
    QHBoxLayout *btns = new QHBoxLayout();

    QPushButton *resetBtn = new QPushButton("Сбросить", this);
    resetBtn->setFixedSize(130, 45);
    resetBtn->setCursor(Qt::PointingHandCursor);
    resetBtn->setStyleSheet("QPushButton { background: white; border: 1px solid #CCC; border-radius: 8px; font-weight: bold; font-family: 'Century Gothic'; font-size: 14px; color: #111; }");
    connect(resetBtn, &QPushButton::clicked, this, [this](){
        for(auto c : m_combos) c->setCurrentIndex(0);
    });

    QPushButton *cancelBtn = new QPushButton("Отмена", this);
    cancelBtn->setFixedSize(110, 45);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet("QPushButton { background: white; border: 1px solid #CCC; border-radius: 8px; font-weight: bold; font-family: 'Century Gothic'; font-size: 14px; color: #111; }");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *applyBtn = new QPushButton("Применить", this);
    applyBtn->setFixedSize(130, 45);
    applyBtn->setCursor(Qt::PointingHandCursor);
    applyBtn->setStyleSheet("QPushButton { background: #BDBDBD; color: #111; border-radius: 8px; font-weight: bold; font-family: 'Century Gothic'; font-size: 14px; border: none; }");
    connect(applyBtn, &QPushButton::clicked, this, &QDialog::accept);

    btns->addWidget(resetBtn);
    btns->addStretch();
    btns->addWidget(cancelBtn);
    btns->addWidget(applyBtn);
    mainLayout->addLayout(btns);
}

QComboBox* ConstructorFilterDialog::createStyledCombo(const QString &key) {
    QComboBox *cb = new QComboBox(this);
    cb->addItem("Все");
    cb->setFixedHeight(36);
    cb->setStyleSheet(
        "QComboBox { border: 1px solid #CCC; border-radius: 8px; padding: 2px 10px; background: white; font-family: 'Century Gothic'; font-size: 13px; color: #333; } "
        "QComboBox::drop-down { border: none; width: 30px; } "
        "QComboBox::down-arrow { image: none; border-top: 3px solid #777; width: 10px; margin-right: 10px; height: 0px; }"
        );
    m_combos.insert(key, cb);
    return cb;
}

void ConstructorFilterDialog::loadDynamicData() {
    auto fill = [this](const QString &key, const QString &query, const QString &currentVal) {
        if (!m_combos.contains(key)) return;
        QComboBox* cb = m_combos.value(key);
        QSqlQuery q(query);
        while(q.next()) {
            QString val = q.value(0).toString();
            if (!val.isEmpty() && cb->findText(val) == -1) cb->addItem(val);
        }
        int idx = cb->findText(currentVal);
        if (idx != -1) cb->setCurrentIndex(idx);
    };

    fill("style", "SELECT DISTINCT style FROM Items WHERE style IS NOT NULL", m_settings.style);
    fill("color", "SELECT DISTINCT color_name FROM Colors", m_settings.color);

    if (m_catId <= 1) {
        fill("pollen", "SELECT DISTINCT pollen_level FROM Flower_details", m_settings.pollen);
        fill("scent", "SELECT DISTINCT scent_intensity FROM Flower_details", m_settings.scent);
        fill("bud", "SELECT DISTINCT bud_size FROM Flower_details", m_settings.bud);
        fill("stem", "SELECT DISTINCT stem_length_cm FROM Flower_details", m_settings.stem);
        fill("durability", "SELECT DISTINCT lifetime FROM Flower_details", m_settings.durability);
        fill("care", "SELECT DISTINCT care_complexity FROM Flower_details", m_settings.care);
        if(m_combos.contains("safety")) { m_combos["safety"]->addItems({"Да", "Нет"}); m_combos["safety"]->setCurrentText(m_settings.safety); }
        if(m_combos.contains("type")) { m_combos["type"]->addItems({"Основные", "Стаффаж"}); m_combos["type"]->setCurrentText(m_settings.type); }
    } else if (m_catId == 2) {
        fill("material", "SELECT DISTINCT material FROM Packaging_details", m_settings.material);
        if(m_combos.contains("waterproof")) { m_combos["waterproof"]->addItems({"Да", "Нет"}); m_combos["waterproof"]->setCurrentText(m_settings.waterproof); }
    } else if (m_catId == 3) {
        fill("attachment", "SELECT DISTINCT attachment_method FROM Accessories_details", m_settings.attachment);
        if(m_combos.contains("customizable")) { m_combos["customizable"]->addItems({"Да", "Нет"}); m_combos["customizable"]->setCurrentText(m_settings.customizable); }
    }
}

FilterSettings ConstructorFilterDialog::getSettings() const {
    FilterSettings s;
    auto getVal = [this](const QString &key) {
        return m_combos.contains(key) ? m_combos.value(key)->currentText() : "Все";
    };

    s.style = getVal("style");           s.color = getVal("color");
    s.pollen = getVal("pollen");         s.scent = getVal("scent");
    s.bud = getVal("bud");               s.stem = getVal("stem");
    s.durability = getVal("durability"); s.care = getVal("care");
    s.safety = getVal("safety");         s.type = getVal("type");
    s.material = getVal("material");     s.waterproof = getVal("waterproof");
    s.customizable = getVal("customizable"); s.attachment = getVal("attachment");
    return s;
}

void ConstructorFilterDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::white);
    p.setPen(QPen(QColor("#CCCCCC"), 1));
    // Отрисовка обводки подстраивается под новый динамический размер
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 16, 16);
}