#include "tasksdelegate.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QMouseEvent>

QWidget *TasksDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    if (isntEsteem(index))
        return QStyledItemDelegate::createEditor(parent, option, index);

    QWidget *w = new QWidget(parent);
    QHBoxLayout *l = new QHBoxLayout;
    QCheckBox *e = new QCheckBox;

    connect(e, &QCheckBox::stateChanged, this, &TasksDelegate::cbStateChanged);

    l->addWidget(e);
    l->setAlignment(Qt::AlignCenter);
    l->setContentsMargins(0, 0, 0, 0);

    w->setLayout(l);
    w->installEventFilter(new CheckBoxFilter(e));

    return w;
}

void TasksDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (isntEsteem(index)) {
        QStyledItemDelegate::setEditorData(editor, index);
        return;
    }

    QCheckBox *e = cb(editor);
    Esteem est = qvariant_cast<Esteem>(index.data());
    e->setChecked(est.tkn);
}

void TasksDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    if (isntEsteem(index)) {
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    QCheckBox *e = cb(editor);
    Esteem est = qvariant_cast<Esteem>(index.data());
    est.tkn = e->isChecked();
    model->setData(index, est, Qt::EditRole);
}

void TasksDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    if (isntEsteem(index)) {
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);
        return;
    }

    editor->setGeometry(option.rect);
}

QCheckBox *TasksDelegate::cb(const QWidget *w) const
{
    return w->findChild<QCheckBox*>();
}

void TasksDelegate::cbStateChanged()
{
    QCheckBox *cb = qobject_cast<QCheckBox*>(sender());
    QWidget *w = cb->parentWidget();

    emit commitData(w);
}

bool CheckBoxFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QWidget *w = qobject_cast<QWidget *>(watched);
        QMouseEvent *me = dynamic_cast<QMouseEvent *>(event);

        if (!w->rect().contains(me->pos()))
            return false;

        cb->toggle();
        return true;
    }

    return false;
}
