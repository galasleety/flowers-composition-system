#ifndef CONSTRUCTORVIEW_H
#define CONSTRUCTORVIEW_H

#include "constructorfilterdialog.h"
#include "vasecolumn.h" // Подключаем класс центральной колонки
#include "detailscolumn.h"

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>
#include <QGridLayout>

class ConstructorView : public QWidget
{
    Q_OBJECT
public:
    explicit ConstructorView(QWidget *parent = nullptr);
    void setCurrentUser(int userId);
    void onSaveBouquetRequested();

private slots:
    void onSourceToggled(int id, bool checked);
    void onCategoryToggled(int id, bool checked);
    void onSearchChanged(const QString &text);
    void onFiltersClicked();

private:
    void setupUI();
    QWidget* createLeftColumn();
    QWidget* createCenterColumn();
    QWidget* createRightColumn();

    void loadComponentsGrid();

    int m_currentUserId;
    bool m_filtersActive;

    QLineEdit *m_searchEdit;
    QButtonGroup *m_sourceGroup;
    QPushButton *m_btnCatalog;
    QPushButton *m_btnFavorites;
    QPushButton *m_btnFilters;

    QButtonGroup *m_categoryGroup;
    QGridLayout *m_componentsGrid;
    FilterSettings m_activeSettings;

    VaseColumn *m_vaseColumn; // <--- ДОБАВЛЕНО
    DetailsColumn *m_detailsColumn;
};

#endif // CONSTRUCTORVIEW_H