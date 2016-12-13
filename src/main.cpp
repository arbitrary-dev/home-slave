#include "mainwindow.h"
#include <QApplication>
#include <QtSql>
#include <QDebug>

void initDb();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    initDb();

    MainWindow w;
    w.show();

    return a.exec();
}

void initDb()
{
    // TODO check for DB existence, initialize tables otherwise.
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("tasks.db");

    if (db.open()) {
        QSqlQuery q;

        int p = -1, t = -1;

        if (q.exec("SELECT count(*) FROM people")) {
            q.first();
            p = q.value(0).toInt();
        } else {
            qDebug() << "ERR: Query failed\n" << q.lastError();
        }

        if (q.exec("SELECT count(*) FROM tasks")) {
            q.first();
            t = q.value(0).toInt();
        } else {
            qDebug() << "ERR: Query failed\n" << q.lastError();
        }

        qDebug("OK: Connected to DB\n%d people and %d tasks registered", p, t);
    } else {
        qDebug() << "ERR: Connection to DB failed\n%s" << db.lastError();
    }
}
