#include "seasonalitypage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QSqlQuery>
#include <QSqlError>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QDebug>
#include <QSqlDatabase>
#include <QAction>

// ==========================================================================
// РЕАЛИЗАЦИЯ ДИАЛОГА РЕДАКТИРОВАНИЯ
// ==========================================================================

SeasonEditDialog::SeasonEditDialog(int itemId, const QString &name, const QString &imgPath, const QList<int> &currentMonths, QWidget *parent)
    : QDialog(parent), m_itemId(itemId), m_selectedMonths(currentMonths) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(480, 560);
    setupUI(name, imgPath);
    updateMonthButtons();
}

void SeasonEditDialog::setupUI(const QString &name, const QString &imgPath) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(35, 25, 35, 35);
    mainLayout->setSpacing(20);

    QHBoxLayout *header = new QHBoxLayout();
    QLabel *imgLbl = new QLabel();
    imgLbl->setFixedSize(80, 80);
    imgLbl->setStyleSheet("border: none; background: transparent;");

    if(!imgPath.isEmpty()) {
        QPixmap src(imgPath);
        if(!src.isNull()) {
            int pSize = 80 * 2;
            QPixmap target(pSize, pSize);
            target.fill(Qt::transparent);
            QPainter p(&target);
            p.setRenderHint(QPainter::Antialiasing);
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            QPainterPath pathObj;
            pathObj.addEllipse(0, 0, pSize, pSize);
            p.setClipPath(pathObj);
            p.drawPixmap(0, 0, src.scaled(pSize, pSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            target.setDevicePixelRatio(2.0);
            imgLbl->setPixmap(target);
        }
    }

    QLabel *nameLbl = new QLabel(name);
    nameLbl->setStyleSheet("font-family: 'Century Gothic'; font-size: 22px; font-weight: bold; color: #333; border: none; background: transparent;");

    QPushButton *closeBtn = new QPushButton("×");
    closeBtn->setFixedSize(32, 32);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QPushButton { border: none; font-family: 'Arial'; font-size: 26px; color: #CCC; background: transparent; } QPushButton:hover { color: #D86B7A; }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    header->addWidget(imgLbl);
    header->addSpacing(15);
    header->addWidget(nameLbl);
    header->addStretch();
    header->addWidget(closeBtn, 0, Qt::AlignTop);
    mainLayout->addLayout(header);

    QHBoxLayout *seasonsLayout = new QHBoxLayout();
    seasonsLayout->setSpacing(6);
    QStringList seasons = {"Весна", "Лето", "Осень", "Зима", "Все", "Сброс"};
    for(const QString &s : seasons) {
        QPushButton *btn = new QPushButton(s);
        btn->setCursor(Qt::PointingHandCursor);
        if(s == "Сброс") btn->setStyleSheet("QPushButton { background: transparent; color: #AAA; font-family: 'Century Gothic'; font-size: 13px; border: none; }");
        else btn->setStyleSheet("QPushButton { background: #F5F5F5; color: #555; border-radius: 14px; padding: 6px 14px; font-family: 'Century Gothic'; font-size: 11px; border: none; }");
        btn->setProperty("type", s);
        connect(btn, &QPushButton::clicked, this, &SeasonEditDialog::onSeasonClicked);
        seasonsLayout->addWidget(btn);
    }
    mainLayout->addLayout(seasonsLayout);

    QGridLayout *monthsGrid = new QGridLayout();
    monthsGrid->setSpacing(12);
    QStringList monthNames = {"Янв", "Фев", "Мар", "Апр", "Май", "Июн", "Июл", "Авг", "Сен", "Окт", "Ноя", "Дек"};
    for(int i = 0; i < 12; ++i) {
        m_monthButtons[i] = new QPushButton(monthNames[i]);
        m_monthButtons[i]->setFixedSize(125, 52);
        m_monthButtons[i]->setCheckable(true);
        m_monthButtons[i]->setProperty("monthIdx", i + 1);
        m_monthButtons[i]->setCursor(Qt::PointingHandCursor);
        connect(m_monthButtons[i], &QPushButton::clicked, this, &SeasonEditDialog::onMonthToggled);
        monthsGrid->addWidget(m_monthButtons[i], i / 3, i % 3);
    }
    mainLayout->addLayout(monthsGrid);

    QPushButton *saveBtn = new QPushButton("Сохранить изменения");
    saveBtn->setFixedHeight(54);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet("QPushButton { border: 1.5px solid #EAEAEA; border-radius: 27px; background: white; font-family: 'Century Gothic'; font-size: 15px; font-weight: bold; color: #333; } QPushButton:hover { border-color: #D86B7A; background: #FFF0F3; }");
    connect(saveBtn, &QPushButton::clicked, this, &SeasonEditDialog::onSaveClicked);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(saveBtn);
}

void SeasonEditDialog::updateMonthButtons() {
    const QString colors[12] = { "#A1D4F1", "#A1D4F1", "#FDF1A9", "#FDF1A9", "#FDF1A9", "#BDE2B9", "#BDE2B9", "#BDE2B9", "#F3AD9F", "#F3AD9F", "#F3AD9F", "#A1D4F1" };
    for(int i = 0; i < 12; ++i) {
        bool active = m_selectedMonths.contains(i + 1);
        m_monthButtons[i]->setChecked(active);
        if(active) m_monthButtons[i]->setStyleSheet(QString("QPushButton { background-color: %1; border: none; border-radius: 26px; color: #333; font-family: 'Century Gothic'; font-weight: bold; font-size: 13px; }").arg(colors[i]));
        else m_monthButtons[i]->setStyleSheet("QPushButton { background-color: white; border: 1.5px solid #F0F0F0; border-radius: 26px; color: #CCC; font-family: 'Century Gothic'; font-size: 13px; }");
    }
}

void SeasonEditDialog::onMonthToggled() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int m = btn->property("monthIdx").toInt();
    if(btn->isChecked()) { if(!m_selectedMonths.contains(m)) m_selectedMonths.append(m); }
    else { m_selectedMonths.removeAll(m); }
    updateMonthButtons();
}

void SeasonEditDialog::onSeasonClicked() {
    QString type = sender()->property("type").toString();
    if(type == "Сброс") m_selectedMonths.clear();
    else if(type == "Все") { m_selectedMonths.clear(); for(int i=1; i<=12; ++i) m_selectedMonths << i; }
    else if(type == "Зима") { m_selectedMonths.clear(); m_selectedMonths << 12 << 1 << 2; }
    else if(type == "Весна") { m_selectedMonths.clear(); m_selectedMonths << 3 << 4 << 5; }
    else if(type == "Лето") { m_selectedMonths.clear(); m_selectedMonths << 6 << 7 << 8; }
    else if(type == "Осень") { m_selectedMonths.clear(); m_selectedMonths << 9 << 10 << 11; }
    updateMonthButtons();
}

void SeasonEditDialog::onSaveClicked() {
    QSqlQuery q;
    q.prepare("DELETE FROM Seasonality WHERE item_id = :id");
    q.bindValue(":id", m_itemId);
    q.exec();

    if(!m_selectedMonths.isEmpty()) {
        std::sort(m_selectedMonths.begin(), m_selectedMonths.end());

        int startMonth = m_selectedMonths.first();
        int endMonth = m_selectedMonths.last();

        // Умный поиск начала и конца (спасает зиму и переход через декабрь-январь)
        if (m_selectedMonths.size() > 1 && m_selectedMonths.size() < 12) {
            int maxGap = 0;
            int maxGapIndex = 0;

            // Ищем самый большой разрыв между выбранными месяцами
            for (int i = 0; i < m_selectedMonths.size(); ++i) {
                int current = m_selectedMonths[i];
                int next = m_selectedMonths[(i + 1) % m_selectedMonths.size()];

                int gap = next - current;
                if (gap <= 0) gap += 12; // Учитываем переход года

                if (gap > maxGap) {
                    maxGap = gap;
                    maxGapIndex = i;
                }
            }

            // Месяц ПЕРЕД разрывом — это реальный конец
            // Месяц ПОСЛЕ разрыва — это реальное начало
            endMonth = m_selectedMonths[maxGapIndex];
            startMonth = m_selectedMonths[(maxGapIndex + 1) % m_selectedMonths.size()];
        }

        q.prepare("INSERT INTO Seasonality (item_id, start_month, end_month) VALUES (:id, :s, :e)");
        q.bindValue(":id", m_itemId);
        q.bindValue(":s", startMonth);
        q.bindValue(":e", endMonth);
        q.exec();
    }
    accept();
}

void SeasonEditDialog::paintEvent(QPaintEvent *) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.setBrush(Qt::white); p.setPen(Qt::NoPen); p.drawRoundedRect(rect(), 30, 30);
}

