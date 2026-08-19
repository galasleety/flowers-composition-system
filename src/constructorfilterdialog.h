#ifndef CONSTRUCTORFILTERDIALOG_H
#define CONSTRUCTORFILTERDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMap>

struct FilterSettings {
    QString style = "Все", color = "Все", type = "Все", safety = "Все";
    QString pollen = "Все", scent = "Все", bud = "Все", stem = "Все";
    QString durability = "Все", care = "Все", material = "Все", waterproof = "Все";
    QString customizable = "Все", attachment = "Все";
};

class ConstructorFilterDialog : public QDialog {
    Q_OBJECT
public:
    explicit ConstructorFilterDialog(int catId, const FilterSettings &current, QWidget *parent = nullptr);
    FilterSettings getSettings() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override; // Для центрирования

private:
    void setupUI();
    void loadDynamicData();
    QComboBox* createStyledCombo(const QString &key);

    int m_catId;
    FilterSettings m_settings;
    QMap<QString, QComboBox*> m_combos;

    QWidget *m_flowerGroup = nullptr;
    QWidget *m_packagingGroup = nullptr;
    QWidget *m_accessoryGroup = nullptr;
};

#endif // CONSTRUCTORFILTERDIALOG_H