#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPainter>

// Переносим иконку сюда, чтобы её видели и Error, и Confirm
class WarningIcon : public QWidget {
public:
    WarningIcon(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(40, 40);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor("#E14B4B"));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(rect());
        painter.setPen(Qt::white);
        painter.setFont(QFont("Century Gothic", 22, QFont::Bold));
        painter.drawText(rect().adjusted(0, -2, 0, 0), Qt::AlignCenter, "!");
    }
};

// Твоё окно ошибки
class ErrorDialog : public QDialog {
    Q_OBJECT
public:
    explicit ErrorDialog(const QString &title, const QString &message, QWidget *parent = nullptr);
    ~ErrorDialog();
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    void setupUI(const QString &title, const QString &message);
    void fadeOutAndClose();
};

// Твоё новое окно подтверждения
class ConfirmDialog : public QDialog {
    Q_OBJECT
public:
    explicit ConfirmDialog(const QString &title, const QString &message, QWidget *parent = nullptr);
    ~ConfirmDialog();
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    void setupUI(const QString &title, const QString &message);
};

#endif