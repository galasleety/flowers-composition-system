#include "mainwindow.h"
#include "loginwindow.h"
#include "registerwindow.h"
#include "greetingwindow.h"
#include "appwindow.h"
#include "adminwindow.h" // <--- ОБЯЗАТЕЛЬНО ПОДКЛЮЧАЕМ ПАНЕЛЬ АДМИНА

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScreen>
#include <QGuiApplication>
#include <QFont>
#include <QPainterPath>
#include <QDebug>
#include <QFontMetrics>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QPushButton>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPainter>
#include <QVariantAnimation>
#include <cmath>

// --- Реализация очень нежного фона ---
SoftBackgroundWidget::SoftBackgroundWidget(QWidget *parent)
    : QWidget(parent), m_phase(0.0)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    m_timer.start(50, this);
}

void SoftBackgroundWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor baseGray(245, 245, 245);
    painter.fillRect(rect(), baseGray);

    qreal offsetX = std::sin(m_phase) * 60.0;
    qreal offsetY = std::cos(m_phase * 0.8) * 50.0;
    QPointF center = rect().center() + QPointF(offsetX, offsetY);

    qreal gradientRadius = (qreal)rect().width() * 1.6;
    QRadialGradient grad(center, gradientRadius);

    grad.setColorAt(0.0, QColor(255, 220, 230, 145));
    grad.setColorAt(0.4, QColor(245, 245, 245, 70));
    grad.setColorAt(1.0, QColor(245, 245, 245, 0));

    painter.setBrush(grad);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
}

void SoftBackgroundWidget::timerEvent(QTimerEvent *event) {
    if (event->timerId() == m_timer.timerId()) {
        m_phase += 0.015;
        update();
    } else {
        QWidget::timerEvent(event);
    }
}

// --- Реализация ImageTextLabel ---
ImageTextLabel::ImageTextLabel(const QPixmap &imageSource, QWidget *parent)
    : QLabel(parent), m_sourceImage(imageSource)
{
    setText("WER");
    setObjectName("CompositionsText");
    setAlignment(Qt::AlignLeft);
}

void ImageTextLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setFont(font());

    QPixmap scaledImage = m_sourceImage.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPoint imageCenter = rect().center() - scaledImage.rect().center();
    QPainterPath textPath;
    QFontMetrics fm = painter.fontMetrics();
    QRect textRect = fm.boundingRect(rect(), alignment(), text());
    textPath.addText(textRect.topLeft() + QPoint(0, fm.ascent()), font(), text());
    painter.setClipPath(textPath);
    painter.drawPixmap(imageCenter, scaledImage);
    painter.setClipping(false);
}

// --- Реализация ImageMaskLabel ---
void ImageMaskLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QColor targetColor(228, 228, 228, 255);
    qreal gradStart = (qreal)width() * 0.99;
    qreal gradEnd = (qreal)width();
    QLinearGradient gradient(gradStart, 0, gradEnd, 0);
    gradient.setColorAt(0, QColor(targetColor.red(), targetColor.green(), targetColor.blue(), 0));
    gradient.setColorAt(1, targetColor);
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRect(QRectF(gradStart, 0, gradEnd - gradStart + 1.0, (qreal)height()));
}

// --- Реализация MainWindow ---
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), animationsStarted(false),
    centralWidget(nullptr), imageContainer(nullptr),
    contentPanel(nullptr), componentsLabel(nullptr),
    compositionsLabel(nullptr), descriptionLabel(nullptr),
    separator(nullptr), buttonContainer(nullptr)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setStyleSheet("QMainWindow { border: none; outline: none; } QWidget#centralWidget { border: none; }");

    showFullScreen();
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    setupUI(screenGeometry.width() / 2, screenGeometry.height());
}

MainWindow::~MainWindow() {}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!animationsStarted) {
        QTimer::singleShot(120, this, &MainWindow::startIntroAnimation);
        animationsStarted = true;
    }
}

