#include "catalogminicard.h"

#include <QSqlQuery>
#include <QGuiApplication>
#include <QScreen>
#include <QIcon>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>

CatalogMiniCard::CatalogMiniCard(int id, const QString &name, const QString &imagePath, bool variationMode, QWidget *parent)
    : QWidget(parent),
    m_id(id),
    m_imagePath(imagePath),
    m_isVariationMode(variationMode),
    m_isSelected(false),
    m_isHovered(false)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(160, m_isVariationMode ? 220 : 250);
    setCursor(m_isVariationMode ? Qt::OpenHandCursor : Qt::PointingHandCursor);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 15, 10, 15);
    mainLayout->setSpacing(0);

    m_imgLabel = new QLabel(this);
    m_imgLabel->setFixedSize(120, 120);
    m_imgLabel->setAlignment(Qt::AlignCenter);

    if (!m_imagePath.isEmpty()) {
        QPixmap pix(m_imagePath);
        if (!pix.isNull()) {
            qreal dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
            QPixmap highResPix = pix.scaled(m_imgLabel->size() * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            highResPix.setDevicePixelRatio(dpr);
            m_imgLabel->setPixmap(highResPix);
        }
    }

    m_nameLabel = new QLabel(name, this);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setMinimumHeight(55);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; color: #333; background: transparent;");

    mainLayout->addWidget(m_imgLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(m_nameLabel, 0, Qt::AlignHCenter);
    mainLayout->addStretch(1);

    if (!m_isVariationMode) {
        m_dotsLayout = new QHBoxLayout();
        m_dotsLayout->setSpacing(6);
        m_dotsLayout->setAlignment(Qt::AlignCenter);
        mainLayout->addLayout(m_dotsLayout);
        loadColors();
    }

    updateStyle();
}

void CatalogMiniCard::setDoubleImage(const QString &imgBack) {
    if (!m_isVariationMode || imgBack.isEmpty() || m_imagePath.isEmpty()) return;

    QPixmap backPix(imgBack);
    QPixmap frontPix(m_imagePath);

    if (backPix.isNull() || frontPix.isNull()) return;

    int size = 120;
    qreal dpr = QGuiApplication::primaryScreen()->devicePixelRatio();

    QPixmap combined(size * dpr, size * dpr);
    combined.fill(Qt::transparent);
    combined.setDevicePixelRatio(dpr);

    QPainter p(&combined);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    QPixmap scaledBack = backPix.scaled(size * dpr, size * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaledBack.setDevicePixelRatio(dpr);
    p.drawPixmap((size - scaledBack.width() / dpr) / 2.0, (size - scaledBack.height() / dpr) / 2.0, scaledBack);

    QPixmap scaledFront = frontPix.scaled(size * dpr, size * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaledFront.setDevicePixelRatio(dpr);
    p.drawPixmap((size - scaledFront.width() / dpr) / 2.0, (size - scaledFront.height() / dpr) / 2.0, scaledFront);

    p.end();

    m_imgLabel->setPixmap(combined);
}

void CatalogMiniCard::loadColors() {
    if (m_isVariationMode) return;

    QSqlQuery q;
    q.prepare("SELECT C.color_hex FROM Item_variation V JOIN Colors C ON V.color_id = C.color_id WHERE V.item_id = :id");
    q.bindValue(":id", m_id);
    if (q.exec()) {
        while (q.next()) {
            QLabel *dot = new QLabel(this);
            dot->setFixedSize(12, 12);
            dot->setStyleSheet(QString(
                                   "background-color: %1; border-radius: 6px; border: 1px solid rgba(0,0,0,0.15);"
                                   ).arg(q.value(0).toString()));
            m_dotsLayout->addWidget(dot);
        }
    }
}

void CatalogMiniCard::updateStyle() {
    QString style;
    if (m_isSelected && !m_isVariationMode) {
        style = "CatalogMiniCard { background-color: #FFF0F5; border: 2px solid #8E3A4B; border-radius: 15px; }";
    } else if (m_isHovered) {
        style = "CatalogMiniCard { background-color: #FAFAFA; border: 2px solid #F8C3CD; border-radius: 15px; }";
    } else {
        style = "CatalogMiniCard { background-color: #FFFFFF; border: 1px solid #CCCCCC; border-radius: 15px; }";
    }
    setStyleSheet(style);
}

void CatalogMiniCard::setSelected(bool selected) {
    if (m_isSelected == selected) return;
    m_isSelected = selected;
    updateStyle();
}

void CatalogMiniCard::enterEvent(QEnterEvent *event) {
    m_isHovered = true;
    updateStyle();
    QWidget::enterEvent(event);
}

void CatalogMiniCard::leaveEvent(QEvent *event) {
    m_isHovered = false;
    updateStyle();
    QWidget::leaveEvent(event);
}

void CatalogMiniCard::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit itemClicked(m_id, m_imagePath);
        emit clicked(m_id);
    }
    QWidget::mousePressEvent(event);
}