// ==========================================================================
// РЕАЛИЗАЦИЯ ОСНОВНОЙ СТРАНИЦЫ (С КРЕСТИКОМ В ПОИСКЕ)
// ==========================================================================

SeasonalityPage::SeasonalityPage(QWidget *parent) : QWidget(parent) {
    setStyleSheet("background-color: #FFFFFF;");
    setupUI();
    loadData();
}

void SeasonalityPage::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 25, 20, 25);
    mainLayout->setSpacing(25);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(3);
    QLabel *titleLbl = new QLabel("Календарь сезонности", this);
    titleLbl->setStyleSheet("font-family: 'Century Gothic'; font-size: 24px; font-weight: bold; color: #333; border: none; background: transparent;");
    QLabel *subTitleLbl = new QLabel("Нажмите на карточку, чтобы настроить доступные месяцы", this);
    subTitleLbl->setStyleSheet("font-family: 'Century Gothic'; font-size: 12px; color: #999; border: none; background: transparent;");
    titleLayout->addWidget(titleLbl); titleLayout->addWidget(subTitleLbl);
    headerLayout->addLayout(titleLayout); headerLayout->addStretch();

    // ПОИСКОВАЯ СТРОКА С КРЕСТИКОМ
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Поиск...");
    m_searchEdit->setFixedSize(260, 40);
    m_searchEdit->setStyleSheet(
        "QLineEdit { "
        "  border: 1px solid #EAEAEA; "
        "  border-radius: 20px; "
        "  padding-left: 18px; "
        "  padding-right: 35px; " // Место под крестик
        "  font-family: 'Century Gothic'; "
        "  font-size: 13px; "
        "  background-color: #FAFAFA; "
        "}"
        );

    // Добавляем крестик как Action внутри QLineEdit
    m_clearAction = m_searchEdit->addAction(QIcon(), QLineEdit::TrailingPosition);
    m_clearAction->setVisible(false); // Прячем изначально

    // Создаем маленькую серую иконку крестика программно (чтобы не искать файл)
    QPixmap clearPix(20, 20);
    clearPix.fill(Qt::transparent);
    QPainter p(&clearPix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor("#CCCCCC"), 2));
    p.drawLine(6, 6, 14, 14);
    p.drawLine(14, 6, 6, 14);
    m_clearAction->setIcon(QIcon(clearPix));

    connect(m_clearAction, &QAction::triggered, m_searchEdit, &QLineEdit::clear);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &SeasonalityPage::onSearchTextChanged);

    headerLayout->addWidget(m_searchEdit, 0, Qt::AlignVCenter);
    mainLayout->addLayout(headerLayout);

    m_mainScroll = new QScrollArea(this);
    m_mainScroll->setWidgetResizable(true);
    m_mainScroll->setFrameShape(QFrame::NoFrame);
    m_mainScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_mainScroll->setStyleSheet("QScrollArea { background: transparent; border: none; } QScrollBar:vertical { width: 6px; background: transparent; } QScrollBar::handle:vertical { background: #DCDCDC; border-radius: 3px; }");

    m_scrollContent = new QWidget();
    m_gridLayout = new QGridLayout(m_scrollContent);
    m_gridLayout->setContentsMargins(0, 5, 5, 5);
    m_gridLayout->setSpacing(20);
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_mainScroll->setWidget(m_scrollContent);
    mainLayout->addWidget(m_mainScroll);
}

