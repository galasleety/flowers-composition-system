#ifndef SUCCESSDIALOG_H
#define SUCCESSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>

class SuccessDialog : public QDialog {
    Q_OBJECT
public:
    explicit SuccessDialog(const QString &message, QWidget *parent = nullptr);
};

#endif