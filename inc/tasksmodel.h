#ifndef TASKSMODEL_H
#define TASKSMODEL_H

#include <QtSql>

class TasksModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TasksModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    typedef QString Person;
    typedef QString Task;

    struct Esteem {
        float val;
        bool tkn; // taken
    };

    typedef QHash<Person, Esteem> Esteems;

    struct Row {
        Task task;
        Esteems esteems;
    };

    QVector<Person> vpeople;
    QVector<Row> vdata;

    void initPeople();
    void initData();
};

#endif // TASKSMODEL_H
