#ifndef FAVORITESPANEL_H
#define FAVORITESPANEL_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>

class FavoritesPanel : public QWidget {
    Q_OBJECT
public:
    explicit FavoritesPanel(QWidget *parent = nullptr);
    void setCurrentUser(int userId);

public slots:
    void loadFavorites(); // Метод для обновления списка

private slots:
    void onRemoveRequested(int variationId);

signals:
    void goToCatalogRequested(); // Сигнал для перехода

private:
    void setupUI();
    int m_userId;
    QGridLayout *m_favoritesGrid;
    QLabel *m_countLabel;
    QComboBox  *m_sortCombo;

};

#endif