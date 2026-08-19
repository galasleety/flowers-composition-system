#include "catalogtopbar.h"

#include <QHBoxLayout>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStandardItem>
#include <QTimer>

CatalogTopBar::CatalogTopBar(QWidget *parent) : QWidget(parent)
{
    m_searchModel = nullptr;
    setupUI();
    updateSearchCompleter();
}

QString CatalogTopBar::currentCategory() const { return m_categoryCombo->currentText(); }
QString CatalogTopBar::currentSearchText() const { return m_searchEdit->text().trimmed(); }
QString CatalogTopBar::currentSortText() const { return m_sortCombo->currentText(); }

void CatalogTopBar::setupUI()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(15);
    layout->addStretch();

    // Увеличили высоту визуально: радиус 20px (для высоты 40px) и шрифт 14px
    QString comboStyle =
        "QComboBox { background-color: #F2F2F2; border: none; border-radius: 20px; padding: 0px 20px; font-family: 'Century Gothic'; font-size: 14px; color: #333333; } "
        "QComboBox::drop-down { border: none; width: 40px; } "
        "QComboBox::down-arrow { width: 12px; height: 5px; background: #333333; } "
        "QComboBox QAbstractItemView { border: 1px solid #E0E0E0; border-radius: 8px; background-color: #FFFFFF; selection-background-color: #F5F5F5; selection-color: #111111; }";

    m_categoryCombo = new QComboBox();
    m_categoryCombo->addItems({"Все категории", "Основные цветы", "Стаффаж", "Упаковка", "Аксессуары"});
    m_categoryCombo->setFixedSize(220, 40); // Высота 40
    m_categoryCombo->setStyleSheet(comboStyle);
    m_categoryCombo->setCursor(Qt::PointingHandCursor);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Поиск по названию...");
    m_searchEdit->setFixedSize(300, 40); // Высота 40
    m_searchEdit->setStyleSheet("QLineEdit { background-color: #FFFFFF; border: 1px solid #E0E0E0; border-radius: 20px; padding: 0px 20px; font-family: 'Century Gothic'; font-size: 14px; color: #333333; }");

    // Встроенный серый крестик
    m_searchEdit->setClearButtonEnabled(true);

    m_searchCompleter = new QCompleter(this);
    m_searchCompleter->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    m_searchCompleter->setMaxVisibleItems(3);

    m_searchModel = new QStandardItemModel(0, 1, m_searchCompleter);
    m_searchCompleter->setModel(m_searchModel);

    QAbstractItemView *popup = m_searchCompleter->popup();
    popup->setStyleSheet(
        "QListView { background-color: #FFFFFF; border: 1px solid #E0E0E0; border-radius: 12px; padding: 4px; font-family: 'Century Gothic'; font-size: 14px; color: #333333; outline: none; } "
        "QListView::item { padding: 8px 12px; border-radius: 6px; } "
        "QListView::item:hover, QListView::item:selected { background-color: #F5F5F5; color: #111111; } "
        "QListView::item:disabled { color: #AAAAAA; font-style: italic; background-color: transparent; } "
        "QScrollBar:vertical { border: none; background: #F9F9F9; width: 8px; border-radius: 4px; margin: 4px 2px 4px 2px; } "
        "QScrollBar::handle:vertical { background: #D3D3D3; min-height: 20px; border-radius: 4px; } "
        "QScrollBar::handle:vertical:hover { background: #BDBDBD; } "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; } "
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
        );
    popup->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_searchEdit->setCompleter(m_searchCompleter);

    m_sortCombo = new QComboBox();
    m_sortCombo->addItems({"Сортировать по: Имени (А-Я)", "Сортировать по: Имени (Я-А)"});
    m_sortCombo->setFixedSize(290, 40); // Высота 40
    m_sortCombo->setStyleSheet(comboStyle);
    m_sortCombo->setCursor(Qt::PointingHandCursor);

    layout->addWidget(m_categoryCombo);
    layout->addWidget(m_searchEdit);
    layout->addWidget(m_sortCombo);
    layout->addStretch();

    // Подключения сигналов
    connect(m_categoryCombo, &QComboBox::currentTextChanged, this, &CatalogTopBar::updateSearchCompleter);
    connect(m_sortCombo, &QComboBox::currentTextChanged, this, &CatalogTopBar::filtersChanged);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &CatalogTopBar::filtersChanged);

    connect(m_searchEdit, &QLineEdit::textEdited, this, &CatalogTopBar::onSearchTextChanged);

    connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (text.isEmpty()) {
            onSearchTextChanged("");
            emit filtersChanged();
        }
    });

    connect(m_searchCompleter, QOverload<const QString &>::of(&QCompleter::activated), this, [this](const QString &text) {
        if (text == "Ничего не найдено") {
            m_searchEdit->clear();
        } else {
            m_searchEdit->setText(text);
            m_searchEdit->clearFocus();
            emit filtersChanged();
        }
    });
}

void CatalogTopBar::updateSearchCompleter()
{
    m_baseSuggestions.clear();
    QString category = m_categoryCombo->currentText();

    QString queryStr = "SELECT DISTINCT I.name FROM Items I WHERE I.name NOT LIKE '%(Перед)%' ";

    if (category == "Основные цветы") queryStr += "AND I.item_id IN (SELECT item_id FROM Flower_details WHERE subtype = 'main') ";
    else if (category == "Стаффаж") queryStr += "AND I.item_id IN (SELECT item_id FROM Flower_details WHERE subtype = 'filler') ";
    else if (category == "Упаковка") queryStr += "AND I.item_type = 'packaging' ";
    else if (category == "Аксессуары") queryStr += "AND I.item_type = 'accessory' ";

    QSqlQuery q;
    if (q.exec(queryStr)) {
        while (q.next()) m_baseSuggestions << q.value(0).toString();
    }

    onSearchTextChanged(m_searchEdit->text());
    emit filtersChanged();
}

void CatalogTopBar::onSearchTextChanged(const QString &text)
{
    if (!m_searchModel) return;
    m_searchModel->setRowCount(0);
    bool found = false;

    for (const QString &itemText : m_baseSuggestions) {
        if (text.isEmpty() || itemText.contains(text, Qt::CaseInsensitive)) {
            m_searchModel->appendRow(new QStandardItem(itemText));
            found = true;
        }
    }

    if (!found && !text.isEmpty()) {
        QStandardItem *item = new QStandardItem("Ничего не найдено");
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        m_searchModel->appendRow(item);
    }
}