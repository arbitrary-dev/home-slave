#ifndef TASKSMODEL_H
#define TASKSMODEL_H

#include <QtSql>

struct Person {
    uint id;
    QString name;

    operator QVariant() const { return name; }
};

struct Task {
    uint id;
    QString name;

    operator QVariant() const { return name; }
};

struct Esteem {
    int val;
    bool tkn; // taken
};

typedef QHash<Person, Esteem> Esteems;

struct Row {
    Task task;
    Esteems esteems;
};

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
    QVector<Person> vpeople;
    QVector<Row> vdata;

    void initPeople();
    void initData();
};

inline bool operator ==(const Person &a, const Person &b) {
    return a.id == b.id;
}

inline uint qHash(const Person &p, uint seed) {
    return qHash(p.id, seed);
}

#endif // TASKSMODEL_H
