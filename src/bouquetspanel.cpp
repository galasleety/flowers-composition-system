#include "bouquetspanel.h"
#include "bouquetcard.h"
#include "errordialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QPushButton>
#include <QScrollBar>

#include "bouquetspanel.h"
#include "bouquetcard.h"
#include "bouquetdetailsdialog.h" // Обязательно подключи это!
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QPushButton>
#include <QScrollBar>

void BouquetsPanel::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 10, 0, 0);
    mainLayout->setSpacing(20);

    // --- ВЕРХНЯЯ ПАНЕЛЬ ---
    QWidget *topBar = new QWidget();
    QHBoxLayout *barLayout = new QHBoxLayout(topBar);
    barLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *title = new QLabel("Мои букеты", this);
    title->setStyleSheet("font-family: 'Century Gothic'; font-size: 26px; font-weight: bold; color: #333;");

    m_countLabel = new QLabel("0 букетов", this);
    m_countLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 14px; color: #999; margin-top: 10px;");

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Найти букет...");
    m_searchEdit->setFixedSize(220, 40);
    m_searchEdit->setStyleSheet("QLineEdit { border: 1px solid #EEE; border-radius: 12px; padding-left: 15px; background: #F9F9F9; font-family: 'Century Gothic'; }");

    // Лямбда для поиска
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString &) {
        loadBouquets();
    });

    m_sortCombo = new QComboBox();
    m_sortCombo->addItems({"Сначала новые", "По названию"});
    m_sortCombo->setFixedSize(180, 40);
    m_sortCombo->setStyleSheet(
        "QComboBox { border: 1px solid #EEE; border-radius: 12px; padding-left: 15px; background: #F9F9F9; font-family: 'Century Gothic'; } "
        "QComboBox::drop-down { border: none; width: 0px; } "
        "QComboBox::down-arrow { image: none; }"
        );

    // Лямбда для сортировки
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        loadBouquets();
    });

    QPushButton *btnToConstructor = new QPushButton("В конструктор", this);
    btnToConstructor->setFixedSize(160, 40);
    btnToConstructor->setCursor(Qt::PointingHandCursor);
    btnToConstructor->setStyleSheet(
        "QPushButton { border: 1px solid #F8C3CD; border-radius: 12px; color: #D86B7A; font-family: 'Century Gothic'; font-weight: bold; background: white; } "
        "QPushButton:hover { background: #FFF0F3; }"
        );
    connect(btnToConstructor, &QPushButton::clicked, this, [this]() {
        emit goToConstructorRequested();
    });

    barLayout->addWidget(title);
    barLayout->addWidget(m_countLabel);
    barLayout->addStretch();
    barLayout->addWidget(m_searchEdit);
    barLayout->addWidget(m_sortCombo);
    barLayout->addWidget(btnToConstructor);
    mainLayout->addWidget(topBar);

    // --- ОБЛАСТЬ ПРОКРУТКИ И СЕТКА ---
    QScrollArea *scrollArea = new QScrollArea(this); // ИСПРАВЛЕНА ПЕРЕМЕННАЯ
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QString scrollStyle =
        "QScrollBar:vertical { border: none; background: #F5F5F5; width: 8px; margin: 0px; border-radius: 4px; } "
        "QScrollBar::handle:vertical { background: #BDBDBD; border-radius: 4px; min-height: 25px; } "
        "QScrollBar::handle:vertical:hover { background: #A0A0A0; } "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; } "
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }";

    scrollArea->verticalScrollBar()->setStyleSheet(scrollStyle);

    QWidget *container = new QWidget();
    m_grid = new QGridLayout(container);
    m_grid->setSpacing(25);
    m_grid->setContentsMargins(0, 10, 0, 0);
    m_grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    scrollArea->setWidget(container);
    mainLayout->addWidget(scrollArea, 1);
}

void BouquetsPanel::loadBouquets() {
    if (m_userId <= 0) return;

    QLayoutItem *item;
    while ((item = m_grid->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    QString searchTxt = m_searchEdit->text().trimmed().toLower();

    QString queryStr = "SELECT bouquet_id, bouquet_name, description, image_data "
                       "FROM SavedBouquets WHERE user_id = :uid ";

    if (m_sortCombo->currentIndex() == 0) {
        queryStr += "ORDER BY bouquet_id DESC";
    } else {
        queryStr += "ORDER BY bouquet_name ASC";
    }

    QSqlQuery q;
    q.prepare(queryStr);
    q.bindValue(":uid", m_userId);

    int count = 0;
    if (q.exec()) {
        int row = 0, col = 0;
        while (q.next()) {
            QString bName = q.value(1).toString();

            if (!searchTxt.isEmpty() && !bName.toLower().contains(searchTxt)) {
                continue;
            }

            BouquetCard *card = new BouquetCard(
                q.value(0).toInt(),
                bName,
                q.value(2).toString(),
                q.value(3).toByteArray(),
                this
                );

            // 1. Коннект для окна "Подробнее"
            connect(card, &BouquetCard::detailsRequested, this, [this](int id) {
                QWidget *overlay = new QWidget(this->window());
                overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
                overlay->setGeometry(this->window()->rect());
                overlay->show();

                BouquetDetailsDialog dialog(id, this->window());
                dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());
                dialog.exec();
                overlay->deleteLater();
            });

            // --- ВОТ ЭТОТ БЛОК НУЖНО ДОБАВИТЬ ---
            // 2. Коннект для кнопки удаления
            connect(card, &BouquetCard::deleteRequested, this, &BouquetsPanel::deleteBouquet);
            // ------------------------------------

            m_grid->addWidget(card, row, col);
            if (++col > 3) { col = 0; row++; }
            count++;
        }
    }

    if (count == 0) {
        QLabel *empty = new QLabel("Букеты не найдены", this);
        empty->setStyleSheet("color: #CCC; font-family: 'Century Gothic'; font-size: 18px; padding-top: 100px;");
        m_grid->addWidget(empty, 0, 0, 1, 4, Qt::AlignCenter);
    }

    m_countLabel->setText(QString("%1 букет%2").arg(count).arg(count == 1 ? "" : "а"));
}

BouquetsPanel::BouquetsPanel(QWidget *parent) : QWidget(parent), m_userId(-1) {
    setupUI();
}

void BouquetsPanel::setCurrentUser(int userId) {
    m_userId = userId;
    loadBouquets();
}

void BouquetsPanel::showEvent(QShowEvent *event) {
    QWidget::showEvent(event); // Вызываем базовый метод

    // Как только вкладка показалась на экране — запрашиваем свежие данные из БД
    if (m_userId > 0) {
        loadBouquets();
    }
}

void BouquetsPanel::deleteBouquet(int id) {
    // Создаем темный фон поверх главного окна
    QWidget *overlay = new QWidget(this->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->setGeometry(this->window()->rect());
    overlay->show();

    // Вызываем твой кастомный диалог
    ConfirmDialog dialog("Удаление", "Вы уверены, что хотите удалить этот букет?", this->window());
    dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());

    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery q;
        q.prepare("DELETE FROM SavedBouquets WHERE bouquet_id = :bid");
        q.bindValue(":bid", id);

        if (q.exec()) {
            loadBouquets(); // Перезагружаем сетку после успешного удаления
        } else {
            qDebug() << "Ошибка удаления букета:" << q.lastError().text();
        }
    }

    overlay->deleteLater();
}
