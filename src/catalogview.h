#ifndef CATALOGVIEW_H
#define CATALOGVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QList>
#include <QScrollArea> // Добавь это
#include "catalogbottomgrid.h"

class CatalogTopBar;
class CatalogRightColumn; // Предварительное объявление правой колонки

class CatalogView : public QWidget
{
    Q_OBJECT

public:
    explicit CatalogView(QWidget *parent = nullptr);

    void setCurrentUser(int userId);
    void scrollToAndSelectItem(int itemId, const QString &itemType);

private slots:
    void loadMainCatalog();
    void displaySlide(int index);
    void onNextClicked();
    void onPrevClicked();
    void onFavoriteClicked();
    void onVariationSelected(int variationId); // Слот для смены цвета


private:
    void setupUI();
    QWidget* createMainCard();
    void clearLayout(QLayout *layout);
    void updatePaginationLabel();
    void updateFavoriteButtonState();
    void updateImage();
    CatalogBottomGrid *m_bottomGrid;

    int m_currentUserId = -1;

    QList<int> m_slideItemIds;
    int m_currentSlideIndex = 0;

    QList<int> m_itemVariationIds;
    int m_currentVariationId = -1;
    int m_currentVariationIndex = 0;
    int m_totalVariations = 0;

    CatalogTopBar *m_topBar;
    CatalogRightColumn *m_rightColumn; // Указатель на правую колонку

    // Левая колонка
    QLabel *m_titleLabel;
    QLabel *m_descLabel;
    QHBoxLayout *m_tagsLayout;
    QPushButton *m_favoriteBtn;
    QLabel *m_variationsCountLabel;

    // Изображение
    QLabel *m_imageLabel;

    // Навигация
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;
};

#endif // CATALOGVIEW_H