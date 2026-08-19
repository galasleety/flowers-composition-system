#ifndef SAVEBOUQUETDIALOG_H
#define SAVEBOUQUETDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>

// Структура данных для анализа (должна быть ВНЕ класса или перед ним)
struct AnalysisData {
    QString harmony, harmonyDesc;
    QString compatibility, compatibilityDesc;
    QString style, styleDesc;
    QString life, lifeDesc;
    QString scent, scentDesc;
    QString tox, toxDesc;
};

class SaveBouquetDialog : public QDialog {
    Q_OBJECT
public:
    explicit SaveBouquetDialog(const AnalysisData &analysis, QWidget *parent = nullptr);
    QString getBouquetName() const { return m_nameEdit->text(); }

private:
    QLineEdit *m_nameEdit;
};

#endif