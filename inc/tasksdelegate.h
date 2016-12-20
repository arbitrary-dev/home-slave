#ifndef TASKSDELEGATE_H
#define TASKSDELEGATE_H

#include <QStyledItemDelegate>
#include <QCheckBox>

#include "tasksmodel.h"

class TasksDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    TasksDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) { }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

private:
    static inline bool isStage(const QModelIndex &index, TasksModel::Stage stage);

    QWidget *makeCheckBox(QWidget *parent) const;
    QWidget *makeComboBox(QWidget *parent) const;

    static bool isntEsteem(const QModelIndex &index) { return !index.data(Qt::EditRole).canConvert<Esteem>(); }
    static Esteem esteem(const QModelIndex &index) { return qvariant_cast<Esteem>(index.data(Qt::EditRole)); }
};

class LayoutClickFilter : public QObject
{
    Q_OBJECT

public:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

// Helpers

template<class T>
static inline T *find(const QWidget *w) {
    return w->findChild<T *>();
}

#endif // TASKSDELEGATE_H
