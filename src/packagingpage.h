#ifndef PACKAGINGPAGE_H
#define PACKAGINGPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>

class PackagingPage : public QWidget {
    Q_OBJECT
public:
    explicit PackagingPage(QWidget *parent = nullptr);
    void loadPackaging();

private slots:
    void onSearchTextChanged(const QString &text);
    void onPackagingSelected();
    void onDeletePackagingClicked();
    void onEditPackagingClicked();

    void onAddPackagingClicked();

    void onAddVariationClicked();
    void onDeleteVariationClicked(int variationId);

private:
    void setupUI();
    void loadVariations(int itemId);

    // Левая панель
    QLineEdit *m_searchEdit;
    QListWidget *m_packagingList;
    QPushButton *m_btnAddPackaging;

    // Правая панель
    QWidget *m_rightPanel;
    QFrame *m_infoCard;
    QLabel *m_iconLabel;

    QStackedWidget *m_textStack;

    // Шапка: Просмотр
    QLabel *m_nameLabel;
    QLabel *m_descLabel;
    QLabel *m_idLabel;

    // Шапка: Редактирование
    QLineEdit *m_nameEditField;
    QTextEdit *m_descEditField;

    // Характеристики (ПРОСМОТР)
    QStackedWidget *m_detailsStack;
    QLabel *m_valMaterial, *m_valWaterproof, *m_valBreathability, *m_valShockAbs;

    // Характеристики (РЕДАКТИРОВАНИЕ)
    QComboBox *m_editMaterial;
    QComboBox *m_editWaterproof;
    QComboBox *m_editBreathability;
    QComboBox *m_editShockAbs;

    // Секция вариаций (та же логика, что у цветов)
    QLabel *m_varTitle;
    QPushButton *m_btnAddVariation;
    QScrollArea *m_varScrollArea;
    QWidget *m_variationsContainer;
    QHBoxLayout *m_variationsLayout;

    struct PendingVariation {
        int colorId;
        QString colorName;
        QString imagePath;
    };
    QList<PendingVariation> m_pendingVariations;
    void renderPendingVariations();

    QPushButton *m_btnEditPackaging;
    QPushButton *m_btnDeletePackaging;

    bool m_isEditing;
    bool m_isAddingNew;
};

#endif // PACKAGINGPAGE_H