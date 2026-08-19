#include "notespanel.h"
#include "notecard.h"
#include "errordialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSqlQuery>
#include <QSqlError>
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QMessageBox>
#include <QDebug>

NotesPanel::NotesPanel(QWidget *parent) : QWidget(parent), m_userId(-1) {
    setupUI();
}

void NotesPanel::setCurrentUser(int userId) {
    m_userId = userId;
    loadNotes();
}

void NotesPanel::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    if (m_userId > 0) loadNotes();
}

void NotesPanel::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 10, 0, 0);
    mainLayout->setSpacing(20);

    // --- ВЕРХНЯЯ ПАНЕЛЬ ---
    QWidget *topBar = new QWidget();
    QHBoxLayout *barLayout = new QHBoxLayout(topBar);
    barLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *title = new QLabel("Мои заметки", this);
    title->setStyleSheet("font-family: 'Century Gothic'; font-size: 24px; font-weight: bold; color: #333;");

    // Создаем счетчик
    m_countLabel = new QLabel("0 заметок", this);
    m_countLabel->setStyleSheet("font-family: 'Century Gothic'; font-size: 14px; color: #999; margin-left: 10px; margin-top: 5px;");

    QPushButton *btnAdd = new QPushButton("Добавить заметку", this);
    btnAdd->setFixedSize(180, 40);
    btnAdd->setCursor(Qt::PointingHandCursor);
    btnAdd->setStyleSheet(
        "QPushButton { border: 1px solid #F8C3CD; border-radius: 12px; color: #D86B7A; font-family: 'Century Gothic'; font-weight: bold; background: white; } "
        "QPushButton:hover { background: #FFF0F3; }"
        );
    connect(btnAdd, &QPushButton::clicked, this, [this]() { openNoteDialog(-1); });

    barLayout->addWidget(title);
    barLayout->addWidget(m_countLabel); // Добавляем счетчик сразу после заголовка
    barLayout->addStretch();
    barLayout->addWidget(btnAdd);
    mainLayout->addWidget(topBar);

    // --- СЕТКА ЗАМЕТОК ---
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background: transparent; border: none;");

    QWidget *container = new QWidget();
    container->setStyleSheet("background: transparent;");

    m_grid = new QGridLayout(container);
    m_grid->setSpacing(25);
    m_grid->setContentsMargins(0, 10, 0, 0);
    m_grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    scrollArea->setWidget(container);
    mainLayout->addWidget(scrollArea, 1);
}

