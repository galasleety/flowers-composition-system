#ifndef ACCESSORIESPAGE_H
#define ACCESSORIESPAGE_H

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

class AccessoriesPage : public QWidget {
    Q_OBJECT
public:
    explicit AccessoriesPage(QWidget *parent = nullptr);
    void loadAccessories();

private slots:
    void onSearchTextChanged(const QString &text);
    void onAccessorySelected();
    void onDeleteAccessoryClicked();
    void onEditAccessoryClicked();

    void onAddAccessoryClicked();

    void onAddVariationClicked();
    void onDeleteVariationClicked(int variationId);

private:
    void setupUI();
    void loadVariations(int itemId);

    // Левая панель
    QLineEdit *m_searchEdit;
    QListWidget *m_accessoriesList;
    QPushButton *m_btnAddAccessory;

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
    QLabel *m_valCustomizable, *m_valAttachment, *m_valDurability, *m_valHeat, *m_valNotes;

    // Характеристики (РЕДАКТИРОВАНИЕ)
    QComboBox *m_comboCustomizable;
    QComboBox *m_comboAttachment;
    QComboBox *m_comboDurability;
    QComboBox *m_comboHeat;
    QComboBox *m_comboNotes;

    // Секция вариаций
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

    QPushButton *m_btnEditAccessory;
    QPushButton *m_btnDeleteAccessory;

    bool m_isEditing;
    bool m_isAddingNew;
};

#endif // ACCESSORIESPAGE_H