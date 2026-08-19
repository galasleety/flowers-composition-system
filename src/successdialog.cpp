#include "successdialog.h"
#include <QPushButton>
#include <QFrame>
#include <QGraphicsDropShadowEffect>

SuccessDialog::SuccessDialog(const QString &message, QWidget *parent) : QDialog(parent) {
    // 1. Настройки прозрачности и отсутствия рамок
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(350, 250);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20); // Отступ для тени

    // 2. Белый контейнер
    QFrame *container = new QFrame(this);
    container->setStyleSheet("background-color: #FFFFFF; border-radius: 30px; border: none;");


    mainLayout->addWidget(container);

    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(30, 25, 30, 25);
    layout->setSpacing(15);

    // 3. Иконка или просто заголовок (можно добавить символ галочки)
    QLabel *icon = new QLabel("✓", this);
    icon->setStyleSheet("color: #27AE60; font-size: 40px; font-weight: bold; border: none;");
    icon->setAlignment(Qt::AlignCenter);
    layout->addWidget(icon);

    QLabel *msgLabel = new QLabel(message, this);
    msgLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 16px; color: #333; border: none;");
    msgLabel->setAlignment(Qt::AlignCenter);
    msgLabel->setWordWrap(true);
    layout->addWidget(msgLabel);

    // 4. Розовая кнопка ОК
    QPushButton *okBtn = new QPushButton("ОТЛИЧНО", this);
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setFixedSize(140, 45);
    okBtn->setStyleSheet(
        "QPushButton { background: #F8C3CD; border-radius: 22px; color: #333; "
        "font-weight: bold; font-family: 'Century Gothic'; border: none; } "
        "QPushButton:hover { background: #F5A9B8; }"
        );
    layout->addWidget(okBtn, 0, Qt::AlignCenter);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
}