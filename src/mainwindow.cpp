#include "mainwindow.h"
#include "tasksmodel.h"
#include "tasksdelegate.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    TasksModel *m = new TasksModel;
    QTableView *t = ui->tableView;
    t->setModel(m);

    QHeaderView *hh = t->horizontalHeader();
    hh->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);

    t->setShowGrid(false);

    t->setItemDelegate(new TasksDelegate);
    t->setEditTriggers(QAbstractItemView::SelectedClicked);
    t->setSelectionMode(QAbstractItemView::NoSelection);

    toggleStage(); // FIXME x

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

    typedef void (QAbstractItemView::*Func) (const QModelIndex &);
    Func peFunc = m->isStage(TasksModel::ST_INPUT_ESTEEMS)
        ? &QAbstractItemView::closePersistentEditor
        : &QAbstractItemView::openPersistentEditor;

    int rc = m->rowCount();
    int cc = m->columnCount();

    for (int r = 0; r < rc; ++r)
        for (int c = 0; c < cc; ++c)
        {
            QModelIndex i = m->index(r, c);
            if (i.data().canConvert<Esteem>())
                (t->*peFunc)(i);
        }
}
