#ifndef CATALOGMINICARD_H
#define CATALOGMINICARD_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QHBoxLayout>

class CatalogMiniCard : public QWidget {
    Q_OBJECT

public:
    explicit CatalogMiniCard(int id, const QString &name, const QString &imagePath, bool variationMode = false, QWidget *parent = nullptr);

    void setSelected(bool selected);
    void setDoubleImage(const QString &imgBack);

    // ВОТ ЭТА ПОТЕРЯННАЯ ФУНКЦИЯ ДЛЯ ВЫЗОВА ИЗ ДРУГИХ ФАЙЛОВ:
    int getId() const { return m_id; }

signals:
    void itemClicked(int id, const QString &imagePath);
    void clicked(int id);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void updateStyle();
    void loadColors();

    int m_id;
    QString m_imagePath;
    bool m_isVariationMode;
    bool m_isSelected;
    bool m_isHovered;

    QLabel *m_imgLabel;
    QLabel *m_nameLabel;
    QHBoxLayout *m_dotsLayout;
};

#endif // CATALOGMINICARD_H