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
    static inline QCheckBox *cb(const QWidget *w);

    static bool isntEsteem(const QModelIndex &index) { return !index.data(Qt::EditRole).canConvert<Esteem>(); }
    static Esteem esteem(const QModelIndex &index) { return qvariant_cast<Esteem>(index.data(Qt::EditRole)); }

private slots:
    void cbStateChanged();
};

class CheckBoxFilter : public QObject
{
    Q_OBJECT

public:
    CheckBoxFilter(QCheckBox *_cb) : cb(_cb) { }

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QCheckBox *cb;
};

#endif // TASKSDELEGATE_H
