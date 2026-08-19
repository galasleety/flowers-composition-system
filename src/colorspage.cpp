#include "colorspage.h"
#include "errordialog.h"
#include "confirmdialog.h" // Предполагается, что он у тебя есть
#include <QSqlQuery>
#include <QSqlError>
#include <QFrame>
#include <QScrollArea>
#include <QDebug>

ColorsPage::ColorsPage(QWidget *parent) : QWidget(parent), m_isEditing(false), m_isAddingNew(false) {
    setStyleSheet("background-color: #FFFFFF;");
    setupUI();
    loadColors();
}

void ColorsPage::setupUI() {
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

    QLabel *titleLabel = new QLabel("ПАЛИТРА ЦВЕТОВ", leftPanel);
    titleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; color: #333;");
    leftLayout->addWidget(titleLabel);

    m_searchEdit = new QLineEdit(leftPanel);
    m_searchEdit->setPlaceholderText("Поиск оттенка...");
    m_searchEdit->setFixedHeight(38);
    m_searchEdit->setStyleSheet("QLineEdit { border: 1px solid #D86B7A; border-radius: 19px; padding-left: 15px; font-family: 'Century Gothic'; }");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ColorsPage::onSearchTextChanged);
    leftLayout->addWidget(m_searchEdit);

    m_colorsList = new QListWidget(leftPanel);
    m_colorsList->setFrameShape(QFrame::NoFrame);
    m_colorsList->setFocusPolicy(Qt::NoFocus);
    m_colorsList->setStyleSheet(
        "QListWidget { background: transparent; outline: none; border: none; } "
        "QListWidget::item { height: 50px; border-radius: 12px; color: #555555; padding-left: 15px; font-family: 'Century Gothic'; font-size: 15px; } "
        "QListWidget::item:hover { background-color: #FFF0F3; color: #333333; } "
        "QListWidget::item:selected { background-color: #FFF0F3; color: #000000; font-weight: bold; border: none; outline: none; } "
        "QScrollBar:vertical { border: none; background: transparent; width: 4px; margin: 0px; } "
        "QScrollBar::handle:vertical { background: #DCDCDC; border-radius: 2px; min-height: 30px; } "
        "QScrollBar::handle:vertical:hover { background: #D86B7A; } "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; } "
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        );
    connect(m_colorsList, &QListWidget::itemSelectionChanged, this, &ColorsPage::onColorSelected);
    leftLayout->addWidget(m_colorsList);

    m_btnAddColor = new QPushButton("+ Добавить цвет", leftPanel);
    m_btnAddColor->setFixedHeight(38);
    m_btnAddColor->setCursor(Qt::PointingHandCursor);
    m_btnAddColor->setStyleSheet("QPushButton { border: 1px solid #DCDCDC; border-radius: 12px; font-family: 'Century Gothic'; font-size: 12px; color: #555; } QPushButton:hover { background: #F5F5F5; }");
    connect(m_btnAddColor, &QPushButton::clicked, this, &ColorsPage::onAddColorClicked);
    leftLayout->addWidget(m_btnAddColor);

    QFrame *vSeparator = new QFrame(this);
    vSeparator->setFrameShape(QFrame::VLine);
    vSeparator->setStyleSheet("background-color: #E8E8E8; border: none; width: 1px; margin: 25px 0;");

    // ==========================================
    // ПРАВАЯ ЧАСТЬ
    // ==========================================
    m_rightPanel = new QWidget(this);
    QVBoxLayout *rightMainLayout = new QVBoxLayout(m_rightPanel);
    rightMainLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *mainScroll = new QScrollArea(m_rightPanel);
    mainScroll->setWidgetResizable(true);
    mainScroll->setFrameShape(QFrame::NoFrame);
    mainScroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    QWidget *scrollContent = new QWidget();
    scrollContent->setObjectName("mainScrollContent");
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout *rightLayout = new QVBoxLayout(scrollContent);
    rightLayout->setContentsMargins(30, 30, 30, 30);
    rightLayout->setAlignment(Qt::AlignTop);

    // КАРТОЧКА ЦВЕТА
    m_infoCard = new QFrame(scrollContent);
    m_infoCard->setFixedHeight(180);
    m_infoCard->setStyleSheet("QFrame { border: 2px solid #DCDCDC; border-radius: 15px; background-color: white; }");

    QHBoxLayout *cardLayout = new QHBoxLayout(m_infoCard);
    cardLayout->setContentsMargins(20, 20, 20, 20);
    cardLayout->setSpacing(25);

    // Квадратик, отображающий сам цвет
    m_colorPreviewBlock = new QLabel(m_infoCard);
    m_colorPreviewBlock->setFixedSize(110, 110);
    m_colorPreviewBlock->setStyleSheet("border: 1px solid #EAEAEA; border-radius: 12px; background-color: #FAFAFA;");

    m_textStack = new QStackedWidget(m_infoCard);
    m_textStack->setStyleSheet("background: transparent; border: none;");

    // --- Режим ПРОСМОТРА ---
    QWidget *viewWidget = new QWidget();
    QVBoxLayout *viewLayout = new QVBoxLayout(viewWidget);
    viewLayout->setContentsMargins(0, 0, 0, 0);
    viewLayout->setSpacing(5);

    m_colorNameLabel = new QLabel("Название цвета", viewWidget);
    m_colorNameLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 24px; font-weight: bold; color: #000; border: none;");

    m_colorHexLabel = new QLabel("#000000", viewWidget);
    m_colorHexLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 14px; color: #888; border: none;");

    m_colorIdLabel = new QLabel("ID: #0", viewWidget);
    m_colorIdLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; color: #AAA; font-weight: bold; border: none;");

    viewLayout->addWidget(m_colorNameLabel);
    viewLayout->addWidget(m_colorHexLabel);
    viewLayout->addStretch();
    viewLayout->addWidget(m_colorIdLabel);

    // --- Режим РЕДАКТИРОВАНИЯ ---
    QWidget *editWidget = new QWidget();
    QVBoxLayout *editLayout = new QVBoxLayout(editWidget);
    editLayout->setContentsMargins(0, 0, 0, 0);
    editLayout->setSpacing(10);

    m_nameEditField = new QLineEdit(editWidget);
    m_nameEditField->setPlaceholderText("Название (напр. Красный)");
    m_nameEditField->setStyleSheet("border: 1px solid #DCDCDC; border-radius: 6px; font-family: 'Century Gothic'; font-weight: bold; font-size: 16px; padding: 5px 10px;");

    m_hexEditField = new QLineEdit(editWidget);
    m_hexEditField->setPlaceholderText("HEX код (напр. #FF0000)");
    m_hexEditField->setStyleSheet("border: 1px solid #DCDCDC; border-radius: 6px; font-family: 'Century Gothic'; font-size: 14px; padding: 5px 10px;");

    editLayout->addWidget(m_nameEditField);
    editLayout->addWidget(m_hexEditField);
    editLayout->addStretch();

    m_textStack->addWidget(viewWidget);
    m_textStack->addWidget(editWidget);

    // Кнопки действий
    QVBoxLayout *btnContainer = new QVBoxLayout();
    btnContainer->setSpacing(10);
    btnContainer->setAlignment(Qt::AlignTop);

    m_btnEditColor = new QPushButton("Редактировать", m_infoCard);
    m_btnEditColor->setFixedSize(125, 36);
    m_btnEditColor->setCursor(Qt::PointingHandCursor);
    m_btnEditColor->setStyleSheet("QPushButton { border: 2px solid #DCDCDC; border-radius: 18px; font-family: 'Century Gothic'; font-size: 12px; color: #333; } QPushButton:hover { background: #F5F5F5; }");
    connect(m_btnEditColor, &QPushButton::clicked, this, &ColorsPage::onEditColorClicked);

    m_btnDeleteColor = new QPushButton("Удалить", m_infoCard);
    m_btnDeleteColor->setFixedSize(60, 36);
    m_btnDeleteColor->setCursor(Qt::PointingHandCursor);
    m_btnDeleteColor->setStyleSheet("QPushButton { border: none; color: #D86B7A; font-family: 'Century Gothic'; font-size: 12px; } QPushButton:hover { text-decoration: underline; }");
    connect(m_btnDeleteColor, &QPushButton::clicked, this, &ColorsPage::onDeleteColorClicked);

    btnContainer->addWidget(m_btnEditColor);
    btnContainer->addWidget(m_btnDeleteColor, 0, Qt::AlignCenter);

    cardLayout->addWidget(m_colorPreviewBlock);
    cardLayout->addWidget(m_textStack, 1);
    cardLayout->addLayout(btnContainer);

    rightLayout->addWidget(m_infoCard);
    rightLayout->addStretch(1);

    mainScroll->setWidget(scrollContent);
    rightMainLayout->addWidget(mainScroll);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(vSeparator);
    mainLayout->addWidget(m_rightPanel, 1);

    scrollContent->hide();
}

