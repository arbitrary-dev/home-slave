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

    static const char *STR_INVAL_ESTEEMS;
    static const char *STR_INPUT_ESTEEMS;
    static const char *STR_TAKE_TASKS;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    void refreshView();
    void disableToggleButton(bool disable);

    static inline int colWidth(const QAbstractTableModel *m, int col);

private slots:
    void toggleStage();
    void refreshToggleButton();
};

#endif // MAINWINDOW_H
