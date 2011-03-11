#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <U2Misc/DialogUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Gui/ProjectTreeController.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>

#include "ConstructMoleculeDialog.h"
#include "EditFragmentDialog.h"
#include "CreateFragmentDialog.h"

#include <memory>

namespace U2 {


ConstructMoleculeDialog::ConstructMoleculeDialog(const QList<DNAFragment>& fragmentList,  QWidget* p )
: QDialog(p), fragments(fragmentList)
{
    setupUi(this);
    
    foreach (const DNAFragment& frag, fragments) {
        QString fragItem = QString("%1 (%2) %3").arg(frag.getSequenceName())
            .arg(frag.getSequenceDocName())
            .arg(frag.getName());
        fragmentListWidget->addItem(fragItem);
    }

    
    LastOpenDirHelper lod;
    GUrl url = GUrlUtils::rollFileName(lod.dir + "/new_mol.gb", DocumentUtils::getNewDocFileNameExcludesHint());
    filePathEdit->setText(url.getURLString());
    fragmentListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    molConstructWidget->setColumnWidth(1, molConstructWidget->width()*0.5);
   
    connect(browseButton, SIGNAL(clicked()), SLOT(sl_onBrowseButtonClicked()));
    connect(takeButton, SIGNAL(clicked()), SLOT(sl_onTakeButtonClicked()));
    connect(takeAllButton, SIGNAL(clicked()), SLOT(sl_onTakeAllButtonClicked()));
    connect(fromProjectButton, SIGNAL(clicked()), SLOT(sl_onAddFromProjectButtonClicked()));
    connect(clearButton, SIGNAL(clicked()), SLOT(sl_onClearButtonClicked()));
    connect(upButton, SIGNAL(clicked()), SLOT(sl_onUpButtonClicked()) );
    connect(downButton, SIGNAL(clicked()), SLOT(sl_onDownButtonClicked()));
    connect(removeButton, SIGNAL(clicked()), SLOT(sl_onRemoveButtonClicked()));
    connect(makeCircularBox, SIGNAL(clicked()), SLOT(sl_makeCircularBoxClicked()));
    connect(makeBluntBox, SIGNAL(clicked()), SLOT(sl_forceBluntBoxClicked()) );
    connect(editFragmentButton, SIGNAL(clicked()), SLOT(sl_onEditFragmentButtonClicked()));
    connect(molConstructWidget, SIGNAL(	itemClicked ( QTreeWidgetItem *, int)), SLOT(sl_onItemClicked(QTreeWidgetItem *, int)) );
    
    molConstructWidget->installEventFilter(this);

}

void ConstructMoleculeDialog::accept()
{
    if (selected.isEmpty()) {
        QMessageBox::information(this, windowTitle(), tr("No fragments are selected!\n Please construct molecule from available fragments."));
        return;        
    }
    
    QList<DNAFragment> toLigate;
    foreach(int idx, selected) 
    {
        toLigate.append(fragments[idx]);
    }
        
    LigateFragmentsTaskConfig cfg;
    cfg.checkOverhangs = !makeBluntBox->isChecked();
    cfg.makeCircular = makeCircularBox->isChecked();
    cfg.docUrl = filePathEdit->text();
    cfg.openView = openViewBox->isChecked();
    cfg.saveDoc = saveImmediatlyBox->isChecked();
    cfg.annotateFragments = annotateFragmentsBox->isChecked();
    
    Task* task = new LigateFragmentsTask(toLigate, cfg); 
    AppContext::getTaskScheduler()->registerTopLevelTask(task);

    QDialog::accept();
}


void ConstructMoleculeDialog::sl_onBrowseButtonClicked()
{
    LastOpenDirHelper lod;
    lod.url = QFileDialog::getSaveFileName(this, tr("Set new molecule file name"), lod.dir, tr("Genbank (*.gb )"));
    if (!lod.url.isEmpty()) {
        GUrl result = lod.url;
        filePathEdit->setText(result.getURLString());
    }
}

void ConstructMoleculeDialog::sl_onTakeButtonClicked()
{
    QList<QListWidgetItem*> items = fragmentListWidget->selectedItems();

    foreach (QListWidgetItem* item, items) {
        int curRow = fragmentListWidget->row(item);
        if (!selected.contains(curRow)) {
            selected.append(curRow);
        }
    }

    update();

}

void ConstructMoleculeDialog::sl_onTakeAllButtonClicked()
{
    selected.clear();
    int count = fragmentListWidget->count();
    
    for (int i = 0; i < count; ++i) {
        selected.append(i);
    }
    update();   
}

void ConstructMoleculeDialog::sl_onClearButtonClicked()
{
    selected.clear();
    update();
}

void ConstructMoleculeDialog::sl_onUpButtonClicked()
{
    QTreeWidgetItem* item = molConstructWidget->currentItem();
    if (item == NULL || selected.size() == 1) {
        return;
    }

    int index = molConstructWidget->indexOfTopLevelItem(item);
    int newIndex = index - 1 == -1 ? selected.size() - 1 : index - 1;

    qSwap(selected[index], selected[newIndex]);

    update();

    molConstructWidget->setCurrentItem(molConstructWidget->topLevelItem(newIndex), true);  

}

void ConstructMoleculeDialog::sl_onDownButtonClicked()
{
    QTreeWidgetItem* item = molConstructWidget->currentItem();
    if (item == NULL || selected.size() == 1) {
        return;
    }

    int index = molConstructWidget->indexOfTopLevelItem(item);
    int newIndex = index + 1 == selected.count() ? 0 : index + 1;

    qSwap(selected[index], selected[newIndex]);
    
    update();

    molConstructWidget->setCurrentItem(molConstructWidget->topLevelItem(newIndex), true);
    
}

void ConstructMoleculeDialog::sl_onRemoveButtonClicked()
{
    QTreeWidgetItem* item = molConstructWidget->currentItem();
    if (item == NULL) {
        return;
    }
    int index = molConstructWidget->indexOfTopLevelItem(item);
    selected.removeAt(index);

    update();
}

void ConstructMoleculeDialog::update()
{
    static const QString BLUNT(tr("Blunt"));

    molConstructWidget->clear();

    foreach (int index, selected ) {
        QListWidgetItem* item = fragmentListWidget->item(index);
        assert(item != NULL);
        if (item != NULL) {
            QTreeWidgetItem* newItem = new QTreeWidgetItem(molConstructWidget);
            const DNAFragment& fragment = fragments.at(index);
            newItem->setText(0, fragment.getLeftTerminus().termType == OVERHANG_TYPE_BLUNT ? BLUNT : fragment.getLeftTerminus().overhang);
            newItem->setToolTip(0, tr("5'overhang"));
            newItem->setText(1, item->text());
            newItem->setText(2, fragment.getRightTerminus().termType == OVERHANG_TYPE_BLUNT ? BLUNT : fragment.getRightTerminus().overhang);
            newItem->setToolTip(2, tr("3'overhang"));
            newItem->setCheckState(3, fragment.isInverted() ? Qt::Checked : Qt::Unchecked);
            newItem->setText(3, fragment.isInverted() ? tr("yes") : tr("no"));
            newItem->setToolTip(3, tr("Make fragment reverse complement"));
            
            molConstructWidget->addTopLevelItem(newItem);
        }
    }

    bool checkTermsConsistency = !makeBluntBox->isChecked();

    if (checkTermsConsistency) { 
        QTreeWidgetItem* prevItem = NULL;
        int count = molConstructWidget->topLevelItemCount();
        for(int i = 0; i < count; ++i) {
            QTreeWidgetItem* item = molConstructWidget->topLevelItem(i);
            if (prevItem != NULL) {
                QColor color = prevItem->text(2) == item->text(0) ? Qt::green : Qt::red;
                prevItem->setTextColor(2, color);
                item->setTextColor(0, color);
            }
            prevItem = item;

        }
        if (makeCircularBox->isChecked() && count > 0) {
            QTreeWidgetItem* first = molConstructWidget->topLevelItem(0);
            QTreeWidgetItem* last = molConstructWidget->topLevelItem(count - 1);
            QColor color = first->text(0) == last->text(2) ? Qt::green : Qt::red;
            first->setTextColor(0, color);
            last->setTextColor(2, color);
        }
    }

}

void ConstructMoleculeDialog::sl_makeCircularBoxClicked()
{
    update();
}

void ConstructMoleculeDialog::sl_forceBluntBoxClicked()
{
    update();
}

void ConstructMoleculeDialog::sl_onEditFragmentButtonClicked()
{
    QTreeWidgetItem* item = molConstructWidget->currentItem();
    if (item == NULL) {
        return;
    }
    
    int idx = molConstructWidget->indexOfTopLevelItem(item);
    DNAFragment& fragment = fragments[ selected[idx] ];

    EditFragmentDialog dlg(fragment, this);
    if (dlg.exec() == -1 ) {
        return;
    }

    update();

}



bool ConstructMoleculeDialog::eventFilter( QObject* obj , QEvent* event )
{
    if (obj == molConstructWidget && event->type() == QEvent::FocusOut) {
        molConstructWidget->clearSelection();
    }
    
    return QDialog::eventFilter(obj, event);

}

void ConstructMoleculeDialog::sl_onItemClicked( QTreeWidgetItem * item, int column )
{
    if (column == 3) {
        int idx = molConstructWidget->indexOfTopLevelItem(item);
        DNAFragment& fragment = fragments[ selected[idx] ];
        if (item->checkState(column) == Qt::Checked) {
           fragment.setInverted(true);
        } else {
            fragment.setInverted(false);
        }
        update();
    }
}

void ConstructMoleculeDialog::sl_onAddFromProjectButtonClicked()
{
    ProjectTreeControllerModeSettings settings;
    settings.objectTypesToShow.append(GObjectTypes::SEQUENCE);
    std::auto_ptr<DNASequenceObjectConstraints> seqConstraints(new DNASequenceObjectConstraints());
    seqConstraints->alphabetType = DNAAlphabet_NUCL;
    settings.objectConstraints.append(seqConstraints.get());

    QList<GObject*> objects = ProjectTreeItemSelectorDialog::selectObjects(settings,this);

    if (!objects.isEmpty()) {
        foreach(GObject* obj, objects) {
            if (obj->isUnloaded()) {
                continue;
            }
            DNASequenceObject* seqObj = qobject_cast<DNASequenceObject*>(obj);
            
            if (seqObj) {
                CreateFragmentDialog dlg(seqObj, this);
                if (dlg.exec() == QDialog::Accepted) {
                    DNAFragment frag = dlg.getFragment();
                        QString fragItem = QString("%1 (%2) %3").arg(frag.getSequenceName())
                        .arg(frag.getSequenceDocName())
                        .arg(frag.getName());
                    fragments.append(frag);
                    fragmentListWidget->addItem(fragItem);
                    break;
                }    


            }
        }
    }    
}




} // U2
