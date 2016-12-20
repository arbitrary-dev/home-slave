#include "tasksdelegate.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QMouseEvent>

QWidget *TasksDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    if (isntEsteem(index))
        return QStyledItemDelegate::createEditor(parent, option, index);

    QWidget *w = new QWidget(parent);
    QHBoxLayout *l = new QHBoxLayout;

    QWidget *e =
        isStage(index, TasksModel::ST_INPUT_ESTEEMS)
            ? makeComboBox(w)
            : makeCheckBox(w);

    l->addWidget(e);
    l->setAlignment(Qt::AlignCenter);
    l->setContentsMargins(0, 0, 0, 0);

    w->setLayout(l);
    w->installEventFilter(new LayoutClickFilter());

    return w;
}

void TasksDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (isntEsteem(index)) {
        QStyledItemDelegate::setEditorData(editor, index);
        return;
    }

    Esteem est = esteem(index);

    if (isStage(index, TasksModel::ST_INPUT_ESTEEMS)) {
        QComboBox *e = find<QComboBox>(editor);
        e->setCurrentIndex(est.val - 1);
    } else {
        QCheckBox *e = find<QCheckBox>(editor);
        e->setChecked(est.tkn);
    }
}

void TasksDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    if (isntEsteem(index)) {
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    Esteem est = esteem(index);

    if (isStage(index, TasksModel::ST_INPUT_ESTEEMS)) {
        QComboBox *e = find<QComboBox>(editor);
        est.val = e->currentIndex() + 1;
    } else {
        QCheckBox *e = find<QCheckBox>(editor);
        est.tkn = e->isChecked();
    }

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

bool TasksDelegate::isStage(const QModelIndex &index, TasksModel::Stage stage)
{
    const TasksModel *model = dynamic_cast<const TasksModel *>(index.model());
    return model->isStage(stage);
}

QWidget *TasksDelegate::makeCheckBox(QWidget *parent) const
{
    QCheckBox *cb = new QCheckBox;
    connect(cb, &QCheckBox::stateChanged, [=] { emit commitData(parent); });
    return cb;
}

QWidget *TasksDelegate::makeComboBox(QWidget *parent) const
{
    QComboBox *cmb = new QComboBox;

    for (int i = 1; i <= 5; ++i) {
        QString str = QString::number(i);

        if (i == 1 || i == 5) {
            str.append(" - ");
            str.append(tr(i == 1 ? "easy" : "hard"));
        }

        cmb->addItem(str);
    }

    connect(cmb, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), // activated used in layout filter v
            [=] {
                emit commitData(parent);
                emit closeEditor(parent);
            });

    return cmb;
}

bool LayoutClickFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QWidget *w = qobject_cast<QWidget *>(watched);
        QMouseEvent *me = dynamic_cast<QMouseEvent *>(event);

        if (!w->rect().contains(me->pos()))
            return false;

        if (find<QCheckBox>(w))
            find<QCheckBox>(w)->toggle();
        else
            emit find<QComboBox>(w)->activated(-1); // activated connected to commit & close ^

        return true;
    }

    return false;
}
