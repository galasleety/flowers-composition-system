#include "detailscolumn.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSet>
#include <QMap>
#include <QDebug>
#include <QScrollBar>

DetailsColumn::DetailsColumn(QWidget *parent) : QWidget(parent) {
    setFixedWidth(300);
    setupUI();
}

void DetailsColumn::setupUI() {
    QVBoxLayout *baseLayout = new QVBoxLayout(this);
    baseLayout->setContentsMargins(0, 0, 0, 0);
    baseLayout->setSpacing(15); // Отступ между блоком анализа и блоком состава

    QString scrollStyle =
        "QScrollBar:vertical { border: none; background: transparent; width: 6px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #D3D3D3; border-radius: 3px; min-height: 20px; }"
        "QScrollBar::handle:vertical:hover { background: #BDBDBD; }";

    // ==========================================
    // ВЕРХНИЙ БЛОК: АНАЛИЗ БУКЕТА (Занимает 2/3)
    // ==========================================
    QWidget *analysisWidget = new QWidget(this);
    QVBoxLayout *analysisLayout = new QVBoxLayout(analysisWidget);
    analysisLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *analysisTitle = new QLabel("Анализ букета", analysisWidget);
    analysisTitle->setStyleSheet("font-family: 'Century Gothic'; font-size: 18px; color: #444; margin-left: 20px; margin-top: 15px;");
    analysisLayout->addWidget(analysisTitle);

    QScrollArea *analysisScroll = new QScrollArea(analysisWidget);
    analysisScroll->setFrameShape(QFrame::NoFrame);
    analysisScroll->setWidgetResizable(true);
    analysisScroll->setStyleSheet("background: transparent; border: none;");
    analysisScroll->verticalScrollBar()->setStyleSheet(scrollStyle);

    QWidget *analysisContent = new QWidget();
    QVBoxLayout *aContentLayout = new QVBoxLayout(analysisContent);
    aContentLayout->setContentsMargins(20, 10, 15, 20);
    aContentLayout->setSpacing(20);
    aContentLayout->setAlignment(Qt::AlignTop);

    m_colorsLayout = new QHBoxLayout();
    m_colorsLayout->setSpacing(8);
    m_colorsLayout->setAlignment(Qt::AlignLeft);
    aContentLayout->addLayout(m_colorsLayout);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #EAEAEA;");
    aContentLayout->addWidget(line);

    aContentLayout->addWidget(createAnalysisBlock("Цветовая гармония", m_harmonyVal, m_harmonyDesc));
    aContentLayout->addWidget(createAnalysisBlock("Ботаническая совместимость", m_compatVal, m_compatDesc));
    aContentLayout->addWidget(createAnalysisBlock("Итоговый стиль", m_styleVal, m_styleDesc));
    aContentLayout->addWidget(createAnalysisBlock("Ожидаемая стойкость", m_lifeVal, m_lifeDesc));
    aContentLayout->addWidget(createAnalysisBlock("Ароматика", m_scentVal, m_scentDesc));
    aContentLayout->addWidget(createAnalysisBlock("Безопасность", m_toxVal, m_toxDesc));

    analysisScroll->setWidget(analysisContent);
    analysisLayout->addWidget(analysisScroll);


    // ==========================================
    // НИЖНИЙ БЛОК: СОСТАВ КОМПОЗИЦИИ (Занимает 1/3)
    // ==========================================
    QWidget *compositionWidget = new QWidget(this);
    QVBoxLayout *compositionLayout = new QVBoxLayout(compositionWidget);
    compositionLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *divLine = new QFrame();
    divLine->setFrameShape(QFrame::HLine);
    divLine->setStyleSheet("color: #EAEAEA; margin: 0 20px;");
    compositionLayout->addWidget(divLine);

    QLabel *compositionTitle = new QLabel("Состав композиции", compositionWidget);
    compositionTitle->setStyleSheet("font-family: 'Century Gothic'; font-size: 18px; color: #444; margin-left: 20px; margin-top: 10px; margin-bottom: 5px;");
    compositionLayout->addWidget(compositionTitle);

    QScrollArea *compScroll = new QScrollArea(compositionWidget);
    compScroll->setFrameShape(QFrame::NoFrame);
    compScroll->setWidgetResizable(true);
    compScroll->setStyleSheet("background: transparent; border: none;");
    compScroll->verticalScrollBar()->setStyleSheet(scrollStyle);

    QWidget *compContent = new QWidget();
    m_compListLayout = new QVBoxLayout(compContent);
    m_compListLayout->setContentsMargins(20, 5, 15, 15);
    m_compListLayout->setSpacing(12);
    m_compListLayout->setAlignment(Qt::AlignTop);

    compScroll->setWidget(compContent);
    compositionLayout->addWidget(compScroll);

    // Добавляем блоки в главный слой с пропорцией 2 к 1
    baseLayout->addWidget(analysisWidget, 2);
    baseLayout->addWidget(compositionWidget, 1);
}