void ColorsPage::loadColors() {
    m_colorsList->clear();
    QSqlQuery q("SELECT color_id, color_name FROM Colors ORDER BY color_name");
    while (q.next()) {
        QListWidgetItem *item = new QListWidgetItem(q.value(1).toString());
        item->setData(Qt::UserRole, q.value(0).toInt());
        m_colorsList->addItem(item);
    }
}

void ColorsPage::onSearchTextChanged(const QString &text) {
    for (int i = 0; i < m_colorsList->count(); ++i) {
        QListWidgetItem *item = m_colorsList->item(i);
        item->setHidden(!item->text().contains(text, Qt::CaseInsensitive));
    }
}

void ColorsPage::onColorSelected() {
    QListWidgetItem *item = m_colorsList->currentItem();
    QWidget *scrollContent = m_rightPanel->findChild<QWidget*>("mainScrollContent");

    if (!item) {
        if (scrollContent) scrollContent->hide();
        return;
    }

    if (scrollContent) scrollContent->show();

    m_isEditing = false;
    m_isAddingNew = false;
    m_btnDeleteColor->show();

    m_textStack->setCurrentIndex(0);
    m_btnEditColor->setText("Редактировать");
    m_btnEditColor->setStyleSheet("QPushButton { border: 2px solid #DCDCDC; border-radius: 18px; font-family: 'Century Gothic'; font-size: 12px; color: #333; } QPushButton:hover { background: #F5F5F5; }");

    int id = item->data(Qt::UserRole).toInt();
    QSqlQuery q;

    // ПРИМЕЧАНИЕ: Если в таблице Colors есть столбец с hex-кодом, добавь его в запрос.
    // Пока предполагаю, что есть 'hex_code'. Если нет, можешь убрать его.
    q.prepare("SELECT color_name, hex_code FROM Colors WHERE color_id = :id");
    q.bindValue(":id", id);

    if (q.exec() && q.next()) {
        QString name = q.value(0).toString();
        QString hex = q.value(1).toString();

        m_colorNameLabel->setText(name);
        m_colorHexLabel->setText(hex.isEmpty() ? "HEX не указан" : hex);
        m_colorIdLabel->setText(QString("ID: #%1").arg(id));

        m_nameEditField->setText(name);
        m_hexEditField->setText(hex);

        if (!hex.isEmpty() && hex.startsWith("#")) {
            m_colorPreviewBlock->setStyleSheet(QString("border-radius: 12px; background-color: %1;").arg(hex));
        } else {
            m_colorPreviewBlock->setStyleSheet("border: 1px dashed #CCC; border-radius: 12px; background-color: #FAFAFA;");
        }
    }
}

