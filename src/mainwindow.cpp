#include "mainwindow.h"
#include "tasksmodel.h"
#include "tasksdelegate.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QTableView *t = ui->tableView;

    TasksModel *m = new TasksModel;
    t->setModel(m);

    QHeaderView *hh = t->horizontalHeader();
    hh->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);

    t->setShowGrid(false);
    t->setItemDelegate(new TasksDelegate);
    t->setEditTriggers(QAbstractItemView::DoubleClicked);
    t->setSelectionMode(QAbstractItemView::NoSelection);

    refreshTable();

    t->show();
}

MainWindow::~MainWindow()
{
    QTableView *t = ui->tableView;

    delete t->itemDelegate();
    t->setItemDelegate(NULL);

    delete ui;
}

void MainWindow::toggleStage()
{
    QTableView *t = ui->tableView;
    TasksModel *m = dynamic_cast<TasksModel*>(t->model());

    m->toggleStage();
    refreshTable();
}

// TODO test
void MainWindow::refreshTable()
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
            if (i.data().canConvert<Esteem>())
                (t->*peFunc)(i);
        }

    // Merge cols in an input new task row.
    if (s_input)
        t->setSpan(rc - 1, 0, 1, cc);
}
