#include "CreateSubalignimentDialogController.h"

#include <U2Misc/DialogUtils.h>
#include <U2Formats/GenbankLocationParser.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>

#include <QtGui/qfiledialog.h>
#include <QtGui/qmessagebox.h>

namespace U2{

CreateSubalignimentDialogController::CreateSubalignimentDialogController(MAlignmentObject *_mobj, const QRect& selection, QWidget *p)
: QDialog(p), mobj(_mobj){
    setupUi(this);

    connect(browseButton, SIGNAL(clicked()), SLOT(sl_browseButtonClicked()));
    connect(allButton, SIGNAL(clicked()), SLOT(sl_allButtonClicked()));
    connect(noneButton, SIGNAL(clicked()), SLOT(sl_noneButtonClicked()));
    connect(invertButton, SIGNAL(clicked()), SLOT(sl_invertButtonClicked()));

    int rowNumber = mobj->getMAlignment().getNumRows();
    int alignLength = mobj->getMAlignment().getLength();

    sequencesTableWidget->clearContents();
    sequencesTableWidget->setRowCount(rowNumber);
    sequencesTableWidget->setColumnCount(1);
    sequencesTableWidget->verticalHeader()->setHidden( true );
    sequencesTableWidget->horizontalHeader()->setHidden( true );
    sequencesTableWidget->setShowGrid(false);
    sequencesTableWidget->horizontalHeader()->setResizeMode( 0, QHeaderView::Stretch );
    
    int startSeq = -1;
    int endSeq = -1;
    int startPos = -1;
    int endPos = -1;
    if (selection.isNull()) {
        startPos = 1;
        endPos = alignLength;
        startSeq = 0;
        endSeq = rowNumber -1;
     } else {
        startSeq = selection.y();
        endSeq = selection.y() + selection.height() - 1;
        startPos = selection.x() + 1;
        endPos = selection.x() + selection.width();
    }
    
    startPosBox->setMaximum(alignLength);
    endPosBox->setMaximum(alignLength);
    
    startPosBox->setValue(startPos);
    endPosBox->setValue(endPos);
    
    for (int i=0; i<rowNumber; i++) {
        QCheckBox *cb = new QCheckBox(mobj->getMAlignment().getRow(i).getName(), this);
        cb->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
        if ( (i >= startSeq) && (i <= endSeq)) {
            cb->setChecked(true);
        }
        sequencesTableWidget->setCellWidget(i, 0, cb);
        sequencesTableWidget->setRowHeight(i, 15);
    }

    QList<DocumentFormatId> dfIdList = AppContext::getDocumentFormatRegistry()->getRegisteredFormats(), supportedFormats;
    
    //CRITICAL: do not create doc here -> add the way to learn about constraints without doc!
    foreach(DocumentFormatId dfId, dfIdList){
        DocumentFormat *df = AppContext::getDocumentFormatRegistry()->getFormatById(dfId);
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::VFS_FILE);
        Document *d = df->createNewDocument(iof, GUrl("fake", GUrl_VFSFile));
        bool supported = df->isObjectOpSupported(d, DocumentFormat::DocObjectOp_Add, GObjectTypes::MULTIPLE_ALIGNMENT);
        if(supported){
            foreach(QString ext, df->getSupportedDocumentFileExtensions()){
                filter.append("*." + ext + " ");
            }
        }
    }
}

void CreateSubalignimentDialogController::sl_browseButtonClicked(){
    LastOpenDirHelper h;

    QString newPath = QFileDialog::getSaveFileName(this, tr("Select file to save..."), h.dir, filter );
    filepathEdit->setText(newPath);
}

void CreateSubalignimentDialogController::sl_allButtonClicked(){
    for (int i=0; i<sequencesTableWidget->rowCount(); i++) {
        QCheckBox *cb = qobject_cast<QCheckBox*>(sequencesTableWidget->cellWidget(i, 0));
        cb->setChecked(true);
    }
}

void CreateSubalignimentDialogController::sl_invertButtonClicked(){
    for (int i=0; i<sequencesTableWidget->rowCount(); i++) {
        QCheckBox *cb = qobject_cast<QCheckBox*>(sequencesTableWidget->cellWidget(i, 0));
        cb->setChecked(!cb->isChecked());
    }
}

void CreateSubalignimentDialogController::sl_noneButtonClicked(){
    for (int i=0; i<sequencesTableWidget->rowCount(); i++) {
        QCheckBox *cb = qobject_cast<QCheckBox*>(sequencesTableWidget->cellWidget(i, 0));
        cb->setChecked(false);
    }
}

void CreateSubalignimentDialogController::accept(){
    QFileInfo fi(filepathEdit->text());
    QDir dirToSave(fi.dir());
    if (!dirToSave.exists()){
        QMessageBox::critical(this, this->windowTitle(), tr("Directory to save is not exists"));
        return;
    }
    if(filepathEdit->text().isEmpty()){
        QMessageBox::critical(this, this->windowTitle(), tr("No path specified"));
        return;
    }
    if(fi.baseName().isEmpty()){
        QMessageBox::critical(this, this->windowTitle(), tr("Filename to save is empty"));
        return;
    }
    
    // '-1' because in memory positions start from 0 not 1
    int start = startPosBox->value() - 1;
    int end = endPosBox->value() - 1;
    int seqLen = mobj->getMAlignment().getLength();
    
    if( start > end ) {
        QMessageBox::critical(this, windowTitle(), tr("Start position must be greater than end position"));
        return;
    }
    
    U2Region region(start, end - start + 1), sequence(0, seqLen);
    if(!sequence.contains(region)){
        QMessageBox::critical(this, this->windowTitle(), tr("Entered region not contained in current sequence"));
        return;
    }

    selectSeqNames();

    if(selectedNames.size() == 0){
        QMessageBox::critical(this, this->windowTitle(), tr("You must select at least one sequence"));
        return;
    }

    window = region;

    this->close();
    QDialog::accept();
}

void CreateSubalignimentDialogController::selectSeqNames(){
    QStringList names;
    for (int i=0; i<sequencesTableWidget->rowCount(); i++) {
        QCheckBox *cb = qobject_cast<QCheckBox*>(sequencesTableWidget->cellWidget(i, 0));
        if(cb->isChecked()){
            names.append(cb->text());
        }
    }
    selectedNames = names;
}

};
