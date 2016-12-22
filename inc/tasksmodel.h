#ifndef TASKSMODEL_H
#define TASKSMODEL_H

#include <QtSql>

struct Person {
    int id;
    QString name;

    operator QVariant() const { return name; }
};

struct Task {
    int id;
    QString name;

    operator QVariant() const { return name; }
};

struct Esteem {
    int val;
    bool tkn; // taken

    operator QVariant() const { return QVariant::fromValue(*this); }
};

Q_DECLARE_METATYPE(Esteem)

typedef QHash<Person, Esteem> Esteems;

// TODO replace with Task
struct Row {
    Task task;
    Esteems esteems;
};

class TasksModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static const char *STR_NO_ESTEEM;

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

private:
    Stage currStage = ST_INPUT_ESTEEMS;

    QVector<Person> vpeople; // TODO rfct to QHash<int, Person>
    QVector<Row> vdata;

    void initPeople();
    void initData();

    void addTaskRow(const QModelIndex &index, const QString &name);

    inline bool inEsteems(const QModelIndex &index) const;
    inline double calcAvgEsteem(QList<Esteem> &es) const;
};

inline bool operator ==(const Person &a, const Person &b) {
    return a.id == b.id;
}

inline uint qHash(const Person &p, uint seed) {
    return qHash(p.id, seed);
}

#endif // TASKSMODEL_H
