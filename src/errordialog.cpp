#include "errordialog.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QTimer>
#include <QSoundEffect>
#include <QUrl>

// --- ERROR DIALOG (Твой оригинальный стиль) ---
ErrorDialog::ErrorDialog(const QString &title, const QString &message, QWidget *parent)
    : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(510, 210);
    setupUI(title, message);

    QSoundEffect *effect = new QSoundEffect(this);
    effect->setSource(QUrl("qrc:/sources/music/sound.wav"));
    effect->play();

    QTimer::singleShot(3000, this, &ErrorDialog::fadeOutAndClose);
}

ErrorDialog::~ErrorDialog() {}

void ErrorDialog::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor("#FFFFFF"));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 10, 10);
}

void ErrorDialog::setupUI(const QString &title, const QString &message) {
    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(30, 25, 30, 20);
    layout->setHorizontalSpacing(15);
    layout->setVerticalSpacing(10);

    WarningIcon *icon = new WarningIcon(this);
    layout->addWidget(icon, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignLeft);

    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 24px; font-weight: bold; color: #333333;");
    layout->addWidget(titleLabel, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *msgLabel = new QLabel(message, this);
    msgLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 17px; color: #555555; line-height: 1.4;");
    msgLabel->setWordWrap(true);
    msgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    msgLabel->setMinimumWidth(400);
    layout->addWidget(msgLabel, 1, 1, Qt::AlignLeft | Qt::AlignTop);

    layout->setRowStretch(1, 1);

    QPushButton *okBtn = new QPushButton("ОК", this);
    okBtn->setFixedSize(100, 36);
    okBtn->setStyleSheet("QPushButton { background-color: #E14B4B; color: white; border-radius: 8px; font-family: 'Century Gothic'; font-size: 14px; font-weight: bold; } QPushButton:hover { background-color: #E85A5A; }");
    connect(okBtn, &QPushButton::clicked, this, &ErrorDialog::fadeOutAndClose);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    layout->addLayout(btnLayout, 2, 1);
}

void ErrorDialog::fadeOutAndClose() {
    QPropertyAnimation *anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(300);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    connect(anim, &QPropertyAnimation::finished, this, &QDialog::accept);
    anim->start(QPropertyAnimation::DeleteWhenStopped);
}

// --- CONFIRM DIALOG (Тот же стиль, но две кнопки) ---
ConfirmDialog::ConfirmDialog(const QString &title, const QString &message, QWidget *parent)
    : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(510, 210);
    setupUI(title, message);
}

ConfirmDialog::~ConfirmDialog() {}

void ConfirmDialog::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor("#FFFFFF"));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 10, 10);
}

void ConfirmDialog::setupUI(const QString &title, const QString &message) {
    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(30, 25, 30, 20);
    layout->setHorizontalSpacing(15);
    layout->setVerticalSpacing(10);

    WarningIcon *icon = new WarningIcon(this);
    layout->addWidget(icon, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignLeft);

    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 24px; font-weight: bold; color: #333333;");
    layout->addWidget(titleLabel, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *msgLabel = new QLabel(message, this);
    msgLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 17px; color: #555555; line-height: 1.4;");
    msgLabel->setWordWrap(true);
    msgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    msgLabel->setMinimumWidth(400);
    layout->addWidget(msgLabel, 1, 1, Qt::AlignLeft | Qt::AlignTop);

    layout->setRowStretch(1, 1);

    // Две кнопки в ряд
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    btnLayout->addStretch();

    QPushButton *cancelBtn = new QPushButton("Отмена", this);
    cancelBtn->setFixedSize(110, 36);
    cancelBtn->setStyleSheet("QPushButton { background-color: #F2F2F2; color: #555555; border-radius: 8px; font-family: 'Century Gothic'; font-size: 14px; } QPushButton:hover { background-color: #EAEAEA; }");

    QPushButton *delBtn = new QPushButton("Удалить", this);
    delBtn->setFixedSize(110, 36);
    delBtn->setStyleSheet("QPushButton { background-color: #E14B4B; color: white; border-radius: 8px; font-family: 'Century Gothic'; font-size: 14px; font-weight: bold; } QPushButton:hover { background-color: #E85A5A; }");

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(delBtn);
    layout->addLayout(btnLayout, 2, 1);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(delBtn, &QPushButton::clicked, this, &QDialog::accept);
}