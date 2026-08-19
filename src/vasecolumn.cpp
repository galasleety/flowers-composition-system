#include "vasecolumn.h"
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPainter>
#include <QBuffer>
#include <QFrame>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>
#include <QPen>
#include <QSqlQuery>
#include <QSqlError>
#include <algorithm>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

static qreal getTargetSizeForVariation(int variationId) {
    QSqlQuery q;
    q.prepare("SELECT i.item_type FROM Items i JOIN Item_variation v ON i.item_id = v.item_id WHERE v.variation_id = :id");
    q.bindValue(":id", variationId);

    QString type = "flower";
    if (q.exec() && q.next()) {
        type = q.value(0).toString().toLower();
    }

    qreal baseSize = 250.0;
    if (type == "accessory") return baseSize / 3.0;
    if (type == "packaging") return baseSize * 2.0;

    return baseSize;
}

static void normalizeLayers(QGraphicsScene *scene) {
    if (!scene) return;
    QList<QGraphicsItem*> backs, flowers, fronts;

    for (auto item : scene->items()) {
        if (item->zValue() <= -5000) backs.append(item);
        else if (item->zValue() >= 5000) fronts.append(item);
        else flowers.append(item);
    }

    auto sortZ = [](QGraphicsItem *a, QGraphicsItem *b) { return a->zValue() < b->zValue(); };
    std::sort(backs.begin(), backs.end(), sortZ);
    std::sort(flowers.begin(), flowers.end(), sortZ);
    std::sort(fronts.begin(), fronts.end(), sortZ);

    for (int i = 0; i < backs.size(); ++i) backs[i]->setZValue(-10000 + i);
    for (int i = 0; i < flowers.size(); ++i) flowers[i]->setZValue(0 + i);
    for (int i = 0; i < fronts.size(); ++i) fronts[i]->setZValue(10000 + i);
}

BouquetItem::BouquetItem(int varId, const QPixmap &pixmap, bool isGhost)
    : QGraphicsPixmapItem(pixmap), m_varId(varId), m_isGhost(isGhost)
{
    if (m_isGhost) {
        setFlags(QGraphicsItem::GraphicsItemFlags());
        setAcceptedMouseButtons(Qt::NoButton);
    } else {
        setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
    }

    setTransformOriginPoint(pixmap.width() / 2.0, pixmap.height() / 2.0);
    setTransformationMode(Qt::SmoothTransformation);
}

BouquetItem::~BouquetItem() {
    if (m_pairedItem) {
        if (m_pairedItem->scene()) {
            m_pairedItem->scene()->removeItem(m_pairedItem);
        }
        delete m_pairedItem;
        m_pairedItem = nullptr;
    }
}

// ИДЕАЛЬНАЯ СИНХРОНИЗАЦИЯ: Учитывает разницу в ширине и высоте!
QVariant BouquetItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (m_pairedItem) {
        if (change == ItemPositionHasChanged) {
            QPointF newPos = value.toPointF();
            qreal offsetX = (boundingRect().width() - m_pairedItem->boundingRect().width()) / 2.0;
            qreal offsetY = (boundingRect().height() - m_pairedItem->boundingRect().height()) / 2.0;
            m_pairedItem->setPos(newPos.x() + offsetX, newPos.y() + offsetY);
        } else if (change == ItemRotationHasChanged) {
            m_pairedItem->setRotation(value.toReal());
        }
    }
    return QGraphicsPixmapItem::itemChange(change, value);
}

void BouquetItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QStyleOptionGraphicsItem myOption = *option;
    myOption.state &= ~QStyle::State_Selected;
    QGraphicsPixmapItem::paint(painter, &myOption, widget);

    if (isSelected() && !m_isGhost) {
        QPen pen(QColor("#F8C3CD"), 2, Qt::DashLine);
        pen.setCosmetic(true);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

BouquetView::BouquetView(QWidget *parent) : QGraphicsView(parent) {
    setAcceptDrops(true);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::RubberBandDrag);

    setStyleSheet("QGraphicsView { background: transparent; border: none; }");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QString scrollStyle =
        "QScrollBar:vertical { border: none; background: transparent; width: 8px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #D3D3D3; border-radius: 4px; min-height: 25px; }"
        "QScrollBar::handle:vertical:hover { background: #BDBDBD; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; border: none; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }";
    verticalScrollBar()->setStyleSheet(scrollStyle);
}

void BouquetView::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("application/x-bouquet-item")) event->acceptProposedAction();
}

void BouquetView::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
}

void BouquetView::dropEvent(QDropEvent *event) {
    QByteArray data = event->mimeData()->data("application/x-bouquet-item");
    QStringList parts = QString::fromUtf8(data).split("|");

    if (parts.size() >= 3) {
        emit itemDropped(parts[0].toInt(), parts[1], parts[2], mapToScene(event->position().toPoint()));
        event->acceptProposedAction();
    } else if (parts.size() >= 2) {
        emit itemDropped(parts[0].toInt(), parts[1], "", mapToScene(event->position().toPoint()));
        event->acceptProposedAction();
    }
}

VaseColumn::VaseColumn(QWidget *parent) : QWidget(parent) {
    setMinimumWidth(300);
    setupUI();
}

void VaseColumn::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(15);

    QLabel *title = new QLabel("Предпросмотр", this);
    title->setStyleSheet("font-family: 'Century Gothic'; font-size: 20px; color: #444444; font-weight: normal;");
    mainLayout->addWidget(title);

    QFrame *canvasFrame = new QFrame(this);
    canvasFrame->setStyleSheet("QFrame { background-color: #FFFFFF; border: 2px dashed #F8EAEC; border-radius: 12px; }");
    QVBoxLayout *canvasLayout = new QVBoxLayout(canvasFrame);
    canvasLayout->setContentsMargins(5, 5, 5, 5);

    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(0, 0, 1500, 1500);
    connect(m_scene, &QGraphicsScene::selectionChanged, this, &VaseColumn::updateButtonStates);

    m_view = new BouquetView(this);
    m_view->setScene(m_scene);
    m_view->setMinimumSize(100, 100);
    connect(m_view, &BouquetView::itemDropped, this, &VaseColumn::onItemDropped);

    canvasLayout->addWidget(m_view);
    mainLayout->addWidget(canvasFrame, 1);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(8);
    bottomLayout->setAlignment(Qt::AlignCenter);

    m_btnSave = new QPushButton("Сохранить", this);
    m_btnSave->setFixedSize(110, 40);
    m_btnSave->setCursor(Qt::PointingHandCursor);
    m_btnSave->setStyleSheet("QPushButton { background-color: #F8C3CD; color: #333; font-family: 'Century Gothic'; border-radius: 20px; border: none; } QPushButton:hover { background-color: #F5A9B8; }");
    connect(m_btnSave, &QPushButton::clicked, this, &VaseColumn::saveRequested);

    m_btnClear = new QPushButton("Очистить", this);
    m_btnClear->setFixedSize(100, 40);
    m_btnClear->setCursor(Qt::PointingHandCursor);
    m_btnClear->setStyleSheet("QPushButton { background-color: #FFFFFF; color: #555; font-family: 'Century Gothic'; border: 1px solid #E0E0E0; border-radius: 20px; } QPushButton:hover { border: 1px solid #CCCCCC; }");
    connect(m_btnClear, &QPushButton::clicked, this, &VaseColumn::onClearCanvas);

    bottomLayout->addWidget(m_btnSave);
    bottomLayout->addWidget(m_btnClear);
    bottomLayout->addSpacing(10);

    m_btnRotL = createToolButton("⟲", "Повернуть влево");
    connect(m_btnRotL, &QPushButton::clicked, this, &VaseColumn::onRotateLeft);
    m_btnRotR = createToolButton("⟳", "Повернуть вправо");
    connect(m_btnRotR, &QPushButton::clicked, this, &VaseColumn::onRotateRight);

    m_btnSendBackward = createToolButton("▼", "Ниже");
    connect(m_btnSendBackward, &QPushButton::clicked, this, &VaseColumn::onSendBackward);

    m_btnBringForward = createToolButton("▲", "Выше");
    connect(m_btnBringForward, &QPushButton::clicked, this, &VaseColumn::onBringForward);

    m_btnDelete = createToolButton("✕", "Удалить");
    connect(m_btnDelete, &QPushButton::clicked, this, &VaseColumn::onDeleteSelected);

    bottomLayout->addWidget(m_btnRotL);
    bottomLayout->addWidget(m_btnRotR);
    bottomLayout->addWidget(m_btnSendBackward);
    bottomLayout->addWidget(m_btnBringForward);
    bottomLayout->addWidget(m_btnDelete);

    mainLayout->addLayout(bottomLayout);
    updateButtonStates();
}

