#ifndef CATALOGBOTTOMGRID_H
#define CATALOGBOTTOMGRID_H

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QIcon> // Подключаем класс QIcon

class CatalogBottomGrid : public QWidget
{
    Q_OBJECT
public:
    explicit CatalogBottomGrid(QWidget *parent = nullptr);
    void refresh(const QString &category = "", const QString &searchText = "");
    void highlightItem(int itemId);

signals:
    void itemSelected(int itemId);

private:
    // ИСПРАВЛЕНО: Теперь второй параметр принимает настоящую иконку QIcon
    void addCategorySection(const QString &title, const QIcon &icon, const QString &sqlQuery);

    QVBoxLayout *m_mainLayout;
};

#endif // CATALOGBOTTOMGRID_H