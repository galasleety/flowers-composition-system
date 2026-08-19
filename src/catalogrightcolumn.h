#ifndef CATALOGRIGHTCOLUMN_H
#define CATALOGRIGHTCOLUMN_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser> // Используем для скроллинга текста

class CatalogRightColumn : public QWidget
{
    Q_OBJECT

public:
    explicit CatalogRightColumn(QWidget *parent = nullptr);
    void loadItem(int itemId, int currentVariationId);

signals:
    void variationSelected(int variationId);

private:
    void setupUI();
    void clearLayout(QLayout *layout);

    void loadColors(int itemId, int currentVariationId);
    void loadInfoBlocks(int itemId);

    QVBoxLayout *m_mainLayout;

    // Контейнер для списка цветов (теперь они идут в столбик)
    QVBoxLayout *m_colorsListLayout;

    // Блок "Особенности"
    QWidget *m_featuresBlock;
    QTextBrowser *m_featuresText;

    // Блок "Уход и параметры"
    QWidget *m_careBlock;
    QTextBrowser *m_careText;
};

#endif // CATALOGRIGHTCOLUMN_H