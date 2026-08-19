#include "palettepage.h"
#include "errordialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QPainter>
#include <QFrame>
#include <QDebug>
#include <QFont>
#include <QColorDialog> // Подключили для выбора цвета

PalettePage::PalettePage(QWidget *parent) : QWidget(parent) {
    setStyleSheet("background-color: #FFFFFF;");
    setupUI();
    loadColors();
}

void PalettePage::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ==========================================
    // ЛЕВАЯ ПАНЕЛЬ
    // ==========================================
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setFixedWidth(300);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(25, 25, 15, 25);
    leftLayout->setSpacing(15);

    QLabel *title = new QLabel("ПАЛИТРА ЦВЕТОВ", leftPanel);
    title->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; color: #333; letter-spacing: 1px; border: none; background: transparent;");
    leftLayout->addWidget(title);

    m_searchEdit = new QLineEdit(leftPanel);
    m_searchEdit->setPlaceholderText("Поиск оттенка...");
    m_searchEdit->setFixedHeight(38);
    m_searchEdit->setStyleSheet("QLineEdit { border: 1px solid #EAEAEA; border-radius: 19px; padding-left: 15px; font-family: 'Century Gothic'; font-size: 13px; background: #FAFAFA; }");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &PalettePage::onSearchTextChanged);
    leftLayout->addWidget(m_searchEdit);

    m_colorsList = new QListWidget(leftPanel);
    m_colorsList->setFrameShape(QFrame::NoFrame);
    m_colorsList->setIconSize(QSize(24, 24));
    m_colorsList->setFocusPolicy(Qt::NoFocus);
    m_colorsList->setStyleSheet(
        "QListWidget { background: transparent; outline: none; border: none; } "
        "QListWidget::item { height: 50px; border-radius: 12px; color: #555555; padding-left: 10px; font-family: 'Century Gothic'; font-size: 15px; border: none; outline: none; } "
        "QListWidget::item:hover { background-color: #FFF0F3; color: #333333; } "
        "QListWidget::item:selected { background-color: #FFF0F3; color: #000; font-weight: bold; border: none; outline: none; } "
        "QScrollBar:vertical { border: none; background: transparent; width: 4px; } "
        "QScrollBar::handle:vertical { background: #DCDCDC; border-radius: 2px; } "
        "QScrollBar::handle:vertical:hover { background: #D86B7A; }"
        );
    connect(m_colorsList, &QListWidget::itemSelectionChanged, this, &PalettePage::onColorSelected);
    leftLayout->addWidget(m_colorsList);

    m_btnAddColor = new QPushButton("+ Добавить цвет", leftPanel);
    m_btnAddColor->setFixedHeight(40);
    m_btnAddColor->setCursor(Qt::PointingHandCursor);
    m_btnAddColor->setStyleSheet("QPushButton { border: 1.5px solid #EAEAEA; border-radius: 20px; font-family: 'Century Gothic'; font-size: 12px; font-weight: bold; color: #555; background: white; } QPushButton:hover { background: #F9F9F9; border-color: #D86B7A; }");
    connect(m_btnAddColor, &QPushButton::clicked, this, &PalettePage::onAddColorClicked);
    leftLayout->addWidget(m_btnAddColor);

    QFrame *vSeparator = new QFrame(this);
    vSeparator->setFixedWidth(1);
    vSeparator->setStyleSheet("background-color: #F2F2F2; border: none;");

    // ==========================================
    // ПРАВАЯ ПАНЕЛЬ (СТЕК)
    // ==========================================
    m_rightStack = new QStackedWidget(this);
    m_rightStack->setStyleSheet("QStackedWidget { background: transparent; border: none; outline: none; }");

    // СТРАНИЦА 0: ЗАГЛУШКА
    QWidget *emptyPage = new QWidget();
    emptyPage->setStyleSheet("background: transparent; border: none; outline: none;");
    QVBoxLayout *emptyLayout = new QVBoxLayout(emptyPage);
    QLabel *emptyLbl = new QLabel("Выберите цвет из списка или добавьте новый", emptyPage);
    emptyLbl->setAlignment(Qt::AlignCenter);
    emptyLbl->setStyleSheet("font-family: 'Century Gothic'; font-size: 15px; color: #AAA; background: transparent; border: none;");
    emptyLayout->addWidget(emptyLbl);
    m_rightStack->addWidget(emptyPage);

    // СТРАНИЦА 1: КОНТЕНТ
    QWidget *contentPage = new QWidget();
    contentPage->setStyleSheet("background: transparent; border: none; outline: none;");
    QVBoxLayout *rightLayout = new QVBoxLayout(contentPage);
    rightLayout->setContentsMargins(30, 25, 30, 25);
    rightLayout->setSpacing(25);

    QFrame *infoCard = new QFrame();
    infoCard->setFixedHeight(160);
    infoCard->setStyleSheet("QFrame { background: white; border: 1px solid #F2F2F2; border-radius: 25px; outline: none; }");
    QHBoxLayout *cardH = new QHBoxLayout(infoCard);
    cardH->setContentsMargins(30, 0, 30, 0);
    cardH->setSpacing(25);

    m_bigColorPreview = new QLabel();
    m_bigColorPreview->setFixedSize(100, 100);
    m_bigColorPreview->setStyleSheet("border-radius: 50px; background: #FAFAFA; border: 1px solid #EEE;");
    cardH->addWidget(m_bigColorPreview);

    // СТЕК ТЕКСТА (ЖЕСТКО УБИРАЕМ РАМКИ)
    m_textStack = new QStackedWidget();
    m_textStack->setStyleSheet("QStackedWidget { background: transparent; border: none; outline: none; }");

    // Режим просмотра текста
    QWidget *viewPage = new QWidget();
    viewPage->setStyleSheet("background: transparent; border: none; outline: none;");
    QVBoxLayout *viewL = new QVBoxLayout(viewPage);
    viewL->setContentsMargins(0, 0, 0, 0);
    viewL->setSpacing(8);
    viewL->setAlignment(Qt::AlignVCenter);

    m_colorNameLabel = new QLabel("Выберите цвет");
    m_colorNameLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 28px; font-weight: bold; color: #333; border: none; background: transparent;");

    QHBoxLayout *metaH = new QHBoxLayout();
    metaH->setContentsMargins(0, 0, 0, 0);
    m_colorHexLabel = new QLabel("HEX:  #------");
    m_colorHexLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; color: #333; background: #F5F5F5; padding: 5px 15px; border-radius: 12px; border: none;");

    m_colorIdLabel = new QLabel("ID: #0");
    m_colorIdLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 12px; color: #CCC; border: none; font-weight: bold; background: transparent;");

    metaH->addWidget(m_colorHexLabel);
    metaH->addWidget(m_colorIdLabel);
    metaH->addStretch();

    viewL->addWidget(m_colorNameLabel);
    viewL->addLayout(metaH);
    m_textStack->addWidget(viewPage);

    // Режим полей ввода
    QWidget *editPage = new QWidget();
    editPage->setStyleSheet("background: transparent; border: none; outline: none;");
    QVBoxLayout *editL = new QVBoxLayout(editPage);
    editL->setContentsMargins(0, 0, 0, 0);
    editL->setSpacing(10);
    editL->setAlignment(Qt::AlignVCenter);

    m_nameEditField = new QLineEdit();
    m_nameEditField->setPlaceholderText("Название цвета...");
    m_nameEditField->setStyleSheet("QLineEdit { border: 1px solid #DDD; border-radius: 8px; padding: 8px; font-family: 'Century Gothic'; font-size: 16px; font-weight: bold; background: white; }");

    // Блок ввода HEX с кнопкой палитры
    QHBoxLayout *hexLayout = new QHBoxLayout();
    hexLayout->setContentsMargins(0, 0, 0, 0);
    hexLayout->setSpacing(10);

    m_hexEditField = new QLineEdit();
    m_hexEditField->setPlaceholderText("#HEX код...");
    m_hexEditField->setStyleSheet("QLineEdit { border: 1px solid #DDD; border-radius: 8px; padding: 8px; font-family: 'Century Gothic'; font-size: 13px; color: #D86B7A; background: white; }");
    connect(m_hexEditField, &QLineEdit::textEdited, this, &PalettePage::onHexEdited);

    m_btnPickColor = new QPushButton("🎨");
    m_btnPickColor->setFixedSize(36, 36);
    m_btnPickColor->setCursor(Qt::PointingHandCursor);
    m_btnPickColor->setStyleSheet("QPushButton { border: 1px solid #DDD; border-radius: 8px; background: white; font-size: 16px; } QPushButton:hover { background: #F0F0F0; border-color: #D86B7A; }");
    connect(m_btnPickColor, &QPushButton::clicked, this, &PalettePage::onPickColorClicked);

    hexLayout->addWidget(m_hexEditField);
    hexLayout->addWidget(m_btnPickColor);

    editL->addWidget(m_nameEditField);
    editL->addLayout(hexLayout);
    m_textStack->addWidget(editPage);

    cardH->addWidget(m_textStack, 1);

    m_btnEditColor = new QPushButton("Редактировать");
    m_btnEditColor->setFixedSize(130, 38);
    m_btnEditColor->setCursor(Qt::PointingHandCursor);
    m_btnEditColor->setStyleSheet("QPushButton { border: 1.5px solid #EAEAEA; border-radius: 19px; font-family: 'Century Gothic'; font-size: 12px; font-weight: bold; color: #444; background: white; } QPushButton:hover { background: #F9F9F9; border-color: #D86B7A; }");
    connect(m_btnEditColor, &QPushButton::clicked, this, &PalettePage::onEditColorClicked);
    cardH->addWidget(m_btnEditColor);

    rightLayout->addWidget(infoCard);

    m_usedInLabel = new QLabel("ИСПОЛЬЗУЕТСЯ В ТОВАРАХ");
    m_usedInLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; font-weight: bold; color: #AAA; letter-spacing: 2px; border: none; background: transparent;");
    rightLayout->addWidget(m_usedInLabel);

    // СКРОЛЛ ТОВАРОВ (ТОЛЬКО ВЕРТИКАЛЬНЫЙ)
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    // Отключаем горизонтальный скролл жестко
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setStyleSheet(
        "QScrollArea { background: transparent; border: none; } "
        "QScrollBar:vertical { border: none; background: transparent; width: 6px; margin: 0px; } "
        "QScrollBar::handle:vertical { background: #DCDCDC; border-radius: 3px; min-height: 30px; } "
        "QScrollBar::handle:vertical:hover { background: #D86B7A; } "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; } "
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        );

    QWidget *productsContainer = new QWidget();
    productsContainer->setStyleSheet("background: transparent; border: none;");
    m_productsGrid = new QGridLayout(productsContainer);
    m_productsGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_productsGrid->setSpacing(20);
    scroll->setWidget(productsContainer);
    rightLayout->addWidget(scroll, 1);

    m_rightStack->addWidget(contentPage);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(vSeparator);
    mainLayout->addWidget(m_rightStack, 1);

    m_rightStack->setCurrentIndex(0);
}

