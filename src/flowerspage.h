#ifndef FLOWERSPAGE_H
#define FLOWERSPAGE_H

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

class FlowersPage : public QWidget {
    Q_OBJECT
public:
    explicit FlowersPage(QWidget *parent = nullptr);
    void loadFlowers();

private slots:
    void onSearchTextChanged(const QString &text);
    void onFlowerSelected();
    void onDeleteFlowerClicked();
    void onEditFlowerClicked();

    void onAddFlowerClicked();

    void onAddVariationClicked();
    void onDeleteVariationClicked(int variationId);

private:
    void setupUI();
    void loadVariations(int itemId);

    // Левая панель
    QLineEdit *m_searchEdit;
    QListWidget *m_flowersList;
    QPushButton *m_btnAddFlower;

    // Правая панель
    QWidget *m_rightPanel;
    QFrame *m_infoCard;
    QLabel *m_flowerIconLabel;

    QStackedWidget *m_textStack;

    // Шапка: Просмотр
    QLabel *m_flowerNameLabel;
    QLabel *m_flowerDescLabel;
    QLabel *m_flowerIdLabel;

    // Шапка: Редактирование
    QLineEdit *m_nameEditField;
    QTextEdit *m_descEditField;

    // Характеристики (ПРОСМОТР)
    QStackedWidget *m_detailsStack;
    QLabel *m_valBudSize, *m_valScent, *m_valStem, *m_valPollen, *m_valLifetime, *m_valPoison;

    // Характеристики (РЕДАКТИРОВАНИЕ - ВСЕ COMBOBOX)
    QComboBox *m_editBudSize;
    QComboBox *m_editScent;
    QComboBox *m_editStem;
    QComboBox *m_editPollen;
    QComboBox *m_editLifetime;
    QComboBox *m_comboPoison;

    // Секция вариаций
    QLabel *m_varTitle;
    QPushButton *m_btnAddVariation;
    QScrollArea *m_varScrollArea;
    QWidget *m_variationsContainer;
    QHBoxLayout *m_variationsLayout;

    QPushButton *m_btnEditFlower;
    QPushButton *m_btnDeleteFlower;

    bool m_isEditing;
    bool m_isAddingNew;

    struct PendingVariation {
        int colorId;
        QString colorName;
        QString imagePath;
    };
    QList<PendingVariation> m_pendingVariations;
    void renderPendingVariations();
};

#endif // FLOWERSPAGE_H