void SeasonalityPage::onSearchTextChanged(const QString &text) {
    if (!m_gridLayout) return;

    // Показываем/скрываем крестик
    if (m_clearAction) {
        m_clearAction->setVisible(!text.isEmpty());
    }

    // 1. Ищем ВСЕ карточки среди дочерних элементов m_scrollContent
    QList<QWidget*> allCards;
    for (QObject *child : m_scrollContent->children()) {
        QWidget *w = qobject_cast<QWidget*>(child);
        // Проверяем, что это именно наша карточка (по наличию свойства itemId)
        if (w && w->property("itemId").isValid()) {
            allCards.append(w);
        }
    }

    // 2. Убираем все найденные карточки из слоя (чтобы избежать дублей и перестроить сетку)
    for (QWidget *w : allCards) {
        m_gridLayout->removeWidget(w);
    }

    // 3. Заново добавляем только те карточки, которые подходят под запрос
    int visibleIndex = 0;
    for (QWidget *w : allCards) {
        bool matches = w->property("itemName").toString().contains(text, Qt::CaseInsensitive);
        w->setVisible(matches);

        if (matches) {
            m_gridLayout->addWidget(w, visibleIndex / 4, visibleIndex % 4);
            visibleIndex++;
        }
    }
}

QWidget* SeasonalityPage::createCard(int itemId, const QString &name, const QString &imgPath, const QList<int> &activeMonths) {
    QFrame *card = new QFrame();
    card->setFixedSize(195, 240);
    card->setProperty("itemId", itemId);
    card->setProperty("itemName", name);
    card->setProperty("months", QVariant::fromValue(activeMonths));
    card->setProperty("imgPath", imgPath);
    card->setCursor(Qt::PointingHandCursor);
    card->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #F0F0F0; border-radius: 24px; } QFrame:hover { border: 1px solid #F8C3CD; background-color: #FEFAFB; }");
    card->installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(15, 20, 15, 20); layout->setSpacing(12);
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QLabel *imgLbl = new QLabel(card);
    imgLbl->setFixedSize(110, 110);
    imgLbl->setAlignment(Qt::AlignCenter);
    imgLbl->setStyleSheet("border: none; background: transparent;");
    if (!imgPath.isEmpty()) imgLbl->setPixmap(getRoundedPixmap(imgPath, 110));
    else imgLbl->setStyleSheet("border: none; border-radius: 55px; background: #FAFAFA;");

    QLabel *nameLbl = new QLabel(name, card);
    nameLbl->setAlignment(Qt::AlignCenter); nameLbl->setWordWrap(true);
    nameLbl->setStyleSheet("font-family: 'Century Gothic'; font-size: 18px; font-weight: bold; color: #333; border: none;");

    QWidget *monthsWidget = new QWidget(card);
    monthsWidget->setStyleSheet("border: none; background: transparent;");
    QGridLayout *monthsGrid = new QGridLayout(monthsWidget);
    monthsGrid->setContentsMargins(0, 0, 0, 0); monthsGrid->setSpacing(6);

    const QString colors[12] = { "#A1D4F1", "#A1D4F1", "#FDF1A9", "#FDF1A9", "#FDF1A9", "#BDE2B9", "#BDE2B9", "#BDE2B9", "#F3AD9F", "#F3AD9F", "#F3AD9F", "#A1D4F1" };

    for (int i = 0; i < 12; ++i) {
        QLabel *dot = new QLabel();
        dot->setFixedSize(14, 14);
        if (activeMonths.contains(i + 1)) dot->setStyleSheet(QString("background-color: %1; border-radius: 7px; border: none;").arg(colors[i]));
        else dot->setStyleSheet("background-color: transparent; border: 1.2px solid #EAEAEA; border-radius: 7px;");
        monthsGrid->addWidget(dot, i / 6, i % 6);
    }

    layout->addWidget(imgLbl, 0, Qt::AlignHCenter);
    layout->addWidget(nameLbl, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(monthsWidget, 0, Qt::AlignHCenter);
    return card;
}

void SeasonalityPage::loadData() {
    QLayoutItem *child;
    while ((child = m_gridLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    QSqlQuery q("SELECT i.item_id, i.name, (SELECT image FROM Item_variation WHERE item_id = i.item_id LIMIT 1), s.start_month, s.end_month "
                "FROM Items i LEFT JOIN Seasonality s ON i.item_id = s.item_id WHERE i.item_type IN ('flower', 'staffage') ORDER BY i.name");

    int index = 0;
    while (q.next()) {
        QList<int> activeMonths;
        if (!q.value(3).isNull()) {
            int s = q.value(3).toInt(), e = q.value(4).toInt();
            if (s <= e) for (int m = s; m <= e; ++m) activeMonths << m;
            else { for (int m = s; m <= 12; ++m) activeMonths << m; for (int m = 1; m <= e; ++m) activeMonths << m; }
        }
        m_gridLayout->addWidget(createCard(q.value(0).toInt(), q.value(1).toString(), q.value(2).toString(), activeMonths), index / 4, index % 4);
        index++;
    }
}

bool SeasonalityPage::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QFrame *card = qobject_cast<QFrame*>(watched);
        if (card) {
            showOverlay();
            SeasonEditDialog dlg(card->property("itemId").toInt(), card->property("itemName").toString(),
                                 card->property("imgPath").toString(), card->property("months").value<QList<int>>(), this->window());

            dlg.move(this->window()->geometry().center() - dlg.rect().center());

            if(dlg.exec() == QDialog::Accepted) loadData();
            hideOverlay();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void SeasonalityPage::showOverlay() {
    m_overlay = new QWidget(this->window());
    m_overlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);");
    m_overlay->resize(this->window()->size()); m_overlay->show();
}

void SeasonalityPage::hideOverlay() { if (m_overlay) { m_overlay->deleteLater(); m_overlay = nullptr; } }

QPixmap SeasonalityPage::getRoundedPixmap(const QString &path, int size) {
    QPixmap src(path); if(src.isNull()) return QPixmap();
    int pSize = size * 2; QPixmap target(pSize, pSize); target.fill(Qt::transparent);
    QPainter p(&target); p.setRenderHint(QPainter::Antialiasing); p.setRenderHint(QPainter::SmoothPixmapTransform);
    QPainterPath pathObj; pathObj.addEllipse(0, 0, pSize, pSize); p.setClipPath(pathObj);
    p.drawPixmap(0, 0, src.scaled(pSize, pSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    target.setDevicePixelRatio(2.0); return target;
}