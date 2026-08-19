#include "savebouquetdialog.h"
#include <QPushButton>
#include <QFrame>
#include <QScrollBar>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>

SaveBouquetDialog::SaveBouquetDialog(const AnalysisData &analysis, QWidget *parent)
    : QDialog(parent)
{
    // Убираем системные рамки и делаем фон прозрачным
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(480, 620); // Немного увеличим для красоты

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    // Отступы по бокам важны, чтобы тень плавно рассеивалась и не создавала рамок
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Главный белый контейнер
    QFrame *container = new QFrame(this);
    // УБРАНА СЕРАЯ РАМКА (border: none)
    container->setStyleSheet("background-color: #FFFFFF; border-radius: 35px; border: none;");


    mainLayout->addWidget(container);

    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(35, 35, 35, 35);
    layout->setSpacing(20);

    QLabel *title = new QLabel("Сохранение", this);
    title->setStyleSheet("font-family: 'Century Gothic'; font-size: 26px; font-weight: bold; color: #111; border: none;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    m_nameEdit = new QLineEdit("Мой новый букет", this);
    m_nameEdit->setFixedHeight(50);
    m_nameEdit->setStyleSheet(
        "QLineEdit { border: 1px solid #F3F3F3; border-radius: 15px; padding: 0 18px; font-family: 'Century Gothic'; font-size: 18px; background: #FAFAFA; } "
        "QLineEdit:focus { border: 1px solid #F8C3CD; background: #FFFFFF; }"
        );
    layout->addWidget(m_nameEdit);

    QLabel *header = new QLabel("ХАРАКТЕРИСТИКИ", this);
    header->setStyleSheet("font-size: 15px; font-weight: bold; color: #CCC; letter-spacing: 1.5px; border: none; margin-top: 5px;");
    header->setAlignment(Qt::AlignCenter);
    layout->addWidget(header);

    // Скролл-зона: ГАРАНТИРОВАННО БЕЗ РАМОК
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame); // Убирает дефолтную рамку Qt
    scroll->setStyleSheet("background: transparent; border: none;"); // Убирает серый фон и кант
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scroll->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical { width: 4px; background: transparent; } "
        "QScrollBar::handle:vertical { background: #EEE; border-radius: 2px; } "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        );

    QWidget *scrollWidget = new QWidget();
    scrollWidget->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setContentsMargins(0, 0, 8, 0);
    scrollLayout->setSpacing(12);

    // Функция добавления характеристик
    auto addRow = [&](const QString &name, const QString &val, const QString &desc) {
        if (val == "—" || val.isEmpty()) return;

        QFrame *line = new QFrame();
        line->setFixedHeight(1);
        line->setStyleSheet("background-color: #F8F8F8; border: none;"); // Очень бледная линия-разделитель
        scrollLayout->addWidget(line);

        QHBoxLayout *row = new QHBoxLayout();
        QLabel *titleLbl = new QLabel(name);
        titleLbl->setStyleSheet("color: #777; font-family: 'Century Gothic'; font-size: 15px; border: none;");

        QLabel *dataLbl = new QLabel(QString("<b>%1</b><br><span style='color:#999; font-size:10px;'>%2</span>").arg(val, desc));
        dataLbl->setAlignment(Qt::AlignRight);
        dataLbl->setStyleSheet("font-family: 'Century Gothic'; border: none;");
        dataLbl->setWordWrap(true);

        row->addWidget(titleLbl);
        row->addStretch();
        row->addWidget(dataLbl, 2);
        scrollLayout->addLayout(row);
    };

    addRow("Гармония", analysis.harmony, analysis.harmonyDesc);
    addRow("Совместимость", analysis.compatibility, analysis.compatibilityDesc);
    addRow("Стиль", analysis.style, analysis.styleDesc);
    addRow("Стойкость", analysis.life, analysis.lifeDesc);
    addRow("Аромат", analysis.scent, analysis.scentDesc);
    addRow("Безопасность", analysis.tox, analysis.toxDesc);

    scroll->setWidget(scrollWidget);
    layout->addWidget(scroll);

    // Кнопки
    QHBoxLayout *btns = new QHBoxLayout();
    btns->setSpacing(12);

    QPushButton *cancel = new QPushButton("Отмена", this);
    cancel->setCursor(Qt::PointingHandCursor);
    cancel->setStyleSheet("QPushButton { background: #F9F9F9; border-radius: 20px; height: 45px; color: #999; font-family: 'Century Gothic'; border: none; } QPushButton:hover { background: #F2F2F2; }");

    QPushButton *save = new QPushButton("СОХРАНИТЬ", this);
    save->setCursor(Qt::PointingHandCursor);
    save->setStyleSheet("QPushButton { background: #F8C3CD; border-radius: 20px; height: 45px; color: #333; font-weight: bold; font-family: 'Century Gothic'; border: none; } QPushButton:hover { background: #F5A9B8; }");

    btns->addWidget(cancel);
    btns->addWidget(save);
    layout->addLayout(btns);

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(save, &QPushButton::clicked, this, &QDialog::accept);
}