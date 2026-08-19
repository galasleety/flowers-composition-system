#ifndef VASECOLUMN_H
#define VASECOLUMN_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPushButton>
#include <QMap>

// Умный класс элемента букета, который умеет иметь "призрачную" пару
class BouquetItem : public QGraphicsPixmapItem {
public:
    BouquetItem(int varId, const QPixmap &pixmap, bool isGhost = false);
    ~BouquetItem();

    int getVariationId() const { return m_varId; }

    // Является ли элемент просто визуальным верхним слоем (не учитывается в цене)
    bool isGhost() const { return m_isGhost; }

    // Привязать верхнюю часть упаковки к нижней
    void setPairedItem(BouquetItem *item) { m_pairedItem = item; }
    BouquetItem* pairedItem() const { return m_pairedItem; }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    // Перехватываем перемещение, поворот и масштаб, чтобы двигать обе части упаковки синхронно
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    int m_varId;
    bool m_isGhost;
    BouquetItem *m_pairedItem = nullptr;
};

class BouquetView : public QGraphicsView {
    Q_OBJECT
public:
    explicit BouquetView(QWidget *parent = nullptr);
signals:
    // Сигнал теперь передает и переднюю, и заднюю картинки
    void itemDropped(int variationId, const QString &imgFront, const QString &imgBack, QPointF pos);
protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

class VaseColumn : public QWidget {
    Q_OBJECT
public:
    explicit VaseColumn(QWidget *parent = nullptr);

    QMap<int, int> getComponentCounts() const;
    QByteArray getSnapshot() const;

public slots:
    // Функция добавления по клику теперь тоже принимает задник
    void addItemToCenter(int variationId, const QString &imgFront, const QString &imgBack = "");

signals:
    void compositionChanged(const QList<int> &variationIds);
    void saveRequested();

private slots:
    void onItemDropped(int variationId, const QString &imgFront, const QString &imgBack, QPointF pos);
    void onClearCanvas();
    void onDeleteSelected();
    void onRotateLeft();
    void onRotateRight();
    void onBringForward();
    void onSendBackward();
    void updateButtonStates();

private:
    void setupUI();
    QPushButton* createToolButton(const QString &text, const QString &tooltip);
    void notifyChanges();

    QGraphicsScene *m_scene;
    BouquetView *m_view;
    QPushButton *m_btnSave;
    QPushButton *m_btnClear;
    QPushButton *m_btnRotL;
    QPushButton *m_btnRotR;
    QPushButton *m_btnSendBackward;
    QPushButton *m_btnBringForward;
    QPushButton *m_btnDelete;
};

#endif // VASECOLUMN_H