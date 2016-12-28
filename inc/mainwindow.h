#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>

#include "tasksmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    static const char *STR_INVAL_ESTEEMS;
    static const char *STR_INPUT_ESTEEMS;
    static const char *STR_TAKE_TASKS;
    static const char *STR_PERST_CHANGES;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    TasksModel *tasksModel;

    void refreshView();
    void disableToggleButton(bool disable);

    static inline int colWidth(const QAbstractTableModel *m, int col);

private slots:
    void toggleStage();
    void refreshToggleButton();

    // TODO rfct to TasksView::*
    bool taskAdded(const QModelIndex &, int row, int);
    bool taskDeleted(const QModelIndex &, int row, int);
    bool taskChanged(const QModelIndex &index, const QModelIndex &, const QVector<int> &);

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
