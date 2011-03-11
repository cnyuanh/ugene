#include "BlastRunCommonDialog.h"
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GObjectReference.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Misc/DialogUtils.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/MultiTask.h>

#include <QtGui/QFileDialog>
#include <QtGui/QToolButton>
#include <QtGui/QMessageBox>

namespace U2 {

////////////////////////////////////////
//BlastAllSupportRunCommonDialog
BlastRunCommonDialog::BlastRunCommonDialog(BlastTaskSettings& _settings, QWidget* _parent) :
            QDialog(_parent), settings(_settings)
{
    setupUi(this);
    //I don`t know what this in local BLAST
    phiPatternEdit->hide();
    phiPatternLabel->hide();
    //set avaliable number of threads
    numberOfCPUSpinBox->setMaximum(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
    numberOfCPUSpinBox->setValue(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
    //Connecting people
    connect(programName,SIGNAL(currentIndexChanged(int)),SLOT(sl_onProgNameChange(int)));
    connect(databaseToolButton,SIGNAL(clicked()),SLOT(sl_onBrowseDatabasePath()));
    connect(databasePathLineEdit,SIGNAL(textChanged(QString)),SLOT(sl_lineEditChanged()));
    connect(baseNameLineEdit,SIGNAL(textChanged(QString)),SLOT(sl_lineEditChanged()));
    connect(matrixComboBox,SIGNAL(currentIndexChanged(int)),SLOT(sl_onMatrixChanged(int)));
    sl_onMatrixChanged(0);

    connect(scoresComboBox,SIGNAL(currentIndexChanged(int)),SLOT(sl_onMatchScoresChanged(int)));
    sl_onMatchScoresChanged(0);
    connect(megablastCheckBox,SIGNAL(toggled(bool)),SLOT(sl_megablastChecked()));

    connect(okButton,SIGNAL(clicked()),SLOT(sl_runQuery()));
    connect(restoreButton,SIGNAL(clicked()),SLOT(sl_restoreDefault()));
    sl_onProgNameChange(0);
    okButton->setEnabled(false);
}
void BlastRunCommonDialog::sl_onMatchScoresChanged(int index){
    Q_UNUSED(index);
    settings.matchReward=scoresComboBox->currentText().split(" ").at(0).toInt();
    settings.mismatchPenalty=scoresComboBox->currentText().split(" ").at(1).toInt();
    //For help see http://www.ncbi.nlm.nih.gov/staff/tao/URLAPI/blastall/blastall_node76.html
    //Last values is default
    if((scoresComboBox->currentText() == "1 -4") ||
        (scoresComboBox->currentText() == "1 -3")){//-G 1 -E 2; -G 0 -E 2;-G 2 -E 1; -G 1 -E 1; -G 2 -E 2
        costsComboBox->clear();
        costsComboBox->addItem("2 2");
        costsComboBox->addItem("1 2");
        costsComboBox->addItem("0 2");
        costsComboBox->addItem("2 1");
        costsComboBox->addItem("1 1");
    }else if(scoresComboBox->currentText() == "1 -2"){//-G 1 -E 2; -G 0 -E 2; -G 3 -E 1; -G 2 -E 1; -G 1 -E 1; -G 2 -E 2
        costsComboBox->clear();
        costsComboBox->addItem("2 2");
        costsComboBox->addItem("1 2");
        costsComboBox->addItem("0 2");
        costsComboBox->addItem("3 1");
        costsComboBox->addItem("2 1");
        costsComboBox->addItem("1 1");
    }else if(scoresComboBox->currentText() == "1 -1"){//-G 3 -E 2; -G 2 -E 2; -G 1 -E 2; -G 0 -E 2; -G 4 -E 1; -G 3 -E 1; -G 2 -E 1; -G 4 -E 2  :Not supported megablast
        costsComboBox->clear();
        costsComboBox->addItem("4 2");
        costsComboBox->addItem("3 2");
        costsComboBox->addItem("2 2");
        costsComboBox->addItem("1 2");
        costsComboBox->addItem("0 2");
        costsComboBox->addItem("4 1");
        costsComboBox->addItem("3 1");
        costsComboBox->addItem("2 1");
    }else if((scoresComboBox->currentText() == "2 -7") ||
             (scoresComboBox->currentText() == "2 -5")){//-G 2 -E 4; -G 0 -E 4; -G 4 -E 2; -G 2 -E 2; -G 4 -E 4
        costsComboBox->clear();
        costsComboBox->addItem("4 4");
        costsComboBox->addItem("2 4");
        costsComboBox->addItem("0 4");
        costsComboBox->addItem("4 2");
        costsComboBox->addItem("2 2");
    }else if(scoresComboBox->currentText() == "2 -3"){//-G 4 -E 4; -G 2 -E 4; -G 0 -E 4; -G 3 -E 3; -G 6 -E 2; -G 5 -E 2; -G 4 -E 2; -G 2 -E 2, -G 6 -E 4
        costsComboBox->clear();
        costsComboBox->addItem("6 4");
        costsComboBox->addItem("4 4");
        costsComboBox->addItem("2 4");
        costsComboBox->addItem("0 4");
        costsComboBox->addItem("3 3");
        costsComboBox->addItem("6 2");
        costsComboBox->addItem("5 2");
        costsComboBox->addItem("4 2");
        costsComboBox->addItem("2 2");
    }else if((scoresComboBox->currentText() == "4 -5") ||
             (scoresComboBox->currentText() == "5 -4")){//-G 6 -E 5; -G 5 -E 5; -G 4 -E 5; -G 3 -E 5; -G 12 -E 8
        costsComboBox->clear();
        costsComboBox->addItem("12 8");
        costsComboBox->addItem("6 5");
        costsComboBox->addItem("5 5");
        costsComboBox->addItem("4 5");
        costsComboBox->addItem("3 5");
    }else{
        assert(0);
    }

}
void BlastRunCommonDialog::sl_onMatrixChanged(int index){
    Q_UNUSED(index);
    settings.matrix=matrixComboBox->currentText();
    //For help see http://www.ncbi.nlm.nih.gov/staff/tao/URLAPI/blastall/blastall_node77.html
    //Last values is default
    if(matrixComboBox->currentText() == "PAM30"){//-G 5 -E 2; -G 6 -E 2; -G 7 -E 2; -G 8 -E 1; -G 10 -E 1; -G 9 -E 1
        costsComboBox->clear();
        costsComboBox->addItem("9 1");
        costsComboBox->addItem("5 2");
        costsComboBox->addItem("6 2");
        costsComboBox->addItem("7 2");
        costsComboBox->addItem("8 1");
        costsComboBox->addItem("10 1");
    }else if(matrixComboBox->currentText() == "PAM70"){//-G 6 -E 2; -G 7 -E 2; -G 8 -E 2; -G 9 -E 2; -G 11 -E 1; -G 10 -E 1
        costsComboBox->clear();
        costsComboBox->addItem("10 1");
        costsComboBox->addItem("6 2");
        costsComboBox->addItem("7 2");
        costsComboBox->addItem("8 2");
        costsComboBox->addItem("9 2");
        costsComboBox->addItem("11 1");
    }else if(matrixComboBox->currentText() == "BLOSUM45"){//-G 10 -E 3; -G 11 -E 3; -G 12 -E 3; -G 12 -E 2; -G 13 -E 2, -G 14 -E 2;
                                                            //-G 16 -E 2; -G 15 -E 1; -G 16 -E 1; -G 17 -E 1; -G 18 -E 1; -G 19 -E 1; -G 15 -E 2
        costsComboBox->clear();
        costsComboBox->addItem("15 2");
        costsComboBox->addItem("10 3");
        costsComboBox->addItem("11 3");
        costsComboBox->addItem("12 3");
        costsComboBox->addItem("12 2");
        costsComboBox->addItem("13 2");
        costsComboBox->addItem("14 2");
        costsComboBox->addItem("16 2");
        costsComboBox->addItem("15 1");
        costsComboBox->addItem("16 1");
        costsComboBox->addItem("17 1");
        costsComboBox->addItem("18 1");
        costsComboBox->addItem("19 1");
    }else if(matrixComboBox->currentText() == "BLOSUM62"){//-G 7 -E 2; -G 8 -E 2; -G 9 -E 2; -G 10 -E 1; -G 12 -E 1; -G 11 -E 1
        costsComboBox->clear();
        costsComboBox->addItem("11 1");
        costsComboBox->addItem("7 2");
        costsComboBox->addItem("8 2");
        costsComboBox->addItem("9 2");
        costsComboBox->addItem("10 1");
        costsComboBox->addItem("12 1");
    }else if(matrixComboBox->currentText() == "BLOSUM80"){//-G 6 -E 2; -G 7 -E 2; -G 8 -E 2; -G 9 -E 1; -G 11 -E 1; -G 10 -E 1
        costsComboBox->clear();
        costsComboBox->addItem("10 1");
        costsComboBox->addItem("6 2");
        costsComboBox->addItem("7 2");
        costsComboBox->addItem("8 2");
        costsComboBox->addItem("9 1");
        costsComboBox->addItem("11 1");
    }else{
        assert(0);
    }
}
void BlastRunCommonDialog::sl_restoreDefault(){
    needRestoreDefault=true;
    sl_onProgNameChange(0);
    megablastCheckBox->setChecked(false);
    sl_megablastChecked();
    sl_onMatchScoresChanged(0);
    sl_onMatrixChanged(0);
    evalueSpinBox->setValue(10);
    needRestoreDefault=false;
}
void BlastRunCommonDialog::sl_megablastChecked(){
    if(megablastCheckBox->isChecked()){
        if(wordSizeSpinBox->value()<12 || needRestoreDefault){
            wordSizeSpinBox->setValue(28);
        }
        wordSizeSpinBox->setMaximum(100);
        wordSizeSpinBox->setMinimum(12);
    }else{
        if(wordSizeSpinBox->value()<7 || needRestoreDefault){
            wordSizeSpinBox->setValue(11);
        }
        wordSizeSpinBox->setMaximum(100);
        wordSizeSpinBox->setMinimum(7);
    }
}
void BlastRunCommonDialog::sl_onBrowseDatabasePath(){
    LastOpenDirHelper lod("Database Directory");

    QString name;
    lod.url = name = QFileDialog::getExistingDirectory(NULL, tr("Select a directory with database files"), lod.dir);
    if (!name.isEmpty()) {
        databasePathLineEdit->setText(name);
    }
    databasePathLineEdit->setFocus();
}
void BlastRunCommonDialog::sl_onProgNameChange(int index){
    Q_UNUSED(index);
    settings.programName=programName->currentText();
    if(programName->currentText() == "blastn"){//nucl
        programName->setToolTip(tr("Direct nucleotide alignment"));
        megablastCheckBox->setEnabled(true);
        if(megablastCheckBox->isChecked()){
            wordSizeSpinBox->setValue(28);
            wordSizeSpinBox->setMaximum(100);
            wordSizeSpinBox->setMinimum(12);
        }else{
            wordSizeSpinBox->setValue(11);
            wordSizeSpinBox->setMaximum(100);
            wordSizeSpinBox->setMinimum(7);
        }
        matchScoreLabel->show();
        scoresComboBox->show();
        matrixLabel->hide();
        matrixComboBox->hide();
        serviceLabel->hide();
        serviceComboBox->hide();
    }else if(programName->currentText() == "blastp"){//amino
        programName->setToolTip(tr("Direct protein alignment"));
        megablastCheckBox->setEnabled(false);
        wordSizeSpinBox->setValue(3);
        wordSizeSpinBox->setMaximum(3);
        wordSizeSpinBox->setMinimum(2);

        matchScoreLabel->hide();
        scoresComboBox->hide();
        matrixLabel->show();
        matrixComboBox->show();
        serviceLabel->show();
        serviceComboBox->show();
    }else if(programName->currentText() == "blastx"){//nucl
        programName->setToolTip(tr("Protein alignment, input nucleotide is translated input protein before the search"));
        megablastCheckBox->setEnabled(false);
        wordSizeSpinBox->setValue(3);
        wordSizeSpinBox->setMaximum(3);
        wordSizeSpinBox->setMinimum(2);

        matchScoreLabel->hide();
        scoresComboBox->hide();
        matrixLabel->show();
        matrixComboBox->show();
        serviceLabel->show();
        serviceComboBox->show();
    }else if(programName->currentText() == "tblastn"){//amino
        programName->setToolTip(tr("Protein alignment, nucleotide database is translated input protein before the search"));
        megablastCheckBox->setEnabled(false);
        wordSizeSpinBox->setValue(3);
        wordSizeSpinBox->setMaximum(3);
        wordSizeSpinBox->setMinimum(2);

        matchScoreLabel->hide();
        scoresComboBox->hide();
        matrixLabel->show();
        matrixComboBox->show();
        serviceLabel->show();
        serviceComboBox->show();
    }else if(programName->currentText() == "tblastx"){//nucl
        programName->setToolTip(tr("Protein alignment, both input query and database are translated before the search"));
        megablastCheckBox->setEnabled(false);
        wordSizeSpinBox->setValue(3);
        wordSizeSpinBox->setMaximum(3);
        wordSizeSpinBox->setMinimum(2);

        matchScoreLabel->hide();
        scoresComboBox->hide();
        matrixLabel->show();
        matrixComboBox->show();
        serviceLabel->show();
        serviceComboBox->show();
    }else{
        assert(0);
    }
}
}//namespace
