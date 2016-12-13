#include "mainwindow.h"
#include "tasksmodel.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QTableView *t = ui->tableView;
    t->setModel(new TasksModel);

    QHeaderView *hh = t->horizontalHeader();
    hh->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);

    t->setShowGrid(false);
    t->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}
