#ifndef BOUQUETSPANEL_H
#define BOUQUETSPANEL_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QScrollArea>
#include <QShowEvent> // Обязательно добавь этот инклуд

class BouquetsPanel : public QWidget {
    Q_OBJECT
public:
    explicit BouquetsPanel(QWidget *parent = nullptr);
    void setCurrentUser(int userId);
    void loadBouquets();

signals:
    void goToConstructorRequested();

protected:
    // НОВЫЙ МЕТОД: срабатывает при каждом показе вкладки на экране
    void showEvent(QShowEvent *event) override;

private:
    void setupUI();
    void deleteBouquet(int id);

    int m_userId;
    QGridLayout *m_grid;
    QLabel *m_countLabel;
    QLineEdit *m_searchEdit;
    QComboBox *m_sortCombo;

};

#endif