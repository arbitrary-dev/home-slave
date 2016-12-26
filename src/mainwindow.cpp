#include "mainwindow.h"

#include "tasksdelegate.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QCloseEvent>
#include <QMessageBox>

const char *MainWindow::STR_INVAL_ESTEEMS = "In order to proceed, all esteems should be valid.";
const char *MainWindow::STR_INPUT_ESTEEMS = "Input esteems";
const char *MainWindow::STR_TAKE_TASKS = "Take tasks!";
const char *MainWindow::STR_PERST_CHANGES = "Persist changes to %1?";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // TODO remove left border from table viewport
    // TODO shade even rows
    QTableView *t = ui->tableView;
    // TODO ! separator between data & summary row
    // TODO ! locked summary row
    // https://goo.gl/sQe84X
    // https://goo.gl/cjpvEC
    // QAbstractScrollArea::setViewportMargins()

    tasksModel = new TasksModel;
    TasksModel *m = tasksModel;

    t->setModel(m);
    t->setShowGrid(false);
    t->setItemDelegate(new TasksDelegate);
    t->setEditTriggers(QAbstractItemView::DoubleClicked);
    t->setSelectionMode(QAbstractItemView::NoSelection);

    // columns' width
    QHeaderView *hh = t->horizontalHeader();
    hh->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
    for (int i = 1; i < m->columnCount(); ++i)
        t->setColumnWidth(i, colWidth(m, i));

    // stage toggle button
    QPushButton *bToggle = ui->btnToggleStage;
    connect(bToggle, &QPushButton::clicked, this, &MainWindow::toggleStage);

    // handle rows events

    connect(m, &TasksModel::rowsInserted, this, &MainWindow::taskAdded);

    connect(m, &TasksModel::rowsAboutToBeRemoved, this, &MainWindow::taskDeleted);
    connect(m, &TasksModel::rowsRemoved, this,
            [this, bToggle] { if (!bToggle->isEnabled()) refreshToggleButton(); });

    connect(m, &TasksModel::dataChanged, this, &MainWindow::taskChanged);
    // TODO ! update statusbar on current work distribution

    // refresh view

    refreshToggleButton();
    refreshView();

    // TODO ! send welcome! to statusbar
    // TODO columns sorting
}

MainWindow::~MainWindow()
{
    QTableView *t = ui->tableView;

    delete t->itemDelegate();
    t->setItemDelegate(nullptr);

    delete ui;
}

void MainWindow::toggleStage()
{
    QTableView *t = ui->tableView;
    TasksModel *m = dynamic_cast<TasksModel*>(t->model());

    m->toggleStage();
    refreshView();
}

void MainWindow::refreshToggleButton()
{
    QTableView *t = ui->tableView;
    TasksModel *m = dynamic_cast<TasksModel*>(t->model());

    int rc = m->rowCount();
    int cc = m->columnCount();

    // Check if there's any missing esteem.
    for (int r = 0; r < rc; ++r)
        for (int c = 0; c < cc; ++c)
        {
            QModelIndex i(m->index(r, c));
            QVariant qvar = i.data(Qt::EditRole);

            if (!qvar.canConvert<Esteem>())
                continue;

            Esteem est(qvariant_cast<Esteem>(qvar));

            if (est.val > 0)
                continue;

            // Disable toggle button 'till all esteems are valid.
            disableToggleButton(true);

            return;
        }

    // All esteems are valid.
    disableToggleButton(false);
}

bool MainWindow::taskAdded(const QModelIndex &, int row, int)
{
    Task &task = tasksModel->task(row);

    QSqlDatabase::database().transaction(); // start one if not already
    QSqlQuery q;
    q.prepare("INSERT INTO tasks (name) VALUES (:name)");
    q.bindValue(":name", task.name);

    if (!q.exec()) {
        qDebug() << "ERR: Query failed\n" << q.lastError();
        return false;
    }

    q.exec("SELECT last_insert_rowid()");
    q.first();
    int rowid = q.value(0).toInt();
    task.id = rowid;

    // TODO convert to statusbar update
    qDebug("Task #%d \"%s\" was inserted", rowid, qPrintable(task.name));

    // Add esteems associated with task

    q.prepare("INSERT INTO esteems (task, person, esteem, taken) VALUES (:t, :p, :val, :tkn)");
    q.bindValue(":t", task.id);

    QHashIterator<Person, Esteem> i(task.esteems);
    while (i.hasNext()) {
        i.next();
        q.bindValue(":p", i.key());

        {
            const Esteem &e = i.value();
            q.bindValue(":val", e.val);
            q.bindValue(":tkn", e.tkn);
        }

        if (!q.exec()) {
            qDebug() << "ERR: Query failed\n" << q.lastError();
            return false;
        }
    }

    disableToggleButton(true);

    return true;
}