QPushButton* VaseColumn::createToolButton(const QString &text, const QString &tooltip) {
    QPushButton *btn = new QPushButton(text, this);
    btn->setFixedSize(36, 36);
    btn->setToolTip(tooltip);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet("QPushButton { background-color: transparent; border: 1px solid #EAEAEA; border-radius: 18px; color: #777; } QPushButton:hover:!disabled { border: 1px solid #F8C3CD; color: #333; } QPushButton:disabled { color: #DDD; border: 1px solid #F5F5F5; }");
    return btn;
}

void VaseColumn::updateButtonStates() {
    bool selected = !m_scene->selectedItems().isEmpty();
    m_btnRotL->setEnabled(selected);
    m_btnRotR->setEnabled(selected);
    m_btnBringForward->setEnabled(selected);
    m_btnSendBackward->setEnabled(selected);
    m_btnDelete->setEnabled(selected);
}

void VaseColumn::onRotateLeft() { for (auto i : m_scene->selectedItems()) i->setRotation(i->rotation() - 5); }
void VaseColumn::onRotateRight() { for (auto i : m_scene->selectedItems()) i->setRotation(i->rotation() + 5); }

void VaseColumn::onBringForward() {
    for (auto i : m_scene->selectedItems()) i->setZValue(i->zValue() + 1.1);
    normalizeLayers(m_scene);
}
void VaseColumn::onSendBackward() {
    for (auto i : m_scene->selectedItems()) i->setZValue(i->zValue() - 1.1);
    normalizeLayers(m_scene);
}

void VaseColumn::notifyChanges() {
    QList<int> ids;
    for (QGraphicsItem *item : m_scene->items()) {
        BouquetItem *bItem = dynamic_cast<BouquetItem*>(item);
        if (bItem && !bItem->isGhost()) {
            ids.append(bItem->getVariationId());
        }
    }
    emit compositionChanged(ids);
}

void VaseColumn::addItemToCenter(int variationId, const QString &imgFront, const QString &imgBack) {
    QPointF sceneCenter = m_view->mapToScene(m_view->viewport()->rect().center());
    onItemDropped(variationId, imgFront, imgBack, sceneCenter);
}

