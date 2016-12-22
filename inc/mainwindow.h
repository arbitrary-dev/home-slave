#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    static const char *BTN_INPUT_ESTEEMS;
    static const char *BTN_TAKE_TASKS;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    void refreshView();

    static inline int colWidth(const QAbstractTableModel *m, int col);

private slots:
    void toggleStage();
};

#endif // MAINWINDOW_H