void NotesPanel::loadNotes() {
    if (m_userId <= 0) return;

    QLayoutItem *item;
    while ((item = m_grid->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    QSqlQuery q;
    q.prepare("SELECT note_id, title, content FROM UserNotes WHERE user_id = :uid ORDER BY note_id DESC");
    q.bindValue(":uid", m_userId);

    int count = 0;
    if (q.exec()) {
        int row = 0, col = 0;
        while (q.next()) {
            int noteId = q.value(0).toInt();
            QString title = q.value(1).toString();
            QString content = q.value(2).toString();

            NoteCard *card = new NoteCard(noteId, title, content, this);

            connect(card, &NoteCard::editRequested, this, &NotesPanel::openNoteDialog);
            connect(card, &NoteCard::deleteRequested, this, &NotesPanel::deleteNote);

            m_grid->addWidget(card, row, col);
            if (++col > 1) { col = 0; row++; }
            count++;
        }
    }

    if (count == 0) {
        QLabel *empty = new QLabel("Заметок пока нет. Создайте первую!", this);
        empty->setStyleSheet("color: #CCC; font-family: 'Century Gothic'; font-size: 18px; padding-top: 100px;");
        m_grid->addWidget(empty, 0, 0, 1, 2, Qt::AlignCenter);
    }

    // Правильные окончания для русского языка (1 заметка, 2 заметки, 5 заметок)
    QString suffix = "заметок";
    if (count % 10 == 1 && count % 100 != 11) suffix = "заметка";
    else if (count % 10 >= 2 && count % 10 <= 4 && (count % 100 < 10 || count % 100 >= 20)) suffix = "заметки";

    m_countLabel->setText(QString("%1 %2").arg(count).arg(suffix));
}

void NotesPanel::openNoteDialog(int noteId) {
    QDialog dialog(this);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog.setAttribute(Qt::WA_TranslucentBackground);
    dialog.setFixedSize(400, 350);

    // 1. Убираем отступы у самого окна
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 2. Создаем контейнер, который будет играть роль "белого окна с углами"
    QFrame *bgFrame = new QFrame(&dialog);
    bgFrame->setStyleSheet("QFrame { background: white; border-radius: 20px; border: 1px solid #CCC; }");

    // 3. Создаем Layout для контента ВНУТРИ этого белого контейнера
    QVBoxLayout *layout = new QVBoxLayout(bgFrame);
    layout->setContentsMargins(25, 25, 25, 25);
    layout->setSpacing(15);

    QLabel *header = new QLabel(noteId == -1 ? "Новая заметка" : "Редактировать заметку", bgFrame);
    header->setStyleSheet("font-family: 'Century Gothic'; font-size: 18px; font-weight: bold; color: #333; border: none; background: transparent;");

    QLineEdit *editTitle = new QLineEdit(bgFrame);
    editTitle->setPlaceholderText("Заголовок");
    editTitle->setFixedHeight(40);
    editTitle->setStyleSheet("QLineEdit { border: 1px solid #EEE; border-radius: 8px; padding: 8px 12px; font-family: 'Century Gothic'; font-size: 14px; background: #F9F9F9; color: #333; } QLineEdit:focus { border-color: #D86B7A; background: white; }");

    QTextEdit *editContent = new QTextEdit(bgFrame);
    editContent->setPlaceholderText("Текст заметки...");
    editContent->setStyleSheet("QTextEdit { border: 1px solid #EEE; border-radius: 8px; padding: 8px 12px; font-family: 'Century Gothic'; font-size: 14px; background: #F9F9F9; color: #333; } QTextEdit:focus { border-color: #D86B7A; background: white; }");

    if (noteId != -1) {
        QSqlQuery q;
        q.prepare("SELECT title, content FROM UserNotes WHERE note_id = :nid");
        q.bindValue(":nid", noteId);
        if (q.exec() && q.next()) {
            editTitle->setText(q.value(0).toString());
            editContent->setPlainText(q.value(1).toString());
        }
    }

    // --- КРАСИВЫЕ КНОПКИ ---
    QHBoxLayout *btns = new QHBoxLayout();
    btns->setSpacing(10);
    btns->addStretch();

    QPushButton *btnCancel = new QPushButton("Отмена", bgFrame);
    btnCancel->setFixedSize(100, 38);
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnCancel->setStyleSheet(
        "QPushButton { "
        "  background: #FFFFFF; border: 1px solid #CCCCCC; border-radius: 19px; "
        "  color: #666666; font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; "
        "} "
        "QPushButton:hover { background: #F5F5F5; border-color: #AAAAAA; color: #333333; }"
        );

    QPushButton *btnSave = new QPushButton("Сохранить", bgFrame);
    btnSave->setFixedSize(110, 38);
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setStyleSheet(
        "QPushButton { "
        "  background: #D86B7A; border: none; border-radius: 19px; "
        "  color: #FFFFFF; font-family: 'Century Gothic'; font-size: 13px; font-weight: bold; "
        "} "
        "QPushButton:hover { background: #E6A8B5; }"
        );

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, &dialog, &QDialog::accept);

    btns->addWidget(btnCancel);
    btns->addWidget(btnSave);

    // Добавляем все элементы во внутренний layout
    layout->addWidget(header);
    layout->addWidget(editTitle);
    layout->addWidget(editContent);
    layout->addLayout(btns);

    // 4. Добавляем белый контейнер в главное прозрачное окно
    mainLayout->addWidget(bgFrame);

    // Темный оверлей на фоне
    QWidget *overlay = new QWidget(this->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->setGeometry(this->window()->rect());
    overlay->show();

    // Запускаем диалог
    if (dialog.exec() == QDialog::Accepted) {
        QString newTitle = editTitle->text().trimmed();
        QString newContent = editContent->toPlainText().trimmed();

        if (newTitle.isEmpty()) newTitle = "Без названия";

        QSqlQuery q;
        if (noteId == -1) { // Создание
            q.prepare("INSERT INTO UserNotes (user_id, title, content) VALUES (:uid, :title, :content)");
            q.bindValue(":uid", m_userId);
        } else { // Обновление
            q.prepare("UPDATE UserNotes SET title = :title, content = :content WHERE note_id = :nid");
            q.bindValue(":nid", noteId);
        }
        q.bindValue(":title", newTitle);
        q.bindValue(":content", newContent);

        if (q.exec()) loadNotes(); // Перезагружаем сетку
    }

    overlay->deleteLater();
}

void NotesPanel::deleteNote(int noteId) {
    // Создаем темный фон поверх главного окна
    QWidget *overlay = new QWidget(this->window());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->setGeometry(this->window()->rect());
    overlay->show();

    // Вызываем твой кастомный диалог
    ConfirmDialog dialog("Удаление", "Вы действительно хотите удалить эту заметку?", this->window());
    dialog.move(this->window()->frameGeometry().center() - dialog.rect().center());

    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery q;
        q.prepare("DELETE FROM UserNotes WHERE note_id = :nid AND user_id = :uid");
        q.bindValue(":nid", noteId);
        q.bindValue(":uid", m_userId);

        if (q.exec()) {
            loadNotes(); // Обновляем список заметок
        }
    }

    overlay->deleteLater();
}