#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <QPaintEvent>
#include <QShowEvent>
#include <QWidget>
#include <QBasicTimer>

// Класс для очень нежного динамического фона
class SoftBackgroundWidget : public QWidget {
public:
    explicit SoftBackgroundWidget(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
private:
    QBasicTimer m_timer;
    qreal m_phase; // Отвечает за движение центра градиента
};

class ImageTextLabel : public QLabel {
public:
    explicit ImageTextLabel(const QPixmap &imageSource, QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QPixmap m_sourceImage;
};

class ImageMaskLabel : public QLabel {
protected:
    void paintEvent(QPaintEvent *event) override;
};

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void showEvent(QShowEvent *event) override;
private:
    void setupUI(int imagePanelWidth, int windowHeight);
    void startIntroAnimation();

    bool animationsStarted;
    QWidget *centralWidget;
    ImageMaskLabel *imageContainer;
    SoftBackgroundWidget *contentPanel; // Наш новый виджет с градиентом
    QLabel *componentsLabel;
    ImageTextLabel *compositionsLabel;
    QLabel *descriptionLabel;
    QWidget *separator;
    QWidget *buttonContainer;
};

#endif // MAINWINDOW_H