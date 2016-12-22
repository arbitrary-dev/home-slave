#include "mainwindow.h"

#include "tasksmodel.h"
#include "tasksdelegate.h"
#include "ui_mainwindow.h"

#include <QPushButton>

const char *MainWindow::STR_INVAL_ESTEEMS = "In order to proceed, all esteems should be valid.";
const char *MainWindow::STR_INPUT_ESTEEMS = "Input esteems";
const char *MainWindow::STR_TAKE_TASKS = "Take tasks!";

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

    // columns' width
    QHeaderView *hh = t->horizontalHeader();
    hh->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
    for (int i = 1; i < m->columnCount(); ++i)
        t->setColumnWidth(i, colWidth(m, i));

    // stage toggle button
    QPushButton *bToggle = ui->btnToggleStage;
    connect(bToggle, &QPushButton::clicked, this, &MainWindow::toggleStage);
    connect(m, &TasksModel::rowsInserted, this, [this] { disableToggleButton(true); });
    connect(m, &TasksModel::dataChanged, this,
            [this, bToggle] (const QModelIndex &idx, const QModelIndex  &, const QVector<int> &) {
                if (    !bToggle->isEnabled()
                        && idx.data(Qt::EditRole).canConvert<Esteem>())
                    refreshToggleButton();
            });

    refreshToggleButton();
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
