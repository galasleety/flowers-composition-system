#ifndef SEASONALITYPAGE_H
#define SEASONALITYPAGE_H

#include <QWidget>
#include <QScrollArea>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QDialog>
#include <QPushButton>

// --- ДИАЛОГ РЕДАКТИРОВАНИЯ ---
class SeasonEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit SeasonEditDialog(int itemId, const QString &name, const QString &imgPath, const QList<int> &currentMonths, QWidget *parent = nullptr);
    QList<int> getSelectedMonths() const { return m_selectedMonths; }

protected:
    void paintEvent(QPaintEvent *) override;

private slots:
    void onMonthToggled();
    void onSeasonClicked();
    void onSaveClicked();

private:
    void setupUI(const QString &name, const QString &imgPath);
    void updateMonthButtons();

    int m_itemId;
    QList<int> m_selectedMonths;
    QPushButton* m_monthButtons[12];

    const QString m_monthColors[12] = {
        "#A1D4F1", "#A1D4F1", // Янв, Фев
        "#BDE2B9", "#BDE2B9", // Мар, Апр
        "#FDF1A9", "#FDF1A9", // Май, Июн
        "#FDF1A9", "#F3AD9F", // Июл, Авг
        "#F3AD9F", "#A1D4F1", // Сен, Окт
        "#A1D4F1", "#A1D4F1"  // Ноя, Дек
    };
};

// --- ОСНОВНАЯ СТРАНИЦА ---
class SeasonalityPage : public QWidget {
    Q_OBJECT
public:
    explicit SeasonalityPage(QWidget *parent = nullptr);
    void loadData();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onSearchTextChanged(const QString &text);

private:
    void setupUI();
    void showOverlay();
    void hideOverlay();
    QPixmap getRoundedPixmap(const QString &path, int size);
    QWidget* createCard(int itemId, const QString &name, const QString &imgPath, const QList<int> &activeMonths);

    QLineEdit *m_searchEdit = nullptr;
    QScrollArea *m_mainScroll = nullptr;
    QWidget *m_scrollContent = nullptr;
    QGridLayout *m_gridLayout = nullptr;
    QWidget *m_overlay = nullptr;
    QAction *m_clearAction = nullptr;


    const QString m_monthColors[12] = {
        "#A1D4F1", "#A1D4F1", "#BDE2B9", "#BDE2B9", "#FDF1A9", "#FDF1A9",
        "#FDF1A9", "#F3AD9F", "#F3AD9F", "#A1D4F1", "#A1D4F1", "#A1D4F1"
    };
};

#endif // SEASONALITYPAGE_H