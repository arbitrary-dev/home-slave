#include "tasksmodel.h"

TasksModel::TasksModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    initPeople();
    initData();
}

void TasksModel::initPeople()
{
    QSqlQuery q("SELECT * FROM people");
    int iName = q.record().indexOf("name");

    while (q.next())
        vpeople.push_back(q.value(iName).toString());
}

void TasksModel::initData()
{
    QSqlQuery q("SELECT name FROM tasks ORDER BY name");
    int iName = q.record().indexOf("name");

    while (q.next()) {
        Task task(q.value(iName).toString());

        Esteems ests;

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
    int column = index.column();

    if (role == Qt::DisplayRole)
    {
        if (    row < rowCount() - 1
                && column == 0)
            return vdata[row].task;

        return QString("%1, %2").arg(column + 1)
                                .arg(row + 1);
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
