#include "addvariationdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

AddVariationDialog::AddVariationDialog(int itemId, QWidget *parent)
    : QDialog(parent), m_itemId(itemId), m_saved(false) {

    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(350, 400);

    setupUI();
    loadColors();
}

void AddVariationDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // Убрали отступы под тень

    QFrame *bgFrame = new QFrame(this);
    // Убрали тень, добавили легкую обводку, чтобы окно не терялось
    bgFrame->setStyleSheet("QFrame { background-color: #FFFFFF; border-radius: 20px; border: 1px solid #DCDCDC; }");

    QVBoxLayout *frameLayout = new QVBoxLayout(bgFrame);
    frameLayout->setContentsMargins(30, 25, 30, 30);
    frameLayout->setSpacing(20);

    // --- ШАПКА ---
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("Новая вариация", bgFrame);
    titleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 20px; font-weight: bold; color: #000; border: none;");

    QPushButton *closeBtn = new QPushButton("×", bgFrame);
    closeBtn->setFixedSize(30, 30);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QPushButton { background-color: #F5F5F5; border-radius: 15px; font-family: 'Century Gothic'; font-size: 16px; color: #555; border: none; } QPushButton:hover { background-color: #E8E8E8; }");
    connect(closeBtn, &QPushButton::clicked, this, &AddVariationDialog::onCloseClicked);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);
    frameLayout->addLayout(headerLayout);

    // --- ВЫБОР ЦВЕТА ---
    QLabel *lblColor = new QLabel("ВЫБЕРИТЕ ЦВЕТ", bgFrame);
    lblColor->setStyleSheet("font-family: 'Century Gothic'; font-size: 10px; font-weight: bold; color: #888; letter-spacing: 1px; border: none;");

    m_colorCombo = new QComboBox(bgFrame);
    m_colorCombo->setFixedHeight(40);
    m_colorCombo->setStyleSheet("QComboBox { border: 1px solid #BDBDBD; border-radius: 10px; padding: 0 15px; font-family: 'Century Gothic'; font-size: 13px; background: white; } QComboBox::drop-down { border: none; width: 30px; }");

    frameLayout->addWidget(lblColor);
    frameLayout->addWidget(m_colorCombo);

    // --- ИЗОБРАЖЕНИЕ ---
    QLabel *lblImg = new QLabel("ИЗОБРАЖЕНИЕ", bgFrame);
    lblImg->setStyleSheet("font-family: 'Century Gothic'; font-size: 10px; font-weight: bold; color: #888; letter-spacing: 1px; border: none;");

    m_btnBrowse = new QPushButton("Загрузить фото...", bgFrame);
    m_btnBrowse->setFixedHeight(40);
    m_btnBrowse->setCursor(Qt::PointingHandCursor);
    m_btnBrowse->setStyleSheet("QPushButton { border: 1px solid #BDBDBD; border-radius: 10px; text-align: left; padding-left: 15px; font-family: 'Century Gothic'; font-size: 13px; color: #555; background: white; } QPushButton:hover { background: #F9F9F9; }");
    connect(m_btnBrowse, &QPushButton::clicked, this, &AddVariationDialog::onBrowseImage);

    frameLayout->addWidget(lblImg);
    frameLayout->addWidget(m_btnBrowse);

    frameLayout->addStretch();

    // --- КНОПКА СОХРАНИТЬ ---
    m_btnSave = new QPushButton("Сохранить вариацию", bgFrame);
    m_btnSave->setFixedHeight(45);
    m_btnSave->setCursor(Qt::PointingHandCursor);
    m_btnSave->setStyleSheet("QPushButton { background-color: #D86B7A; color: white; border-radius: 12px; font-family: 'Century Gothic'; font-size: 14px; font-weight: bold; border: none; } QPushButton:hover { background-color: #C75A69; }");
    connect(m_btnSave, &QPushButton::clicked, this, &AddVariationDialog::onSaveClicked);
    frameLayout->addWidget(m_btnSave);

    mainLayout->addWidget(bgFrame);
}

void AddVariationDialog::loadColors() {
    QSqlQuery q("SELECT color_id, color_name FROM Colors");
    while (q.next()) {
        m_colorCombo->addItem(q.value(1).toString(), q.value(0).toInt());
    }
}

// ... твой код до onBrowseImage() ...

void AddVariationDialog::onBrowseImage() {
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите изображение", "", "Images (*.png *.jpg *.jpeg)");
    if (!filePath.isEmpty()) {
        m_selectedImagePath = filePath;
        m_btnBrowse->setText(QFileInfo(filePath).fileName());
    }
}

// НОВЫЕ ГЕТТЕРЫ
int AddVariationDialog::getColorId() const {
    return m_colorCombo->currentData().toInt();
}

QString AddVariationDialog::getImagePath() const {
    return m_selectedImagePath;
}

void AddVariationDialog::onSaveClicked() {
    // ЕСЛИ ЦВЕТОК НОВЫЙ (ID == -1), ПРОСТО ОТДАЕМ ДАННЫЕ В ПАМЯТЬ
    if (m_itemId == -1) {
        m_saved = true;
        accept();
        return;
    }

    // ЕСЛИ ЦВЕТОК УЖЕ СУЩЕСТВУЕТ, СОХРАНЯЕМ В БД КАК РАНЬШЕ
    int colorId = m_colorCombo->currentData().toInt();
    QSqlQuery q;
    q.prepare("INSERT INTO Item_variation (item_id, color_id, image) VALUES (:iid, :cid, :img)");
    q.bindValue(":iid", m_itemId);
    q.bindValue(":cid", colorId);
    q.bindValue(":img", m_selectedImagePath);

    if (q.exec()) {
        m_saved = true;
        accept();
    } else {
        qDebug() << "Ошибка добавления вариации:" << q.lastError().text();
    }
}

void AddVariationDialog::onCloseClicked() {
    reject();
}