#include "mainwindow.h"

#include "tasksmodel.h"
#include "tasksdelegate.h"
#include "ui_mainwindow.h"

#include <QPushButton>

const char *MainWindow::BTN_INPUT_ESTEEMS = "Input esteems";
const char *MainWindow::BTN_TAKE_TASKS = "Take tasks!";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QTableView *t = ui->tableView;

    TasksModel *m = new TasksModel;
    t->setModel(m);
    t->setShowGrid(false);
    t->setItemDelegate(new TasksDelegate);
    t->setEditTriggers(QAbstractItemView::DoubleClicked);
    t->setSelectionMode(QAbstractItemView::NoSelection);

    // setup columns' width
    QHeaderView *hh = t->horizontalHeader();
    hh->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
    for (int i = 1; i < m->columnCount(); ++i)
        t->setColumnWidth(i, colWidth(m, i));

    // Stage toggle button.
    QPushButton *bToggle = ui->btnToggleStage;
    connect(bToggle, &QPushButton::clicked, this, &MainWindow::toggleStage);

    refreshView();
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
    refreshView();
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
    bToggle->setText(tr(s_input ? BTN_TAKE_TASKS : BTN_INPUT_ESTEEMS));

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