void ColorsPage::onAddColorClicked() {
    m_colorsList->clearSelection();
    m_isEditing = true;
    m_isAddingNew = true;

    QWidget *scrollContent = m_rightPanel->findChild<QWidget*>("mainScrollContent");
    if (scrollContent) scrollContent->show();

    m_nameEditField->clear();
    m_hexEditField->clear();
    m_colorPreviewBlock->setStyleSheet("border: 1px dashed #CCC; border-radius: 12px; background-color: #FAFAFA;");

    m_textStack->setCurrentIndex(1);
    m_btnEditColor->setText("Сохранить");
    m_btnEditColor->setStyleSheet("QPushButton { border: none; border-radius: 18px; color: white; background: #D86B7A; font-family: 'Century Gothic'; font-weight: bold; }");
    m_btnDeleteColor->hide();
}

void ColorsPage::onEditColorClicked() {
    if (!m_isEditing) {
        m_isEditing = true;
        m_isAddingNew = false;
        m_textStack->setCurrentIndex(1);
        m_btnEditColor->setText("Сохранить");
        m_btnEditColor->setStyleSheet("QPushButton { border: none; border-radius: 18px; color: white; background: #D86B7A; font-family: 'Century Gothic'; font-weight: bold; }");
    } else {
        QString newName = m_nameEditField->text().trimmed();
        QString newHex = m_hexEditField->text().trimmed();

        if (newName.isEmpty()) {
            ErrorDialog err("Ошибка", "Введите название цвета!", this->window());
            err.exec();
            return;
        }

        QSqlQuery q;
        if (m_isAddingNew) {
            q.prepare("INSERT INTO Colors (color_name, hex_code) VALUES (:n, :h)");
            q.bindValue(":n", newName);
            q.bindValue(":h", newHex);
            if (q.exec()) {
                loadColors();
                // Найти и выделить добавленный
                int newId = q.lastInsertId().toInt();
                for(int i = 0; i < m_colorsList->count(); ++i) {
                    if(m_colorsList->item(i)->data(Qt::UserRole).toInt() == newId) {
                        m_colorsList->setCurrentRow(i);
                        break;
                    }
                }
            }
        } else {
            QListWidgetItem *item = m_colorsList->currentItem();
            if (!item) return;
            int id = item->data(Qt::UserRole).toInt();

            q.prepare("UPDATE Colors SET color_name = :n, hex_code = :h WHERE color_id = :id");
            q.bindValue(":n", newName);
            q.bindValue(":h", newHex);
            q.bindValue(":id", id);

            if (q.exec()) {
                item->setText(newName);
                onColorSelected();
            }
        }
    }
}

void ColorsPage::onDeleteColorClicked() {
    QListWidgetItem *item = m_colorsList->currentItem();
    if (!item) return;

    ConfirmDialog d("Удаление", "Удалить этот цвет?", this->window());
    if (d.exec() == QDialog::Accepted) {
        int id = item->data(Qt::UserRole).toInt();
        QSqlQuery q;
        q.prepare("DELETE FROM Colors WHERE color_id = :id");
        q.bindValue(":id", id);
        if (q.exec()) {
            loadColors();
            QWidget *scrollContent = m_rightPanel->findChild<QWidget*>("mainScrollContent");
            if (scrollContent) scrollContent->hide();
        }
    }
}