void VaseColumn::onItemDropped(int variationId, const QString &imgFrontParam, const QString &imgBackParam, QPointF pos) {
    QSqlQuery q;
    q.prepare("SELECT i.item_type, v.image_back FROM Items i JOIN Item_variation v ON i.item_id = v.item_id WHERE v.variation_id = :id");
    q.bindValue(":id", variationId);

    QString type = "flower";
    QString imgBack = imgBackParam.trimmed();

    if (q.exec() && q.next()) {
        type = q.value(0).toString().toLower();
        QString dbBack = q.value(1).toString().trimmed();
        if (!dbBack.isEmpty()) {
            imgBack = dbBack;
        }
    }

    QString imgFront = imgFrontParam.trimmed();
    if (imgFront.isEmpty() && imgBack.isEmpty()) return;

    qreal targetSize = getTargetSizeForVariation(variationId);

    qreal maxBack = -10000, maxFlower = 0, maxFront = 10000;
    for(auto i : m_scene->items()) {
        qreal z = i->zValue();
        if (z <= -5000 && z > maxBack) maxBack = z;
        else if (z > -5000 && z < 5000 && z > maxFlower) maxFlower = z;
        else if (z >= 5000 && z > maxFront) maxFront = z;
    }

    if (type == "packaging" && !imgBack.isEmpty()) {
        QPixmap pmBack(imgBack);
        QPixmap pmFront(imgFront);

        BouquetItem *itemBack = new BouquetItem(variationId, pmBack, false);
        BouquetItem *itemFront = new BouquetItem(variationId, pmFront, true);

        // НЕЗАВИСИМЫЙ МАСШТАБ: Теперь каждый слой рассчитывается от своего физического размера.
        // Зад и перед будут ИДЕАЛЬНО одного размера на холсте.
        qreal scaleFactorBack = targetSize / qMax((qreal)pmBack.width(), (qreal)pmBack.height());
        qreal scaleFactorFront = targetSize / qMax((qreal)pmFront.width(), (qreal)pmFront.height());

        itemBack->setScale(scaleFactorBack);
        itemFront->setScale(scaleFactorFront);

        itemBack->setPos(pos.x() - pmBack.width() / 2.0, pos.y() - pmBack.height() / 2.0);
        itemFront->setPos(pos.x() - pmFront.width() / 2.0, pos.y() - pmFront.height() / 2.0);

        // Привязываем ТОЛЬКО ПОСЛЕ того как расставили по позициям!
        itemBack->setPairedItem(itemFront);

        itemBack->setZValue(maxBack + 1);
        itemFront->setZValue(maxFront + 1);

        m_scene->addItem(itemBack);
        m_scene->addItem(itemFront);

        m_scene->clearSelection();
        itemBack->setSelected(true);
    } else {
        QString activeImg = imgFront.isEmpty() ? imgBack : imgFront;
        QPixmap pm(activeImg);
        BouquetItem *item = new BouquetItem(variationId, pm, false);

        qreal scaleFactor = targetSize / qMax((qreal)pm.width(), (qreal)pm.height());
        item->setScale(scaleFactor);
        item->setPos(pos.x() - pm.width() / 2.0, pos.y() - pm.height() / 2.0);

        if (type == "packaging") {
            item->setZValue(maxBack + 1);
        } else {
            item->setZValue(maxFlower + 1);
        }

        m_scene->addItem(item);
        m_scene->clearSelection();
        item->setSelected(true);
    }

    normalizeLayers(m_scene);
    notifyChanges();
}

void VaseColumn::onClearCanvas() {
    m_scene->clear();
    updateButtonStates();
    notifyChanges();
}

void VaseColumn::onDeleteSelected() {
    for (auto i : m_scene->selectedItems()) {
        m_scene->removeItem(i);
        delete i;
    }
    updateButtonStates();
    notifyChanges();
}

QMap<int, int> VaseColumn::getComponentCounts() const {
    QMap<int, int> counts;
    for (QGraphicsItem *item : m_scene->items()) {
        BouquetItem *bItem = dynamic_cast<BouquetItem*>(item);
        if (bItem && !bItem->isGhost()) {
            counts[bItem->getVariationId()]++;
        }
    }
    return counts;
}

QByteArray VaseColumn::getSnapshot() const {
    m_scene->clearSelection();
    QRectF rect = m_scene->itemsBoundingRect();
    if (rect.isEmpty()) return QByteArray();

    rect.adjust(-20, -20, 20, 20);

    QSize imageSize = rect.size().toSize() * 2;
    QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    m_scene->render(&painter, QRectF(QPointF(0,0), imageSize), rect);
    painter.end();

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG", 100);
    return ba;
}