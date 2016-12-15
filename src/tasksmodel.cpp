#include "tasksmodel.h"

TasksModel::TasksModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    initPeople();
    initData();
}

void TasksModel::initPeople()
{
    QSqlQuery q("SELECT rowid, name FROM people");
    int iid = q.record().indexOf("rowid");
    int iname = q.record().indexOf("name");

    while (q.next()) {
        uint id(q.value(iid).toUInt());
        QString name(q.value(iname).toString());
        Person person = { id, name };
        vpeople.push_back(person);
    }
}

void TasksModel::initData()
{
    QSqlQuery q("SELECT rowid, name FROM tasks ORDER BY name");
    int iid = q.record().indexOf("rowid");
    int iname = q.record().indexOf("name");

    QSqlQuery qe;
    qe.prepare("SELECT * FROM esteems WHERE task = :t AND person = :p");

    while (q.next()) {
        uint id(q.value(iid).toUInt());
        QString name(q.value(iname).toString());
        Task task = { id, name };

        Esteems ests;
        qe.bindValue(":t", task.id);
        foreach (Person person, vpeople) {
            qe.bindValue(":p", person.id);
            qe.exec();

            if (!qe.first()) continue;

            static int ival = qe.record().indexOf("esteem");
            static int itkn = qe.record().indexOf("taken");

            Esteem est = { qe.value(ival).toInt(), qe.value(itkn).toBool() };
            ests.insert(person, est);
        }

        Row row = { task, ests };
        vdata.push_back(row);
    }
}

int TasksModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return vdata.size()
           + 1; // new task row
}

int TasksModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 1 // task name
           + vpeople.size()
           + 1; // total esteem
}

QVariant TasksModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();

    if (role == Qt::DisplayRole)
    {
        if (row < rowCount() - 1) {
            if (col == 0)
                return vdata[row].task;
            else if (col == columnCount() - 1)
                return QString("-1");

            return QString::number(vdata[row].esteems[vpeople[col - 1]].val);
        }

        if (col == 0)
            return tr("insert new task...");
    }

    return QVariant();
}

QVariant TasksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!(role == Qt::DisplayRole && orientation == Qt::Horizontal))
        return QVariant();

    if (section == 0)
        return tr("Task");
    else if (section == columnCount() - 1)
        return tr("Total");

    return vpeople[section - 1];
}
