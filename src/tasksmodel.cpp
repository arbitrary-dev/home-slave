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
        Esteems ests;

        QString name = q.value(iname).toString();
        qe.bindValue(":t", id);

        foreach (Person person, vpeople) {
            qe.bindValue(":p", person.id);
            qe.exec();

            if (!qe.first()) continue;

            int ival = qe.record().indexOf("esteem");
            int itkn = qe.record().indexOf("taken");

            Esteem est = { qe.value(ival).toInt(), qe.value(itkn).toBool() };
            ests.insert(person.id, est);
        }

        Task task = { id, name, ests };
        vdata.push_back(task);
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
double TasksModel::calcAvgEsteem(const QList<Esteem> &es) const
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
const char *TasksModel::STR_INS_NEW_TASK = "insert new task...";

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

    switch (role) {
    case Qt::DisplayRole:
        if (r < rc - (s_input ? 1 : 0)) {
            if (inEsteems(index)) {
                Esteem est = vdata[r].esteems[person(c).id];
                if (est.val == 0)
                    return STR_NO_ESTEEM;
                return s_input ? QVariant::fromValue(est.val) : est; // esteem
            }

            if (c == 0)
                return vdata[r]; // task name

            if (c == cc - 1) {
                QList<Esteem> es = vdata[r].esteems.values();

                if (es.isEmpty())
                    return STR_NO_ESTEEM;

                double avg = calcAvgEsteem(es);
                return QString::number(avg, 'f', 1); // avg. column
            }
        }

        if (s_input && r == rc - 1)
            return tr(STR_INS_NEW_TASK); // insert new task row

        break;

    case Qt::EditRole:
        if (r < rc - (s_input ? 1 : 0)) {
            if (c == 0)
                return vdata[r]; // task name

            if (inEsteems(index)) // esteem
                return vdata[r].esteems[person(c).id];
        }

        break;

    case Qt::TextAlignmentRole:
        if (r < rc - (s_input ? 1 : 0) && (inEsteems(index) || c == cc - 1)) // esteem & avg
            return Qt::AlignCenter;

        break;

    case Qt::FontRole:
        if (s_input && r == rc - 1) { // insert new task row
            QFont f;
            f.setItalic(true);
            return f;
        }

        break;

    case Qt::ForegroundRole:
        if (s_input && r == rc - 1)
            return QColor(Qt::lightGray); // insert new task row

        break;
    }

    return QVariant();
}

bool TasksModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    int r = index.row();
    int c = index.column();
    bool s_input = isStage(ST_INPUT_ESTEEMS);

    if (inEsteems(index)) {
        vdata[r].esteems[person(c).id] = qvariant_cast<Esteem>(value);
        emit dataChanged(index, index);

        if (s_input) {
            // refresh avg. column
            QModelIndex idx = createIndex(r, columnCount() - 1);
            emit dataChanged(idx, idx);
        } else {
            // TODO update statusbar on current work distribution
        }

        return true;
    }

    if (c == 0) {
        QString name = qvariant_cast<QString>(value);

        if (s_input && r == rowCount() - 1) { // add task row
            addTaskRow(index, name);
            return true;
        }

        if (name.trimmed().isEmpty()) { // delete task row, if task name was erased
            delTaskRow(index);
            return true;
        }

        vdata[r].name = name;
        emit dataChanged(index, index);
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

    Task task = { -1, name, Esteems() };
    vdata.append(task);

    emit endInsertRows();
}

void TasksModel::delTaskRow(const QModelIndex &index)
{
    int r = index.row();
    emit beginRemoveRows(index.parent(), r, r);
    vdata.removeAt(r);
    emit endRemoveRows();
}

// TODO test
// TODO context menu to add, rename and delete people
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

        return person(section);

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

Task &TasksModel::task(int row)
{
    return vdata[row];
}

const Task &TasksModel::task(int row) const
{
    return vdata[row];
}

Person &TasksModel::person(int col)
{
    return vpeople[col - 1];
}

const Person &TasksModel::person(int col) const
{
    return vpeople[col - 1];
}
