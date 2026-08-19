#ifndef PALETTEPAGE_H
#define PALETTEPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QStackedWidget>

class PalettePage : public QWidget {
    Q_OBJECT
public:
    explicit PalettePage(QWidget *parent = nullptr);
    void loadColors();

private slots:
    void onSearchTextChanged(const QString &text);
    void onColorSelected();
    void onEditColorClicked();
    void onAddColorClicked();
    void onHexEdited(const QString &text);
    void onPickColorClicked(); // Слот для вызова диалога палитры

private:
    void setupUI();
    void loadUsedProducts(int colorId);
    QWidget* createProductCard(const QString &name, const QString &imgPath);
    QIcon createColorIcon(const QString &hex, int size);

    // Флаги состояния
    bool m_isEditing = false;
    bool m_isAddingNew = false;

    // Левая панель
    QLineEdit *m_searchEdit = nullptr;
    QListWidget *m_colorsList = nullptr;
    QPushButton *m_btnAddColor = nullptr;

    // Правая панель
    QStackedWidget *m_rightStack = nullptr;
    QLabel *m_bigColorPreview = nullptr;
    QStackedWidget *m_textStack = nullptr;

    // Режим просмотра
    QLabel *m_colorNameLabel = nullptr;
    QLabel *m_colorHexLabel = nullptr;
    QLabel *m_colorIdLabel = nullptr;

    // Режим редактирования
    QLineEdit *m_nameEditField = nullptr;
    QLineEdit *m_hexEditField = nullptr;
    QPushButton *m_btnPickColor = nullptr; // Кнопка вызова палитры

    QPushButton *m_btnEditColor = nullptr;

    // Сетка товаров
    QLabel *m_usedInLabel = nullptr;
    QGridLayout *m_productsGrid = nullptr;
};

#endif // PALETTEPAGE_H