bool MainWindow::taskDeleted(const QModelIndex &, int row, int)
{
    Task &task = tasksModel->task(row);

    QSqlDatabase::database().transaction(); // start one if not already
    QSqlQuery q;

    // Delete esteems associated with task

    q.prepare("DELETE FROM esteems WHERE task = :t");
    q.bindValue(":t", task.id);

    if (!q.exec()) {
        qDebug("ERR: \"%s\" failed\n%s", qPrintable(q.executedQuery()), qPrintable(q.lastError().text()));
        return false;
    }

    // Delete the task itself

    q.prepare("DELETE FROM tasks WHERE rowid = :id");
    q.bindValue(":id", task.id);

    if (!q.exec()) {
        qDebug("ERR: \"%s\" failed\n%s", qPrintable(q.executedQuery()), qPrintable(q.lastError().text()));
        return false;
    }

    // TODO convert to statusbar update
    qDebug("Task #%d \"%s\" was deleted", task.id, qPrintable(task.name));

    return true;
}

bool MainWindow::taskChanged(const QModelIndex &index, const QModelIndex &, const QVector<int> &)
{
    bool res = false;
    int c = index.column();
    int r = index.row();
    int rc = tasksModel->rowCount();

    // Check if row is valid
    if (r == rc - 1)
        return false;

    Task &task = tasksModel->task(r);

    QSqlDatabase::database().transaction(); // start one if not already
    QSqlQuery q;

    if (c == 0) {
        // Task name

        q.prepare("UPDATE tasks SET name = :name WHERE rowid = :id");
        q.bindValue(":id", task.id);
        q.bindValue(":name", task.name);

        if (!q.exec()) {
            qDebug() << "ERR: Query failed\n" << q.lastError();
            return false;
        }

        res = true;
    } else if (index.data(Qt::EditRole).canConvert<Esteem>()) {
        // Esteem

        Esteem e = qvariant_cast<Esteem>(index.data(Qt::EditRole));

        q.prepare("REPLACE INTO esteems (task, person, esteem, taken) VALUES (:t, :p, :val, :tkn)");
        q.bindValue(":t", task.id);
        q.bindValue(":p", tasksModel->person(c).id);
        q.bindValue(":val", e.val);
        q.bindValue(":tkn", e.tkn);

        if (!q.exec()) {
            qDebug() << "ERR: Query failed\n" << q.lastError();
            return false;
        }

        if (!ui->btnToggleStage->isEnabled())
            refreshToggleButton();

        res = true;
    }

    // TODO convert to statusbar update
    if (res) qDebug("Task #%d \"%s\" was changed", task.id, qPrintable(task.name));

    return res;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (db.transaction()) {
        // If can acquire transaction, then it wasn't started before, so no changes to persist.
        db.rollback();
        db.close();
        event->accept();
        return;
    }

    using Btn = QMessageBox::StandardButton;

    Btn res = QMessageBox::question(
        this, QCoreApplication::applicationName(),
        tr(STR_PERST_CHANGES).arg("tasks.db"), // TODO !! replace tasks.db with variable
        Btn::Yes | Btn::No | Btn::Cancel);

    switch (res) {
    case Btn::Cancel:
        event->ignore();
        break;

    case Btn::Yes:
        db.commit();
        // fall-through
    case Btn::No:
    default:
        db.close();
        event->accept();
        break;
    }
}

void MainWindow::disableToggleButton(bool disable)
{
    QPushButton *btn = ui->btnToggleStage;
    btn->setDisabled(disable);
    btn->setToolTip(disable ? tr(STR_INVAL_ESTEEMS) : "");
}

// TODO test
void MainWindow::refreshView()
{
    QTableView *t = ui->tableView;
    TasksModel *m = dynamic_cast<TasksModel*>(t->model());
    bool s_input = m->isStage(TasksModel::ST_INPUT_ESTEEMS);

    typedef void (QAbstractItemView::*Func) (const QModelIndex &);
    Func peFunc = s_input
        ? &QAbstractItemView::closePersistentEditor
        : &QAbstractItemView::openPersistentEditor;

    int rc = m->rowCount();
    int cc = m->columnCount();

    // Make checkboxes persistent
    for (int r = 0; r < rc; ++r)
        for (int c = 0; c < cc; ++c)
        {
            QModelIndex i = m->index(r, c);
            if (i.data(Qt::EditRole).canConvert<Esteem>())
                (t->*peFunc)(i);
        }

    // Merge cols in an input new task row
    t->setSpan(rc - 1, 0, 1, s_input ? cc : 1);

    // Stage toggle button text update.
    QPushButton *bToggle = ui->btnToggleStage;
    bToggle->setText(tr(s_input ? STR_TAKE_TASKS : STR_INPUT_ESTEEMS));
}

int MainWindow::colWidth(const QAbstractTableModel *m, int col)
{
    QSize size = qvariant_cast<QSize>(m->headerData(col, Qt::Horizontal, Qt::SizeHintRole));
    return size.width();
}
