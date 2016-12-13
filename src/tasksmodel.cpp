#include "tasksmodel.h"

TasksModel::TasksModel(QObject *parent)
    : QSqlQueryModel(parent)
{
    setQuery("SELECT name FROM tasks");
}

int TasksModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return QSqlQueryModel::rowCount(parent) + 1;
}

int TasksModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return QSqlQueryModel::columnCount(parent) + 3;
}

QVariant TasksModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();

    if (    row    <  rowCount() - 1
         && column == 0)
        return QSqlQueryModel::data(index, role);

    if (role == Qt::DisplayRole)
        return QString("%1, %2").arg(column + 1)
                                .arg(row + 1);

    return QVariant();
}

QVariant TasksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!(role == Qt::DisplayRole && orientation == Qt::Horizontal))
        return QVariant();

    if (section == 0) {
        return tr("Name");
    } else if (section == columnCount() - 1) {
        return tr("Rate");
    }

    return tr("Pers%1").arg(section);
}