QIcon PalettePage::createColorIcon(const QString &hex, int size) {
    QPixmap pixmap(size * 2, size * 2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    QColor color(hex.isEmpty() ? "#F5F5F5" : hex);
    p.setBrush(color);
    p.setPen(QPen(QColor(0, 0, 0, 25), 1.2));
    p.drawEllipse(4, 4, (size * 2) - 8, (size * 2) - 8);
    pixmap.setDevicePixelRatio(2.0);
    return QIcon(pixmap);
}

void PalettePage::loadColors() {
    m_colorsList->blockSignals(true);
    m_colorsList->clear();
    QSqlQuery q("SELECT color_id, color_name, color_hex FROM Colors ORDER BY color_name");

    QFont gothicFont("Century Gothic", 11);

    while (q.next()) {
        QListWidgetItem *item = new QListWidgetItem(q.value(1).toString());
        item->setData(Qt::UserRole, q.value(0).toInt());
        item->setData(Qt::UserRole + 1, q.value(2).toString());
        item->setIcon(createColorIcon(q.value(2).toString(), 22));
        item->setFont(gothicFont);
        m_colorsList->addItem(item);
    }
    m_colorsList->blockSignals(false);
}

void PalettePage::onColorSelected() {
    QListWidgetItem *item = m_colorsList->currentItem();
    if (!item) {
        m_rightStack->setCurrentIndex(0);
        return;
    }

    m_rightStack->setCurrentIndex(1);
    m_isEditing = false;
    m_isAddingNew = false;
    m_textStack->setCurrentIndex(0);

    m_btnEditColor->setText("Редактировать");
    m_btnEditColor->setStyleSheet("QPushButton { border: 1.5px solid #EAEAEA; border-radius: 19px; font-family: 'Century Gothic'; font-size: 12px; font-weight: bold; color: #444; background: white; } QPushButton:hover { background: #F9F9F9; border-color: #D86B7A; }");

    int id = item->data(Qt::UserRole).toInt();
    QString name = item->text();
    QString hex = item->data(Qt::UserRole + 1).toString();

    m_colorNameLabel->setText(name);
    m_colorHexLabel->setText("HEX:  " + (hex.isEmpty() ? "#???" : hex.toUpper()));
    m_colorIdLabel->setText(QString("ID: #%1").arg(id));

    m_bigColorPreview->setStyleSheet(QString("border-radius: 50px; background-color: %1; border: 1px solid #EEE;").arg(hex.isEmpty() ? "#FAFAFA" : hex));

    loadUsedProducts(id);
}

void PalettePage::loadUsedProducts(int colorId) {
    QLayoutItem *child;
    while ((child = m_productsGrid->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    QSqlQuery q;
    q.prepare("SELECT DISTINCT i.name, (SELECT image FROM Item_variation WHERE item_id = i.item_id LIMIT 1) "
              "FROM Items i JOIN Item_variation iv ON i.item_id = iv.item_id WHERE iv.color_id = :id");
    q.bindValue(":id", colorId);

    int col = 0, row = 0;
    if (q.exec()) {
        while (q.next()) {
            m_productsGrid->addWidget(createProductCard(q.value(0).toString(), q.value(1).toString()), row, col);
            col++;
            // ТРОГО 3 В РЯД (Индексы 0, 1, 2)
            if (col > 2) {
                col = 0;
                row++;
            }
        }
    }
}

QWidget* PalettePage::createProductCard(const QString &name, const QString &imgPath) {
    QFrame *card = new QFrame();
    card->setFixedSize(160, 210); // Чуть увеличили, чтобы 3 в ряд смотрелись мощно
    card->setStyleSheet("QFrame { background: white; border: 1px solid #F0F0F0; border-radius: 20px; outline: none; } QFrame:hover { border-color: #F8C3CD; }");

    QVBoxLayout *l = new QVBoxLayout(card);
    l->setContentsMargins(15, 15, 15, 15);
    l->setSpacing(10);

    QLabel *img = new QLabel();
    img->setFixedSize(130, 130);
    img->setAlignment(Qt::AlignCenter);
    img->setStyleSheet("border: none; background: transparent; outline: none;");

    if (!imgPath.isEmpty()) {
        QPixmap original(imgPath);
        if (!original.isNull()) {
            int renderSize = 130 * 2; // Retina рендеринг
            QPixmap highRes = original.scaled(renderSize, renderSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            highRes.setDevicePixelRatio(2.0);
            img->setPixmap(highRes);
        }
    } else {
        img->setStyleSheet("background: #FAFAFA; border-radius: 12px; border: 1px solid #F0F0F0;");
    }

    QLabel *nm = new QLabel(name);
    nm->setAlignment(Qt::AlignCenter);
    nm->setWordWrap(true);
    nm->setStyleSheet("font-family: 'Century Gothic'; font-size: 12px; font-weight: bold; color: #333; border: none; background: transparent; outline: none;");

    l->addWidget(img);
    l->addWidget(nm, 0, Qt::AlignTop);
    return card;
}

void PalettePage::onSearchTextChanged(const QString &text) {
    for (int i = 0; i < m_colorsList->count(); ++i) {
        m_colorsList->item(i)->setHidden(!m_colorsList->item(i)->text().contains(text, Qt::CaseInsensitive));
    }
}

void PalettePage::onHexEdited(const QString &text) {
    if (text.startsWith("#") && (text.length() == 4 || text.length() == 7)) {
        m_bigColorPreview->setStyleSheet(QString("border-radius: 50px; background-color: %1; border: 1px solid #EEE;").arg(text));
    }
}

// ЛОГИКА ВЫЗОВА ЦВЕТОВОЙ ПАЛИТРЫ (ПРЕМИАЛЬНЫЙ ДИЗАЙН)
void PalettePage::onPickColorClicked() {
    QColor initialColor = QColor(m_hexEditField->text().isEmpty() ? "#FFFFFF" : m_hexEditField->text());

    QColorDialog dialog(initialColor, this);

    // 1. Отключаем страшное системное окно Windows (убираем верхнюю плашку с крестиком)
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog.setOption(QColorDialog::DontUseNativeDialog, true);

    // 2. Полностью переписываем стили всех внутренностей
    dialog.setStyleSheet(
        "QColorDialog { "
        "  background-color: #FFFFFF; "
        "  border: 2px solid #EAEAEA; " // Рамка, раз нет системной шапки
        "} "
        "QWidget { "
        "  font-family: 'Century Gothic'; "
        "  color: #333333; "
        "} "
        "QLabel { "
        "  font-size: 13px; "
        "} "
        "QPushButton { "
        "  background-color: #FFFFFF; "
        "  border: 1.5px solid #EAEAEA; "
        "  border-radius: 16px; " /* Сильно скругленные кнопки в стиле приложения */
        "  padding: 8px 20px; "
        "  font-size: 12px; "
        "  font-weight: bold; "
        "  color: #555555; "
        "  min-width: 80px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #FFF0F3; "
        "  border-color: #D86B7A; "
        "  color: #D86B7A; "
        "} "
        "QLineEdit, QSpinBox { "
        "  border: 1px solid #EAEAEA; "
        "  border-radius: 8px; "
        "  padding: 4px; "
        "  background: #FAFAFA; "
        "  font-size: 13px; "
        "  selection-background-color: #FFF0F3; "
        "  selection-color: black; "
        "} "
        "QLineEdit:focus, QSpinBox:focus { "
        "  border-color: #D86B7A; "
        "  background: #FFFFFF; "
        "}"
        );

    // Открываем диалог. Кнопки OK и Cancel теперь тоже стильные!
    if (dialog.exec() == QDialog::Accepted) {
        QString hex = dialog.currentColor().name().toUpper(); // Получаем красивый #HEX
        m_hexEditField->setText(hex);
        onHexEdited(hex); // Мгновенно обновляем наш большой кружок
    }
}

void PalettePage::onEditColorClicked() {
    if (!m_isEditing) {
        m_isEditing = true;
        m_textStack->setCurrentIndex(1);
        m_nameEditField->setText(m_colorNameLabel->text());
        m_hexEditField->setText(m_colorHexLabel->text().remove("HEX:  "));

        m_btnEditColor->setText("Сохранить");
        m_btnEditColor->setStyleSheet("QPushButton { border: none; border-radius: 19px; color: white; background: #D86B7A; font-family: 'Century Gothic'; font-weight: bold; }");
    } else {
        QString newName = m_nameEditField->text().trimmed();
        QString newHex = m_hexEditField->text().trimmed();

        if (newName.isEmpty() || !newHex.startsWith("#")) {
            QWidget *overlay = new QWidget(this->window());
            overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
            overlay->resize(this->window()->size());
            overlay->show();

            ErrorDialog err("Ошибка", "Введите название и HEX-код (напр. #FF0000)", this->window());
            err.move(this->window()->frameGeometry().center() - err.rect().center());
            err.exec();

            overlay->deleteLater();
            return;
        }

        QSqlQuery q;
        if (m_isAddingNew) {
            q.prepare("INSERT INTO Colors (color_name, color_hex) VALUES (:n, :h)");
        } else {
            QListWidgetItem *item = m_colorsList->currentItem();
            if (!item) return;
            int currentId = item->data(Qt::UserRole).toInt();
            q.prepare("UPDATE Colors SET color_name = :n, color_hex = :h WHERE color_id = :id");
            q.bindValue(":id", currentId);
        }

        q.bindValue(":n", newName);
        q.bindValue(":h", newHex);

        if (q.exec()) {
            m_isEditing = false;
            m_isAddingNew = false;
            m_textStack->setCurrentIndex(0);
            m_btnEditColor->setText("Редактировать");
            m_btnEditColor->setStyleSheet("QPushButton { border: 1.5px solid #EAEAEA; border-radius: 19px; font-family: 'Century Gothic'; font-size: 12px; font-weight: bold; color: #444; background: white; } QPushButton:hover { background: #F9F9F9; border-color: #D86B7A; }");

            loadColors();

            for(int i = 0; i < m_colorsList->count(); ++i) {
                if (m_colorsList->item(i)->text() == newName) {
                    m_colorsList->setCurrentRow(i);
                    break;
                }
            }
        }
    }
}

void PalettePage::onAddColorClicked() {
    m_colorsList->blockSignals(true);
    m_colorsList->clearSelection();
    m_colorsList->blockSignals(false);

    m_rightStack->setCurrentIndex(1);
    m_isEditing = true;
    m_isAddingNew = true;

    m_textStack->setCurrentIndex(1);
    m_nameEditField->clear();
    m_hexEditField->setText("#");
    m_colorIdLabel->setText("ID: #NEW");
    m_bigColorPreview->setStyleSheet("border-radius: 50px; background-color: #FAFAFA; border: 2px dashed #DDD;");

    m_productsGrid->parentWidget()->hide();
    m_usedInLabel->hide();

    m_btnEditColor->setText("Сохранить");
    m_btnEditColor->setStyleSheet("QPushButton { border: none; border-radius: 19px; color: white; background: #D86B7A; font-family: 'Century Gothic'; font-weight: bold; }");
}