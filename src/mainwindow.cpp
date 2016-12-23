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

    QTableView *t = ui->tableView;

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

    // refresh view

    refreshToggleButton();
    refreshView();
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

    qDebug("Task #%d \"%s\" was inserted", rowid, qPrintable(task.name));

    // Add esteems associated with task

    q.prepare("INSERT INTO esteems (task, person, esteem, taken) VALUES (:t, :p, :val, :tkn)");
    q.bindValue(":t", task.id);

    QHashIterator<PersonId, Esteem> i(task.esteems);
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

    // TODO taskDeleted

    qDebug("Task #%d \"%s\" was deleted", task.id, qPrintable(task.name));

    return true;
}

bool MainWindow::taskChanged(const QModelIndex &index, const QModelIndex &, const QVector<int> &)
{
    int col = index.column();
    int row = index.row();
    Task &task = tasksModel->task(row);
    QSqlQuery q;

    if (col == 0) {
        // Task name

        q.prepare("UPDATE tasks SET name = :name WHERE rowid = :id");
        q.bindValue(":id", task.id);
        q.bindValue(":name", task.name);

        if (!q.exec()) {
            qDebug() << "ERR: Query failed\n" << q.lastError();
            return false;
        }
    } else if (index.data(Qt::EditRole).canConvert<Esteem>()) {
        // Esteem

        Esteem e = qvariant_cast<Esteem>(index.data(Qt::EditRole));

        q.prepare("UPDATE esteems SET esteem = :val, taken = :tkn WHERE task = :t AND person = :p");
        q.bindValue(":t", task.id);
        q.bindValue(":p", tasksModel->person(col).id);
        q.bindValue(":val", e.val);
        q.bindValue(":tkn", e.tkn);

        if (!q.exec()) {
            qDebug() << "ERR: Query failed\n" << q.lastError();
            return false;
        }

        if (!ui->btnToggleStage->isEnabled())
            refreshToggleButton();
    }

    qDebug("Task #%d \"%s\" was changed", task.id, qPrintable(task.name));

    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    using Btn = QMessageBox::StandardButton;

    Btn res = QMessageBox::question(
        this, QCoreApplication::applicationName(),
        tr(STR_PERST_CHANGES).arg("tasks.db"), // TODO replace tasks.db with variable
        Btn::Yes | Btn::No | Btn::Cancel);

    switch (res) {
    case Btn::Cancel:
        event->ignore();
        break;

    case Btn::Yes:
        QSqlDatabase::database().commit();
        // fall-through
    case Btn::No:
    default:
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

    // Make checkboxes persistent.
    for (int r = 0; r < rc; ++r)
        for (int c = 0; c < cc; ++c)
        {
            QModelIndex i = m->index(r, c);
            if (i.data(Qt::EditRole).canConvert<Esteem>())
                (t->*peFunc)(i);
        }

    // Merge cols in an input new task row.
    if (s_input)
        t->setSpan(rc - 1, 0, 1, cc);

    // Stage toggle button text update.
    QPushButton *bToggle = ui->btnToggleStage;
    bToggle->setText(tr(s_input ? STR_TAKE_TASKS : STR_INPUT_ESTEEMS));

    // refresh
    QModelIndex start = m->index(0, 0);
    QModelIndex end = m->index(rc, cc);
    emit m->dataChanged(start, end);
}

int MainWindow::colWidth(const QAbstractTableModel *m, int col)
{
    QSize size = qvariant_cast<QSize>(m->headerData(col, Qt::Horizontal, Qt::SizeHintRole));
    return size.width();
}