QWidget* DetailsColumn::createAnalysisBlock(const QString &title, QLabel* &valLbl, QLabel* &descLbl) {
    QWidget *w = new QWidget();
    QVBoxLayout *l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(4);

    QLabel *t = new QLabel(title);
    t->setStyleSheet("color: #999; font-family: 'Century Gothic'; font-size: 11px;");

    valLbl = new QLabel("—");
    valLbl->setStyleSheet("font-weight: bold; color: #888; font-family: 'Century Gothic'; font-size: 16px;");

    descLbl = new QLabel("Добавьте элементы для анализа");
    descLbl->setStyleSheet("color: #777; font-family: 'Century Gothic'; font-size: 14px;");
    descLbl->setWordWrap(true);

    l->addWidget(t);
    l->addWidget(valLbl);
    l->addWidget(descLbl);
    return w;
}

// Главный слот, который вызывает сразу обе функции: анализ и отрисовку состава
void DetailsColumn::updateAnalysis(const QList<int> &variationIds) {
    // 1. Сначала обновляем нижний список состава
    updateComposition(variationIds);

    // Цветовая палитра статусов
    QString colorGreen = "#27AE60"; // Отлично
    QString colorGray  = "#7F8C8D"; // Нормально / Нейтрально
    QString colorRed   = "#E74C3C"; // Конфликт / Внимание

    // Очистка палитры квадратиков
    while (QLayoutItem *item = m_colorsLayout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    // Если холст пустой — сбрасываем тексты
    if (variationIds.isEmpty()) {
        QString defStyle = QString("font-weight: bold; color: %1;").arg(colorGray);
        m_harmonyVal->setText("—"); m_harmonyVal->setStyleSheet(defStyle); m_harmonyDesc->setText("Добавьте элементы");
        m_compatVal->setText("—");  m_compatVal->setStyleSheet(defStyle);  m_compatDesc->setText("Добавьте элементы");
        m_styleVal->setText("—");   m_styleVal->setStyleSheet(defStyle);   m_styleDesc->setText("Добавьте элементы");
        m_lifeVal->setText("—");    m_lifeVal->setStyleSheet(defStyle);    m_lifeDesc->setText("Добавьте элементы");
        m_scentVal->setText("—");   m_scentVal->setStyleSheet(defStyle);   m_scentDesc->setText("Добавьте элементы");
        m_toxVal->setText("—");     m_toxVal->setStyleSheet(defStyle);     m_toxDesc->setText("Добавьте элементы");
        return;
    }

    QSet<QString> hexColors;
    QSet<int> itemIds;
    QSet<QString> styles;

    int minLifetime = 999;
    bool hasStrongScent = false;
    bool hasWeakScent = false;
    bool isToxic = false;

    // СБОР ДАННЫХ ИЗ БАЗЫ
    foreach (int varId, variationIds) {
        QSqlQuery q;
        q.prepare("SELECT c.color_hex, i.item_id, i.style, "
                  "f.lifetime, f.scent_intensity, f.is_poisonous "
                  "FROM Item_variation v "
                  "LEFT JOIN Colors c ON v.color_id = c.color_id "
                  "JOIN Items i ON v.item_id = i.item_id "
                  "LEFT JOIN Flower_details f ON i.item_id = f.item_id "
                  "WHERE v.variation_id = :id");
        q.bindValue(":id", varId);

        if (q.exec() && q.next()) {
            if (!q.value(0).isNull()) hexColors.insert(q.value(0).toString());
            itemIds.insert(q.value(1).toInt());
            if (!q.value(2).isNull() && q.value(2).toString() != "") styles.insert(q.value(2).toString());

            if (!q.value(3).isNull()) {
                int life = q.value(3).toInt();
                if (life > 0 && life < minLifetime) minLifetime = life;
            }
            if (!q.value(4).isNull()) {
                QString scent = q.value(4).toString();
                if (scent == "Сильный") hasStrongScent = true;
                if (scent == "Слабый") hasWeakScent = true;
            }
            if (!q.value(5).isNull() && q.value(5).toInt() == 1) {
                isToxic = true;
            }
        }
    }

    // ОТРИСОВКА ЦВЕТОВ (КВАДРАТИКОВ)
    foreach (const QString &hex, hexColors) {
        QFrame *box = new QFrame();
        box->setFixedSize(22, 22);
        box->setStyleSheet(QString("background-color: %1; border-radius: 4px; border: 1px solid #CCC;").arg(hex));
        m_colorsLayout->addWidget(box);
    }
    m_colorsLayout->addStretch();

    // --- 1. ГАРМОНИЯ ---
    if (hexColors.size() == 1) {
        m_harmonyVal->setText("Монохромная");
        m_harmonyVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGreen));
        m_harmonyDesc->setText("Элегантная композиция в оттенках одного цвета.");
    } else if (hexColors.size() <= 3) {
        m_harmonyVal->setText("Гармоничная");
        m_harmonyVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGreen));
        m_harmonyDesc->setText("Сбалансированное сочетание оттенков.");
    } else {
        m_harmonyVal->setText("Контрастная");
        m_harmonyVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGray));
        m_harmonyDesc->setText("Многоцветный и яркий букет.");
    }

    // --- 2. СОВМЕСТИМОСТЬ ---
    bool hasConflict = false;
    QString conflictReason;
    QList<int> ids = itemIds.values();
    for (int i = 0; i < ids.size(); ++i) {
        for (int j = i + 1; j < ids.size(); ++j) {
            QSqlQuery q;
            q.prepare("SELECT description FROM CompatibilityRules WHERE "
                      "((item_a_id = :a AND item_b_id = :b) OR (item_a_id = :b AND item_b_id = :a)) "
                      "AND compatibility_level < 0");
            q.bindValue(":a", ids[i]);
            q.bindValue(":b", ids[j]);
            if (q.exec() && q.next()) {
                hasConflict = true;
                conflictReason = q.value(0).toString();
                break;
            }
        }
        if (hasConflict) break;
    }

    if (hasConflict) {
        m_compatVal->setText("Критическая");
        m_compatVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorRed));
        m_compatDesc->setText(conflictReason);
    } else if (ids.size() > 1) {
        m_compatVal->setText("Отличная");
        m_compatVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGreen));
        m_compatDesc->setText("Растения благоприятно соседствуют друг с другом.");
    } else {
        m_compatVal->setText("Монобукет");
        m_compatVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGreen));
        m_compatDesc->setText("Идеальная совместимость одного вида.");
    }

    // --- 3. СТОЙКОСТЬ ---
    if (minLifetime == 999) {
        m_lifeVal->setText("Не ограничена");
        m_lifeVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGray));
        m_lifeDesc->setText("Композиция состоит только из декора или упаковки.");
    } else if (minLifetime > 10) {
        m_lifeVal->setText(QString("До %1 дней").arg(minLifetime));
        m_lifeVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGreen));
        m_lifeDesc->setText("Букет будет долго сохранять свою свежесть.");
    } else {
        m_lifeVal->setText(QString("До %1 дней").arg(minLifetime));
        m_lifeVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGray));
        m_lifeDesc->setText("Стандартная стойкость для выбранных компонентов.");
    }

    // --- 4. СТИЛЬ ---
    if (styles.isEmpty()) {
        m_styleVal->setText("Не определен");
        m_styleVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGray));
        m_styleDesc->setText("Добавьте цветы для определения стиля.");
    } else if (styles.size() == 1) {
        m_styleVal->setText(styles.values().first());
        m_styleVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGreen));
        m_styleDesc->setText("Букет строго выдержан в единой стилистике.");
    } else {
        m_styleVal->setText("Эклектика");
        m_styleVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGray));
        m_styleDesc->setText("Смешение стилей: " + styles.values().join(" + "));
    }

    // --- 5. АРОМАТИКА ---
    if (hasStrongScent) {
        m_scentVal->setText("Ярко выраженный");
        m_scentVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGray));
        m_scentDesc->setText("Букет обладает сильным цветочным запахом.");
    } else if (hasWeakScent) {
        m_scentVal->setText("Легкий шлейф");
        m_scentVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGreen));
        m_scentDesc->setText("Ненавязчивый, приятный природный аромат.");
    } else {
        m_scentVal->setText("Отсутствует");
        m_scentVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGray));
        m_scentDesc->setText("Букет нейтрален и не имеет выраженного запаха.");
    }

    // --- 6. БЕЗОПАСНОСТЬ ---
    if (isToxic) {
        m_toxVal->setText("Внимание (Токсично)");
        m_toxVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorRed));
        m_toxDesc->setText("В составе присутствуют ядовитые растения. Держите вдали от детей.");
    } else {
        m_toxVal->setText("Безопасно");
        m_toxVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(colorGreen));
        m_toxDesc->setText("Все компоненты безопасны для дома и животных.");
    }
}

