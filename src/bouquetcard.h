#ifndef BOUQUETCARD_H
#define BOUQUETCARD_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class BouquetCard : public QWidget {
    Q_OBJECT
public:
    explicit BouquetCard(int id, const QString &name, const QString &date, const QByteArray &imageData, QWidget *parent = nullptr);

signals:
    void detailsRequested(int id);
    void deleteRequested(int id);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    int m_id;
    bool m_isHovered;
    void updateStyle();
};

#endif