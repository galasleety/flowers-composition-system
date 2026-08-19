#ifndef CATALOGTOPBAR_H
#define CATALOGTOPBAR_H

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QCompleter>
#include <QStandardItemModel>
#include <QStringList>

class CatalogTopBar : public QWidget
{
    Q_OBJECT
public:
    explicit CatalogTopBar(QWidget *parent = nullptr);

    // Методы, чтобы карточка могла узнать, какие фильтры сейчас выбраны
    QString currentCategory() const;
    QString currentSearchText() const;
    QString currentSortText() const;

signals:
    // Сигнал, который говорит: "Фильтры изменились, пора обновить карточку!"
    void filtersChanged();

private slots:
    void updateSearchCompleter();
    void onSearchTextChanged(const QString &text);

private:
    void setupUI();

    QComboBox *m_categoryCombo;
    QLineEdit *m_searchEdit;
    QComboBox *m_sortCombo;

    QCompleter *m_searchCompleter;
    QStandardItemModel *m_searchModel;
    QStringList m_baseSuggestions;
};

#endif // CATALOGTOPBAR_H