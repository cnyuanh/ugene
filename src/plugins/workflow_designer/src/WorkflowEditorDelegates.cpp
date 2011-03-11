#include "WorkflowEditor.h"
#include "IterationListWidget.h"

#include "WorkflowEditorDelegates.h"

namespace U2 {

/********************************
 * ProxyDelegate
 ********************************/
QWidget *ProxyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    //if (owner->custom) 
    {
        QItemDelegate* itemDelegate = index.model()->data(index, DelegateRole).value<PropertyDelegate*>();
        if (itemDelegate) {
            connect(itemDelegate, SIGNAL(commitData(QWidget*)), SIGNAL(commitData(QWidget*)));
            return itemDelegate->createEditor(parent, option, index);
        }
    }
    return QItemDelegate::createEditor(parent, option, index);
}

void ProxyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    //if (owner->custom) 
    {
        QItemDelegate* itemDelegate = index.model()->data(index, DelegateRole).value<PropertyDelegate*>();
        if (itemDelegate) {
            itemDelegate->setEditorData(editor, index);
            return;
        }
    }
    QItemDelegate::setEditorData(editor, index);
}

void ProxyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QVariant old;
    QVariant expanded;
    PropertyDelegate* propertyDelegate = model->data(index, DelegateRole).value<PropertyDelegate*>();
    if (/*owner->custom &&*/ propertyDelegate ) {
        old = model->data(index, ConfigurationEditor::ItemValueRole);
        propertyDelegate->setModelData(editor, model, index);
        expanded = model->data(index, ConfigurationEditor::ItemListValueRole);
    } else {
        old = model->data(index, Qt::EditRole);
        QItemDelegate::setModelData(editor, model, index);
    }
   /* QString name = model->data(index, DescriptorRole).value<Descriptor>().getId();
    if (handlePropertyValueList(name, expanded)) {
        return;
    }*/
    QVariant val = model->data(index, (propertyDelegate == NULL) ? (int)Qt::EditRole : (int)ConfigurationEditor::ItemValueRole);
    if (val != old) {
         QString name = model->data(index, DescriptorRole).value<Descriptor>().getId();
        if (handlePropertyValueList(name, expanded)) {
            return;
        }

        setPropertyValue(name, val);
        if (propertyDelegate) {
            model->setData(index, propertyDelegate->getDisplayValue(val), Qt::DisplayRole);
        }
        model->setData(index, model->data(index, Qt::DisplayRole).toString(), Qt::ToolTipRole);
    }
}

/********************************
 * SuperDelegate
 ********************************/
SuperDelegate::SuperDelegate(WorkflowEditor *parent) : ProxyDelegate(parent), owner(parent) {
}

bool SuperDelegate::handlePropertyValueList(const QString& name, QVariant list) const {
    return owner->iterationList->expandList(owner->actor->getId(), name, list);
}

} // U2
