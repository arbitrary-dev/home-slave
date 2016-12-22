#include "tasksmodel.h"

#include <numeric>

TasksModel::TasksModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    initPeople();
    initData();
}

// TODO rfct to setPeople(QList<Person>)
void TasksModel::initPeople()
{
    QSqlQuery q("SELECT rowid, name FROM people");
    int iid = q.record().indexOf("rowid");
    int iname = q.record().indexOf("name");

    while (q.next()) {
        int id = q.value(iid).toInt();
        QString name = q.value(iname).toString();
        Person person = { id, name };
        vpeople.push_back(person);
    }
}

// TODO rfct to setData(QList<Task>)
void TasksModel::initData()
{
    QSqlQuery q("SELECT rowid, name FROM tasks ORDER BY name");
    int iid = q.record().indexOf("rowid");
    int iname = q.record().indexOf("name");

    QSqlQuery qe;
    qe.prepare("SELECT * FROM esteems WHERE task = :t AND person = :p");

    while (q.next()) {
        int id = q.value(iid).toInt();
        QString name = q.value(iname).toString();
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

    bool s_input = isStage(ST_INPUT_ESTEEMS);

    return vdata.size()
           + (s_input ? 1 : 0); // new task row
}

int TasksModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 1 // task name
           + vpeople.size()
           + 1; // total esteem
}

// TODO test
Qt::ItemFlags TasksModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags fs = QAbstractTableModel::flags(index);

    if ((isStage(ST_INPUT_ESTEEMS) && index.column() == 0) || inEsteems(index))
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
double TasksModel::calcAvgEsteem(QList<Esteem> &es) const
{
    typedef double (*Func) (double a, Esteem b);
    Func op = [](double a, Esteem b) {
        return (a + b.val) / 2;
    };

    double res((*es.begin()).val);
    res = std::accumulate(es.begin() + 1, es.end(), res, op);

    return res;
}

const char *TasksModel::STR_NO_ESTEEM = "--";

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

    // TODO rfct to switch
    if (r < rc - (s_input ? 1 : 0)) {
        if (inEsteems(index)) {
            Esteem est = vdata[r].esteems[vpeople[c - 1]];
            if (r_disp) {
                if (est.val == 0)
                    return STR_NO_ESTEEM;
                return s_input ? QVariant::fromValue(est.val) : est;
            }
            if (r_edit)
                return est;
            if (r_algn)
                return Qt::AlignCenter;
        }

        if (c == 0 && (r_disp || r_edit))
            return vdata[r].task;

        if (c == cc - 1) {
            if (r_disp) {
                QList<Esteem> es = vdata[r].esteems.values();

                if (es.isEmpty())
                    return STR_NO_ESTEEM;

                double avg = calcAvgEsteem(es);
                return QString::number(avg, 'f', 1);
            }
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

    int r = index.row();
    int c = index.column();

    if (inEsteems(index)) {
        vdata[r].esteems[vpeople[c - 1]] = qvariant_cast<Esteem>(value);

        emit dataChanged(index, index);

        // refresh avg. column
        QModelIndex idx = createIndex(r, columnCount() - 1);
        emit dataChanged(idx, idx);

        return true;
    }

    if (c == 0) {
        if (isStage(ST_INPUT_ESTEEMS) && r == rowCount() - 1) {
            QString name = qvariant_cast<QString>(value);
            addTaskRow(index, name);
            return true;
        }

        vdata[r].task.name = qvariant_cast<QString>(value);
        return true;
    }

    return false;
}

void TasksModel::addTaskRow(const QModelIndex &index, const QString &name)
{
    if (name.trimmed().isEmpty())
        return;

    int r = index.row();
    emit beginInsertRows(index.parent(), r, r);

    Task task = { -1, name };
    Row row = { task, Esteems() };
    vdata.append(row);

    emit endInsertRows();
}

// TODO test
QVariant TasksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    bool o_horz = orientation == Qt::Horizontal;

    switch (role) {
    case Qt::DisplayRole:
        if (!o_horz) break;

        if (section == 0)
            return tr("Task");
        if (section == columnCount() - 1)
            return tr("Avg.");

        return vpeople[section - 1];

    case Qt::SizeHintRole:
        if (section > 0 && o_horz)
            return QSize(50, 0);
        break;

    case Qt::TextAlignmentRole:
        if (section == 0)
            return Qt::AlignLeft;
        break;
    }

    return QVariant();
}

void TasksModel::toggleStage()
{
    currStage = isStage(ST_INPUT_ESTEEMS) ? ST_TAKE_TASKS : ST_INPUT_ESTEEMS;
}
