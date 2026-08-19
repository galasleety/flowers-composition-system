#ifndef DETAILSCOLUMN_H
#define DETAILSCOLUMN_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QList>
#include <QScrollArea>
#include <QMap>

class DetailsColumn : public QWidget {
    Q_OBJECT
public:
    explicit DetailsColumn(QWidget *parent = nullptr);

    // === ДОБАВЬ ЭТИ МЕТОДЫ (ГЕТТЕРЫ) ===
    QString getHarmonyValue() const { return m_harmonyVal->text(); }
    QString getHarmonyDesc() const { return m_harmonyDesc->text(); }

    QString getCompatibilityValue() const { return m_compatVal->text(); }
    QString getCompatibilityDesc() const { return m_compatDesc->text(); }

    QString getStyleValue() const { return m_styleVal->text(); }
    QString getStyleDesc() const { return m_styleDesc->text(); }

    QString getLifeValue() const { return m_lifeVal->text(); }
    QString getLifeDesc() const { return m_lifeDesc->text(); }

    QString getScentValue() const { return m_scentVal->text(); }
    QString getScentDesc() const { return m_scentDesc->text(); }

    QString getToxValue() const { return m_toxVal->text(); }
    QString getToxDesc() const { return m_toxDesc->text(); }
    // ==================================

public slots:
    void updateAnalysis(const QList<int> &variationIds);

private:
    void setupUI();
    QWidget* createAnalysisBlock(const QString &title, QLabel* &valLbl, QLabel* &descLbl);
    void updateComposition(const QList<int> &variationIds);

    QHBoxLayout *m_colorsLayout;
    QVBoxLayout *m_compListLayout;

    QLabel *m_harmonyVal;    QLabel *m_harmonyDesc;
    QLabel *m_compatVal;     QLabel *m_compatDesc;
    QLabel *m_styleVal;      QLabel *m_styleDesc;
    QLabel *m_lifeVal;       QLabel *m_lifeDesc;
    QLabel *m_scentVal;      QLabel *m_scentDesc;
    QLabel *m_toxVal;        QLabel *m_toxDesc;
};

#endif