#ifndef ADDVARIATIONDIALOG_H
#define ADDVARIATIONDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QString>

class AddVariationDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddVariationDialog(int itemId, QWidget *parent = nullptr);
    bool isSaved() const { return m_saved; }

    // --- НОВЫЕ ФУНКЦИИ ---
    int getColorId() const;
    QString getImagePath() const;

private slots:
    void onBrowseImage();
    void onSaveClicked();
    void onCloseClicked();

private:
    void setupUI();
    void loadColors();

    int m_itemId;
    bool m_saved;
    QString m_selectedImagePath;

    QComboBox *m_colorCombo;
    QPushButton *m_btnBrowse;
    QPushButton *m_btnSave;
};

#endif // ADDVARIATIONDIALOG_H