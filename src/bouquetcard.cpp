#include "bouquetcard.h"
#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>
#include <QVBoxLayout>
#include <QIcon> // <--- ОБЯЗАТЕЛЬНО ДЛЯ ИКОНОК

BouquetCard::BouquetCard(int id, const QString &name, const QString &date, const QByteArray &imageData, QWidget *parent)
    : QWidget(parent), m_id(id), m_isHovered(false)
{
    // Карточки стали чуть больше
    setFixedSize(190, 290);
    setCursor(Qt::PointingHandCursor);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 15, 10, 15);
    layout->setSpacing(5);

    QLabel *imgLabel = new QLabel(this);
    imgLabel->setFixedSize(150, 150);
    imgLabel->setAlignment(Qt::AlignCenter);

    if (!imageData.isEmpty()) {
        QPixmap pix;
        pix.loadFromData(imageData);
        qreal dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
        QPixmap highRes = pix.scaled(imgLabel->size() * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        highRes.setDevicePixelRatio(dpr);
        imgLabel->setPixmap(highRes);
    }

    QLabel *nameLabel = new QLabel(name, this);
    nameLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 14px; font-weight: bold; color: #333;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);

    QLabel *dateLabel = new QLabel(date, this);
    dateLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 11px; color: #AAA;");
    dateLabel->setAlignment(Qt::AlignCenter);

    QPushButton *btnDetails = new QPushButton("Подробнее", this);
    btnDetails->setFixedHeight(34);
    btnDetails->setCursor(Qt::PointingHandCursor);
    btnDetails->setStyleSheet(
        "QPushButton { background: white; border: 1px solid #DDD; border-radius: 12px; font-family: 'Century Gothic'; color: #666; font-size: 12px; } "
        "QPushButton:hover { background: #FDFDFD; border-color: #F8C3CD; color: #333; }"
        );

    // --- ДОБАВЛЯЕМ КНОПКУ УДАЛЕНИЯ В ПРАВЫЙ ВЕРХНИЙ УГОЛ ---
    QPushButton *btnDelete = new QPushButton(this);
    btnDelete->setFixedSize(30, 30);
    // Отступаем от правого края (Ширина карточки 190 - ширина кнопки 30 - отступ 10)
    btnDelete->move(190 - 30 - 10, 10);
    btnDelete->setCursor(Qt::PointingHandCursor);
    btnDelete->setStyleSheet(
        "QPushButton { background-color: rgba(255, 255, 255, 200); border-radius: 15px; border: none; } "
        "QPushButton:hover { background-color: #FFF0F3; }"
        );
    // ВСТАВЬ СВОЙ ПУТЬ К ИКОНКЕ ТУТ:
    btnDelete->setIcon(QIcon(":/sources/icons/free-icon-trash-can-542673.svg"));
    btnDelete->setIconSize(QSize(18, 18));

    // Подключаем кнопки
    connect(btnDetails, &QPushButton::clicked, this, [this]() {
        emit detailsRequested(m_id);
    });
    connect(btnDelete, &QPushButton::clicked, this, [this]() {
        emit deleteRequested(m_id); // Отправляем сигнал на удаление
    });

    layout->addWidget(imgLabel, 0, Qt::AlignHCenter);
    layout->addWidget(nameLabel);
    layout->addWidget(dateLabel);
    layout->addStretch();
    layout->addWidget(btnDetails);

    updateStyle();
}

void BouquetCard::updateStyle() {
    setStyleSheet(QString(
                      "BouquetCard { background-color: white; border: 1px solid %1; border-radius: 15px; }"
                      ).arg(m_isHovered ? "#F8C3CD" : "#DDDDDD"));
}

void BouquetCard::enterEvent(QEnterEvent *) { m_isHovered = true; updateStyle(); }
void BouquetCard::leaveEvent(QEvent *) { m_isHovered = false; updateStyle(); }