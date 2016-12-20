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

Qt::ItemFlags TasksModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags fs = QAbstractTableModel::flags(index);

    if (index.column() == 0 || inEsteems(index))
        fs |= Qt::ItemIsEditable;

    return fs;
}

inline bool TasksModel::inEsteems(const QModelIndex &index) const
{
    return (isStage(ST_TAKE_TASKS) || index.row() < rowCount() - 1)
           && index.column() > 0
           && index.column() < columnCount() - 1;
}

// TODO test
QVariant TasksModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int r = index.row();
    int rc = rowCount();
    int c = index.column();
    int cc = columnCount();
    bool s_input = isStage(ST_INPUT_ESTEEMS);
    bool r_disp = role == Qt::DisplayRole;
    bool r_edit = role == Qt::EditRole;
    bool r_algn = role == Qt::TextAlignmentRole;

    if (r < rc - (s_input ? 1 : 0)) {
        if (inEsteems(index)) {
            Esteem est = vdata[r].esteems[vpeople[c - 1]];
            if (r_disp)
                return s_input ? QVariant::fromValue(est.val) : est;
            if (r_edit)
                return est;
            if (r_algn)
                return Qt::AlignCenter;
        }

        if (c == 0 && (r_disp || r_edit))
            return vdata[r].task;

        if (c == cc - 1) {
            if (r_disp)
                return QString("-1");
            if (r_algn)
                return Qt::AlignCenter;
        }
    }
    if (s_input && r == rc - 1 && r_disp) {
        return tr("insert new task...");
    }

    return QVariant();
}

bool TasksModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    int row = index.row();
    int col = index.column();

    if (inEsteems(index)) {
        vdata[row].esteems[vpeople[col - 1]] = qvariant_cast<Esteem>(value);
        return true;
    }
    if (col == 0) {
        if (isStage(ST_INPUT_ESTEEMS) && row == rowCount() - 1) {
            // TODO handle new task addition
            return true;
        }

        vdata[row].task.name = qvariant_cast<QString>(value);
        return true;
    }

    return false;
}

// TODO test
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

void TasksModel::toggleStage()
{
    currStage = isStage(ST_INPUT_ESTEEMS) ? ST_TAKE_TASKS : ST_INPUT_ESTEEMS;
}