void MainWindow::setupUI(int imagePanelWidth, int windowHeight)
{
    centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QPixmap sourceImage(":/images/sources/first2.jpg");

    imageContainer = new ImageMaskLabel();
    imageContainer->setPixmap(sourceImage.scaled(QSize(imagePanelWidth, windowHeight), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    imageContainer->setFixedWidth(imagePanelWidth);

    QVBoxLayout *leftLayout = new QVBoxLayout(imageContainer);
    leftLayout->setContentsMargins(20, 125, 20, 0);
    leftLayout->addStretch(0);
    componentsLabel = new QLabel("FLO", imageContainer);
    componentsLabel->setObjectName("ComponentsText");
    componentsLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    leftLayout->addWidget(componentsLabel);
    leftLayout->addStretch(4);
    mainLayout->addWidget(imageContainer);

    contentPanel = new SoftBackgroundWidget(centralWidget);
    contentPanel->setObjectName("contentPanel");
    contentPanel->setMinimumWidth(560);

    QVBoxLayout *rightLayout = new QVBoxLayout(contentPanel);
    rightLayout->setContentsMargins(20, 20, 20, 20);
    rightLayout->setSpacing(0);

    QPushButton *closeButton = new QPushButton("×", contentPanel);
    closeButton->setFixedSize(60, 60);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setObjectName("CloseButton");
    connect(closeButton, &QPushButton::clicked, this, &MainWindow::close);

    QHBoxLayout *topBarLayout = new QHBoxLayout();
    topBarLayout->addStretch();
    topBarLayout->addWidget(closeButton);
    rightLayout->addLayout(topBarLayout);
    rightLayout->addSpacing(45);
    rightLayout->addStretch(0);

    compositionsLabel = new ImageTextLabel(sourceImage, contentPanel);
    rightLayout->addWidget(compositionsLabel);
    rightLayout->addSpacing(17);

    separator = new QWidget(contentPanel);
    separator->setObjectName("SeparatorWidget");
    separator->setFixedHeight(4);
    rightLayout->addWidget(separator);
    rightLayout->addSpacing(35);

    descriptionLabel = new QLabel(contentPanel);
    descriptionLabel->setText("КОМПОЗИЦИИ И КОМПОНЕНТЫ ЦВЕТОЧНЫХ БУКЕТОВ");
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setObjectName("DescriptionContent");
    descriptionLabel->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    rightLayout->addWidget(descriptionLabel);

    buttonContainer = new QWidget(contentPanel);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(42, 40, 42, 0);
    buttonLayout->setSpacing(10);

    QPushButton *loginButton = new QPushButton("ВОЙТИ", buttonContainer);
    loginButton->setObjectName("MainScreenButton");
    loginButton->setCursor(Qt::PointingHandCursor);

    QPushButton *registerButton = new QPushButton("ЗАРЕГИСТРИРОВАТЬСЯ", buttonContainer);
    registerButton->setObjectName("MainScreenButton");
    registerButton->setCursor(Qt::PointingHandCursor);

    auto createOverlay = [this]() {
        QWidget *overlay = new QWidget(this);
        overlay->setGeometry(0, 0, this->width(), this->height());
        overlay->setStyleSheet("background-color: rgba(0, 0, 0, 0); border: none;");
        overlay->show();
        overlay->raise();

        QVariantAnimation *anim = new QVariantAnimation(overlay);
        anim->setDuration(300);
        anim->setStartValue(0);
        anim->setEndValue(115);
        anim->setEasingCurve(QEasingCurve::OutQuad);

        connect(anim, &QVariantAnimation::valueChanged, overlay, [overlay](const QVariant &value){
            overlay->setStyleSheet(QString("background-color: rgba(0, 0, 0, %1); border: none;").arg(value.toInt()));
        });

        anim->start(QAbstractAnimation::DeleteWhenStopped);
        return overlay;
    };

    // ==========================================
    // ЛОГИКА ОКНА ВХОДА С ПРОВЕРКОЙ РОЛИ АДМИНА
    // ==========================================
    connect(loginButton, &QPushButton::clicked, this, [this, createOverlay]() {
        QWidget *overlay = createOverlay();
        LoginWindow *logWindow = new LoginWindow(nullptr);
        logWindow->show();

        connect(logWindow, &LoginWindow::closingStarted, overlay, [overlay]() {
            QVariantAnimation *fadeOut = new QVariantAnimation(overlay);
            fadeOut->setDuration(250);
            fadeOut->setStartValue(115);
            fadeOut->setEndValue(0);
            fadeOut->setEasingCurve(QEasingCurve::InQuad);

            connect(fadeOut, &QVariantAnimation::valueChanged, overlay, [overlay](const QVariant &value){
                overlay->setStyleSheet(QString("background-color: rgba(0, 0, 0, %1); border: none;").arg(value.toInt()));
            });
            connect(fadeOut, &QVariantAnimation::finished, overlay, &QWidget::deleteLater);

            fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
        });

        // ЛОВИМ СИГНАЛ ВХОДА (role ПЕРЕДАЕТСЯ СЮДА)
        connect(logWindow, &LoginWindow::loginSuccessful, this, [this](int userId, const QString &username, const QString &role) {

            // --- ВОТ ОНО! ОПРЕДЕЛЯЕМ АДМИНА И ПЕРЕДАЕМ В ОКНО ПРИВЕТСТВИЯ ---
            bool isAdmin = (role == "admin");
            GreetingWindow *gw = new GreetingWindow(username, true, isAdmin, nullptr);
            gw->show();

            connect(gw, &GreetingWindow::changeBackgroundRequested, this, [this, userId, username, role, gw]() {
                QWidget *nextWindow = nullptr;

                // --- ПРОВЕРКА РОЛИ: КУДА ИДЕМ ДАЛЬШЕ? ---
                if (role == "admin") {
                    AdminWindow *adminWin = new AdminWindow(userId, nullptr);
                    adminWin->setWindowFlags(Qt::FramelessWindowHint);
                    adminWin->showFullScreen();
                    nextWindow = adminWin;
                } else {
                    AppWindow *appWin = new AppWindow(userId, username, nullptr);
                    nextWindow = appWin;
                }

                nextWindow->setWindowOpacity(0.0);
                nextWindow->show();

                QPropertyAnimation *fadeIn = new QPropertyAnimation(nextWindow, "windowOpacity");
                fadeIn->setDuration(600);
                fadeIn->setStartValue(0.0);
                fadeIn->setEndValue(1.0);
                fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

                QPropertyAnimation *fadeOut = new QPropertyAnimation(this, "windowOpacity");
                fadeOut->setDuration(600);
                fadeOut->setStartValue(1.0);
                fadeOut->setEndValue(0.0);
                connect(fadeOut, &QPropertyAnimation::finished, this, &QMainWindow::close);
                fadeOut->start(QAbstractAnimation::DeleteWhenStopped);

                gw->raise();
            });
        });
    });

    // ===============================================
    // ЛОГИКА ОКНА РЕГИСТРАЦИИ (НОВЫЙ ЮЗЕР ВСЕГДА user)
    // ===============================================
    connect(registerButton, &QPushButton::clicked, this, [this, createOverlay]() {
        QWidget *overlay = createOverlay();
        RegisterWindow *regWindow = new RegisterWindow(nullptr);
        regWindow->show();

        connect(regWindow, &RegisterWindow::closingStarted, overlay, [overlay]() {
            QVariantAnimation *fadeOut = new QVariantAnimation(overlay);
            fadeOut->setDuration(250);
            fadeOut->setStartValue(115);
            fadeOut->setEndValue(0);
            fadeOut->setEasingCurve(QEasingCurve::InQuad);

            connect(fadeOut, &QVariantAnimation::valueChanged, overlay, [overlay](const QVariant &value){
                overlay->setStyleSheet(QString("background-color: rgba(0, 0, 0, %1); border: none;").arg(value.toInt()));
            });
            connect(fadeOut, &QVariantAnimation::finished, overlay, &QWidget::deleteLater);

            fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
        });

        connect(regWindow, &RegisterWindow::registrationSuccessful, this, [this](int userId, const QString &username) {

            // --- ЗДЕСЬ ЖЕСТКО СТАВИМ false (НЕ АДМИН), ТАК КАК ЭТО НОВЫЙ ПОЛЬЗОВАТЕЛЬ ---
            GreetingWindow *gw = new GreetingWindow(username, false, false, nullptr);
            gw->show();

            connect(gw, &GreetingWindow::changeBackgroundRequested, this, [this, userId, username, gw]() {
                AppWindow *appWin = new AppWindow(userId, username, nullptr);
                appWin->setWindowOpacity(0.0);
                appWin->show();

                QPropertyAnimation *fadeIn = new QPropertyAnimation(appWin, "windowOpacity");
                fadeIn->setDuration(600);
                fadeIn->setStartValue(0.0);
                fadeIn->setEndValue(1.0);
                fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

                QPropertyAnimation *fadeOut = new QPropertyAnimation(this, "windowOpacity");
                fadeOut->setDuration(600);
                fadeOut->setStartValue(1.0);
                fadeOut->setEndValue(0.0);
                connect(fadeOut, &QPropertyAnimation::finished, this, &QMainWindow::close);
                fadeOut->start(QAbstractAnimation::DeleteWhenStopped);

                gw->raise();
            });
        });
    });

    buttonLayout->addWidget(loginButton);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(registerButton);
    rightLayout->addWidget(buttonContainer);
    rightLayout->addStretch(1);

    mainLayout->addWidget(contentPanel);
    mainLayout->setStretchFactor(imageContainer, 1);
    mainLayout->setStretchFactor(contentPanel, 1);
}

void MainWindow::startIntroAnimation()
{
    auto setupOpacity = [](QWidget* widget) -> QGraphicsOpacityEffect* {
        if (!widget) return nullptr;
        auto effect = new QGraphicsOpacityEffect(widget);
        effect->setOpacity(0.0);
        widget->setGraphicsEffect(effect);
        return effect;
    };

    QGraphicsOpacityEffect *leftOpacity = setupOpacity(componentsLabel);
    QGraphicsOpacityEffect *rightOpacity = setupOpacity(compositionsLabel);
    QGraphicsOpacityEffect *sepOpacity = setupOpacity(separator);
    QGraphicsOpacityEffect *descOpacity = setupOpacity(descriptionLabel);
    QGraphicsOpacityEffect *btnOpacity = setupOpacity(buttonContainer);

    QSequentialAnimationGroup *animationGroup = new QSequentialAnimationGroup(this);
    QParallelAnimationGroup *labelsAnimation = new QParallelAnimationGroup(this);

    if (leftOpacity) {
        QPropertyAnimation *a = new QPropertyAnimation(leftOpacity, "opacity", this);
        a->setDuration(800); a->setEndValue(1.0); a->setEasingCurve(QEasingCurve::InOutQuad);
        labelsAnimation->addAnimation(a);
    }
    if (rightOpacity) {
        QPropertyAnimation *a = new QPropertyAnimation(rightOpacity, "opacity", this);
        a->setDuration(800); a->setEndValue(1.0); a->setEasingCurve(QEasingCurve::InOutQuad);
        labelsAnimation->addAnimation(a);
    }
    animationGroup->addAnimation(labelsAnimation);

    if (sepOpacity) {
        QPropertyAnimation *a = new QPropertyAnimation(sepOpacity, "opacity", this);
        a->setDuration(320); a->setEndValue(1.0);
        animationGroup->addAnimation(a);
    }
    if (descOpacity) {
        QPropertyAnimation *a = new QPropertyAnimation(descOpacity, "opacity", this);
        a->setDuration(520); a->setEndValue(1.0); a->setEasingCurve(QEasingCurve::InOutQuad);
        animationGroup->addAnimation(a);
    }
    if (btnOpacity) {
        QPropertyAnimation *a = new QPropertyAnimation(btnOpacity, "opacity", this);
        a->setDuration(420); a->setEndValue(1.0); a->setEasingCurve(QEasingCurve::InOutQuad);
        animationGroup->addAnimation(a);
    }
    animationGroup->start(QAbstractAnimation::DeleteWhenStopped);
}