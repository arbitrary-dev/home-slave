#ifndef TASKSMODEL_H
#define TASKSMODEL_H

#include <QtSql>

typedef int PersonId;
struct Person {
    PersonId id;
    QString name;

    operator QVariant() const { return name; }
};

inline bool operator ==(const Person &a, const Person &b) {
    return a.id == b.id;
}

inline uint qHash(const Person &p, uint seed) {
    return qHash(p.id, seed);
}

struct Esteem {
    int val;
    bool tkn; // taken

    operator QVariant() const { return QVariant::fromValue(*this); }
};

inline bool operator ==(const Esteem &a, const Esteem &b) {
    return a.val == b.val && a.tkn == b.tkn;
}

Q_DECLARE_METATYPE(Esteem)

typedef QHash<PersonId, Esteem> Esteems; // WARN why PersonId instead of just Person?

typedef int TaskId;
struct Task {
    TaskId id;
    QString name;
    Esteems esteems;

    operator QVariant() const { return name; }
};

class TasksModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static const char *STR_NO_ESTEEM;
    static const char *STR_INS_NEW_TASK;

    enum Stage {
        ST_INPUT_ESTEEMS,
        ST_TAKE_TASKS
    };

    explicit TasksModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void toggleStage();
    bool isStage(Stage stage) const { return currStage == stage; }

    Task &task(int row);
    const Task &task(int row) const;

    Person &person(int col);
    const Person &person(int col) const;

private:
    Stage currStage = ST_INPUT_ESTEEMS;

    QVector<Person> vpeople; // TODO !! rfct to QHash<int, Person> (warum?)
    QVector<Task> vdata;

    void initPeople();
    void initData();

    void addTaskRow(const QModelIndex &index, const QString &name);
    void delTaskRow(const QModelIndex &index);

    inline bool inEsteems(const QModelIndex &index) const;
    inline double calcAvgEsteem(const QList<Esteem> &es) const;
};

#endif // TASKSMODEL_H
