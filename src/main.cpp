#include "mainwindow.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QFile>

bool connectToDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("flowers.db"); // Файл должен лежать в папке сборки или проекта

    if (!db.open()) {
        qDebug() << "Ошибка подключения к БД:" << db.lastError().text();
        return false;
    }
    qDebug() << "База данных успешно подключена!";
    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Подключаем стили
    QFile styleFile(":/styles/main_style.txt");
    if (styleFile.open(QFile::ReadOnly)) {
        a.setStyleSheet(styleFile.readAll());
    }

    // Инициализируем БД
    if (!connectToDatabase()) {
        return -1;
    }

    MainWindow w;
    w.show();

    return a.exec();
}