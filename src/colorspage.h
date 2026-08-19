#ifndef COLORSPAGE_H
#define COLORSPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ColorsPage : public QWidget {
    Q_OBJECT

public:
    explicit ColorsPage(QWidget *parent = nullptr);
    void loadColors();

private slots:
    void onSearchTextChanged(const QString &text);
    void onColorSelected();
    void onAddColorClicked();
    void onEditColorClicked();
    void onDeleteColorClicked();

private:
    void setupUI();

    // Левая панель
    QLineEdit *m_searchEdit;
    QListWidget *m_colorsList;
    QPushButton *m_btnAddColor;

    // Правая панель
    QWidget *m_rightPanel;
    QFrame *m_infoCard;
    QLabel *m_colorPreviewBlock; // Квадратик с цветом

    QStackedWidget *m_textStack;

    // Режим просмотра
    QLabel *m_colorNameLabel;
    QLabel *m_colorHexLabel;
    QLabel *m_colorIdLabel;

    // Режим редактирования
    QLineEdit *m_nameEditField;
    QLineEdit *m_hexEditField;

    QPushButton *m_btnEditColor;
    QPushButton *m_btnDeleteColor;

    bool m_isEditing;
    bool m_isAddingNew;
};

#endif // COLORSPAGE_H