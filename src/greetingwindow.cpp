#include "greetingwindow.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QScreen>
#include <QGuiApplication>

GreetingWindow::GreetingWindow(const QString &username, bool isLogin, bool isAdmin, QWidget *parent)
    : QWidget(parent), m_isAdmin(isAdmin)
{
    // Окно поверх всех, без рамок.
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground); // ВАЖНО: Делает углы за пределами скругления невидимыми

    setFixedSize(500, 260);

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect g = screen->geometry();
        move((g.width() - width()) / 2, (g.height() - height()) / 2);
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(25);

    QLabel *titleLabel = new QLabel(this);
    QLabel *subLabel = new QLabel(this);
    QWidget *separator = new QWidget(this);
    separator->setFixedSize(80, 4);

    // ==========================================
    // ЛОГИКА ДИЗАЙНА: АДМИН vs ПОЛЬЗОВАТЕЛЬ
    // ==========================================
    if (m_isAdmin) {
        // СВЕТЛЫЙ, НО ДРУГОЙ ДИЗАЙН ДЛЯ АДМИНА
        titleLabel->setText(QString("Панель администратора\n%1").arg(username));
        titleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 24px; font-weight: bold; color: #333333;");

        separator->setStyleSheet("background-color: #D86B7A; border-radius: 2px;"); // ЯРКАЯ красная полоса

        subLabel->setText("Настройка прав доступа и загрузка системы...");
        subLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 15px; color: #666666;");
    } else {
        // СТАНДАРТНЫЙ ДИЗАЙН ПОЛЬЗОВАТЕЛЯ
        if (isLogin) {
            titleLabel->setText(QString("С возвращением, %1!").arg(username));
            subLabel->setText("Рады видеть вас снова. Загружаем интерфейс...");
        } else {
            titleLabel->setText(QString("Добро пожаловать, %1!").arg(username));
            subLabel->setText("Аккаунт успешно создан. Загружаем интерфейс...");
        }
        titleLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 28px; font-weight: bold; color: #333333;");
        separator->setStyleSheet("background-color: #F8C3CD; border-radius: 2px;"); // НЕЖНО-розовая полоса
        subLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 17px; color: #666666;");
    }

    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);
    subLabel->setAlignment(Qt::AlignCenter);
    subLabel->setWordWrap(true);

    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    layout->addWidget(separator, 0, Qt::AlignHCenter);
    layout->addWidget(subLabel, 0, Qt::AlignHCenter);
}

GreetingWindow::~GreetingWindow() {}

void GreetingWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // ОБА ОКНА ТЕПЕРЬ БЕЛЫЕ
    QLinearGradient grad(0, 0, width(), height());
    grad.setColorAt(0.0, QColor("#FFFFFF"));
    grad.setColorAt(1.0, QColor("#FFF4F8"));

    painter.setBrush(grad);
    painter.setPen(Qt::NoPen);

    // Рисуем скругленный прямоугольник
    painter.drawRoundedRect(rect(), 25, 25);
}

void GreetingWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // --- ИДЕАЛЬНАЯ АНИМАЦИЯ БЕЗ СЕРЫХ РАМОК ---
    setWindowOpacity(0.0); // Делаем окно полностью прозрачным

    // Анимируем свойство windowOpacity самого окна, а не наложенный эффект
    QPropertyAnimation *anim = new QPropertyAnimation(this, "windowOpacity", this);
    anim->setDuration(400);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutQuad);

    connect(anim, &QPropertyAnimation::finished, this, [this]() {
        QTimer::singleShot(2000, this, [this]() {
            emit changeBackgroundRequested();
            QTimer::singleShot(2000, this, &GreetingWindow::fadeOutAndClose);
        });
    });

    anim->start(QPropertyAnimation::DeleteWhenStopped);
}

void GreetingWindow::fadeOutAndClose()
{
    QPropertyAnimation *anim = new QPropertyAnimation(this, "windowOpacity", this);
    anim->setDuration(300);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::InQuad);

    connect(anim, &QPropertyAnimation::finished, this, [this]() {
        emit finished();
        close();
    });

    anim->start(QPropertyAnimation::DeleteWhenStopped);
}