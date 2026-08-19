#include "bouquetdetailsdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>

BouquetDetailsDialog::BouquetDetailsDialog(int bouquetId, QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    // Сделали окно более компактным (было 700х450)
    setFixedSize(600, 380);

    setupUI(bouquetId);
}

void BouquetDetailsDialog::setupUI(int bid) {
    QWidget *container = new QWidget(this);
    container->setGeometry(0, 0, 600, 380);
    container->setStyleSheet("background: white; border-radius: 25px;");

    QHBoxLayout *mainLayout = new QHBoxLayout(container);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    mainLayout->setSpacing(25);

    // --- 1. ЛЕВО: Изображение ---
    QLabel *imgLabel = new QLabel();
    imgLabel->setFixedSize(240, 330);
    imgLabel->setStyleSheet("background: #F9F9F9; border-radius: 15px;");
    imgLabel->setAlignment(Qt::AlignCenter);

    // Достаем название, описание и картинку напрямую из БД
    QSqlQuery qMain;
    qMain.prepare("SELECT bouquet_name, description, image_data FROM SavedBouquets WHERE bouquet_id = :bid");
    qMain.bindValue(":bid", bid);

    QString bName = "Букет";
    QString bDesc = "Описание отсутствует.";

    if(qMain.exec() && qMain.next()) {
        bName = qMain.value(0).toString();

        // Если описание есть и оно не пустое (NULL), берем его
        if (!qMain.value(1).isNull() && !qMain.value(1).toString().trimmed().isEmpty()) {
            bDesc = qMain.value(1).toString();
        }

        QPixmap pix;
        pix.loadFromData(qMain.value(2).toByteArray());
        qreal dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
        QPixmap highRes = pix.scaled(imgLabel->size() * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        highRes.setDevicePixelRatio(dpr);
        imgLabel->setPixmap(highRes);
    }

    // --- 2. ПРАВО: Текст ---
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(5);

    QHBoxLayout *header = new QHBoxLayout();
    QLabel *nameLabel = new QLabel(bName);
    nameLabel->setStyleSheet("font-size: 20px; font-weight: bold; font-family: 'Century Gothic'; color: #333;");

    QPushButton *btnClose = new QPushButton("×");
    btnClose->setFixedSize(28, 28);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet("QPushButton { border: 1px solid #EEE; border-radius: 14px; font-size: 16px; color: #555; background: white; margin: 0; padding: 0; } QPushButton:hover { background: #F8C3CD; color: white; border: none; }");
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);

    header->addWidget(nameLabel);
    header->addStretch();
    header->addWidget(btnClose);

    QLabel *subtitleLabel = new QLabel("Сохраненный букет");
    subtitleLabel->setStyleSheet("color: #AAA; font-size: 11px; font-family: 'Century Gothic';");

    QLabel *harmonyTitle = new QLabel("АНАЛИЗ КОМПОЗИЦИИ");
    harmonyTitle->setStyleSheet("color: #CCC; font-size: 10px; font-weight: bold; margin-top: 10px; font-family: 'Century Gothic';");

    // Выводим оригинальное описание из базы данных
    QLabel *harmonyDesc = new QLabel(bDesc);
    harmonyDesc->setStyleSheet("border: 1px solid #EEE; border-radius: 10px; padding: 12px; color: #555; font-family: 'Century Gothic'; font-size: 12px;");
    harmonyDesc->setWordWrap(true);
    harmonyDesc->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QLabel *compTitle = new QLabel("СОСТАВ БУКЕТА");
    compTitle->setStyleSheet("color: #CCC; font-size: 10px; font-weight: bold; margin-top: 10px; font-family: 'Century Gothic';");

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("background: transparent;");

    QWidget *listWidget = new QWidget();
    listWidget->setStyleSheet("background: transparent;");
    QVBoxLayout *listLayout = new QVBoxLayout(listWidget);

    // Убираем лишние отступы между цветами
    listLayout->setSpacing(0);
    listLayout->setContentsMargins(0, 0, 10, 0);

    // --- 3. ЗАГРУЗКА СОСТАВА (Только список, без расчета) ---
    QSqlQuery qComp;
    qComp.prepare("SELECT i.name, c.color_name, bc.quantity "
                  "FROM BouquetComponent bc "
                  "JOIN Item_variation v ON bc.variation_id = v.variation_id "
                  "JOIN Items i ON v.item_id = i.item_id "
                  "LEFT JOIN Colors c ON v.color_id = c.color_id "
                  "WHERE bc.bouquet_id = :bid");
    qComp.bindValue(":bid", bid);

    if(qComp.exec()) {
        while(qComp.next()) {
            QString itemName = qComp.value(0).toString();
            QString itemColor = qComp.value(1).isNull() ? "" : qComp.value(1).toString();
            QString itemQty = qComp.value(2).toString();

            QHBoxLayout *row = new QHBoxLayout();
            // Делаем поля строки минимальными
            row->setContentsMargins(0, 3, 0, 3);

            QLabel *n = new QLabel(QString("<span style='font-family: Century Gothic; font-size: 13px; color: #333;'><b>%1</b></span><br><span style='color:#AAA; font-family: Century Gothic; font-size: 11px;'>%2</span>")
                                       .arg(itemName).arg(itemColor));

            QLabel *qty = new QLabel(itemQty + " шт.");
            qty->setStyleSheet("font-weight: bold; color: #999; font-family: 'Century Gothic'; font-size: 13px;");

            row->addWidget(n);
            row->addStretch();
            row->addWidget(qty);
            listLayout->addLayout(row);
        }
    }

    // Добавляем пустую пружину в конец, чтобы элементы состава прижимались к верху, а не растягивались на всё окно
    listLayout->addStretch();

    scrollArea->setWidget(listWidget);

    infoLayout->addLayout(header);
    infoLayout->addWidget(subtitleLabel);
    infoLayout->addWidget(harmonyTitle);
    infoLayout->addWidget(harmonyDesc);
    infoLayout->addWidget(compTitle);
    infoLayout->addWidget(scrollArea, 1);

    mainLayout->addWidget(imgLabel);
    mainLayout->addLayout(infoLayout);
}