// ==========================================
// ЛОГИКА ОТРИСОВКИ СОСТАВА (НИЖНИЙ БЛОК)
// ==========================================
void DetailsColumn::updateComposition(const QList<int> &variationIds) {
    // 1. Очищаем старый список
    while (QLayoutItem *item = m_compListLayout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    if (variationIds.isEmpty()) return;

    // 2. Группируем элементы и считаем их количество (ID -> Количество)
    QMap<int, int> counts;
    for (int id : variationIds) {
        counts[id]++;
    }

    // 3. Создаем виджеты для каждого уникального элемента
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        int varId = it.key();
        int qty = it.value();

        QString name = "Неизвестно";
        QString colorName = "";

        // Достаем названия из базы
        QSqlQuery q;
        q.prepare("SELECT i.name, c.color_name FROM Item_variation v "
                  "JOIN Items i ON v.item_id = i.item_id "
                  "LEFT JOIN Colors c ON v.color_id = c.color_id "
                  "WHERE v.variation_id = :id");
        q.bindValue(":id", varId);
        if (q.exec() && q.next()) {
            name = q.value(0).toString();
            colorName = q.value(1).toString();
        }

        // Контейнер одной позиции состава
        QWidget *itemWidget = new QWidget();
        QVBoxLayout *itemLayout = new QVBoxLayout(itemWidget);
        itemLayout->setContentsMargins(0, 0, 0, 5); // Небольшой отступ снизу
        itemLayout->setSpacing(2);

        // Верхняя строка: Название .... [Кол-во]
        QHBoxLayout *topRow = new QHBoxLayout();
        topRow->setContentsMargins(0, 0, 0, 0);

        QLabel *nameLbl = new QLabel(name);
        nameLbl->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; color: #444;");

        // Тот самый пунктир как на макете
        QFrame *dotLine = new QFrame();
        dotLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        dotLine->setFixedHeight(12);
        dotLine->setStyleSheet("border-bottom: 2px dotted #CCCCCC; margin: 0 5px;");

        // Серый бейджик количества
        QLabel *qtyLbl = new QLabel(QString("x%1").arg(qty));
        qtyLbl->setAlignment(Qt::AlignCenter);
        qtyLbl->setFixedSize(26, 20);
        qtyLbl->setStyleSheet("background-color: #E8E8E8; color: #666; font-family: 'Century Gothic'; font-size: 11px; font-weight: bold; border-radius: 4px;");

        topRow->addWidget(nameLbl);
        topRow->addWidget(dotLine);
        topRow->addWidget(qtyLbl);

        itemLayout->addLayout(topRow);

        // Нижняя строка: Цвет (светло-серым шрифтом)
        if (!colorName.isEmpty()) {
            QLabel *colorLbl = new QLabel(colorName);
            colorLbl->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; color: #999; margin-top: -2px;");
            itemLayout->addWidget(colorLbl);
        }

        m_compListLayout->addWidget(itemWidget);
    }
    m_compListLayout->addStretch();
}