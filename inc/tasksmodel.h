#ifndef TASKSMODEL_H
#define TASKSMODEL_H

#include <QSqlQueryModel>

class TasksModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    explicit TasksModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
};

#endif // TASKSMODEL_H
