#include "notecard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

NoteCard::NoteCard(int id, const QString &title, const QString &content, QWidget *parent)
    : QWidget(parent), m_id(id), m_isHovered(false)
{
    setFixedSize(340, 160);
    setObjectName("noteCard");
    setAttribute(Qt::WA_StyledBackground, true);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 15));
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(25, 20, 15, 15);
    mainLayout->setSpacing(10);

    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 15px; font-weight: bold; color: #333; border: none;");

    QLabel *contentLabel = new QLabel(content, this);
    contentLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 13px; color: #888; border: none;");
    contentLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    contentLabel->setWordWrap(true);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();

    QString btnStyle = "QPushButton { background: transparent; border: none; } "
                       "QPushButton:hover { background: #FFF0F3; border-radius: 5px; }";

    // --- КНОПКА РЕДАКТИРОВАНИЯ ---
    QPushButton *btnEdit = new QPushButton(this);
    btnEdit->setFixedSize(30, 30);
    btnEdit->setCursor(Qt::PointingHandCursor);
    btnEdit->setStyleSheet(btnStyle);
    // ВСТАВЬ СЮДА СВОЙ ПУТЬ К ИКОНКЕ КАРАНДАША:
    btnEdit->setIcon(QIcon(":/sources/icons/free-icon-modify-6115910.svg"));
    btnEdit->setIconSize(QSize(18, 18)); // Размер самой картинки внутри кнопки

    // --- КНОПКА УДАЛЕНИЯ ---
    QPushButton *btnDelete = new QPushButton(this);
    btnDelete->setFixedSize(30, 30);
    btnDelete->setCursor(Qt::PointingHandCursor);
    btnDelete->setStyleSheet(btnStyle);
    // ВСТАВЬ СЮДА СВОЙ ПУТЬ К ИКОНКЕ КОРЗИНЫ:
    btnDelete->setIcon(QIcon(":/sources/icons/free-icon-trash-can-542673.svg"));
    btnDelete->setIconSize(QSize(18, 18));

    connect(btnEdit, &QPushButton::clicked, this, [this]() { emit editRequested(m_id); });
    connect(btnDelete, &QPushButton::clicked, this, [this]() { emit deleteRequested(m_id); });

    bottomLayout->addWidget(btnEdit);
    bottomLayout->addWidget(btnDelete);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(contentLabel, 1);
    mainLayout->addLayout(bottomLayout);

    updateStyle();
}

void NoteCard::updateStyle() {

    if (m_isHovered) {
        setStyleSheet(
            "QWidget#noteCard { "
            "   background-color: white; "
            "   border: 1px solid #D86B7A; "
            "   border-radius: 12px; "
            "   border-left: 6px solid #D86B7A; "
            "}"
            );
    } else {
        setStyleSheet(
            "QWidget#noteCard { "
            "   background-color: white; "
            "   border: 1px solid #E5E5E5; "
            "   border-radius: 12px; "
            "   border-left: 6px solid #E5E5E5; "
            "}"
            );
    }
}

void NoteCard::enterEvent(QEnterEvent *) { m_isHovered = true; updateStyle(); }
void NoteCard::leaveEvent(QEvent *) { m_isHovered = false; updateStyle(); }