#ifndef BOUQUETDETAILSDIALOG_H
#define BOUQUETDETAILSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>

class BouquetDetailsDialog : public QDialog {
    Q_OBJECT
public:
    // Исправленный конструктор
    explicit BouquetDetailsDialog(int bouquetId, QWidget *parent = nullptr);

private:
    void setupUI(int bid);
};

#endif