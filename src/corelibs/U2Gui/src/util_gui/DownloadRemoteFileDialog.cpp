#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlInputSource>
#include <QMessageBox>
#include <QtGui/QFileDialog>

#include <U2Core/AppContext.h>
#include <U2Core/LoadRemoteDocumentTask.h>
#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/Settings.h>
#include <U2Misc/DialogUtils.h>

#include "DownloadRemoteFileDialog.h"
#include "ui/ui_DownloadRemoteFileDialog.h"
#include "OpenViewTask.h"

#define SAVE_DIR QString("downloadremotefiledialog/savedir")

namespace U2 {

QString DownloadRemoteFileDialog::defaultDB("");

DownloadRemoteFileDialog::DownloadRemoteFileDialog(QWidget *p):QDialog(p), isQueryDB(false) {
    ui = new Ui_DownloadRemoteFileDialog;
    ui->setupUi(this);
    
    connect(ui->databasesBox, SIGNAL(currentIndexChanged ( const QString&)), SLOT( sl_updateHint(const QString&)));
    connect(ui->saveFilenameToolButton, SIGNAL(clicked()), SLOT(sl_saveFilenameButtonClicked()));
    
    RemoteDBRegistry& registry = RemoteDBRegistry::getRemoteDBRegistry();
    const QList<QString> dataBases = registry.getDBs(); 
    foreach(const QString& dbName, dataBases) {
        ui->databasesBox->addItem(dbName);
    }
    
    if (!defaultDB.isEmpty()) {
        int index = ui->databasesBox->findText(defaultDB);
        Q_ASSERT(index < dataBases.count());
        ui->databasesBox->setCurrentIndex(index);
    }
    
    setSaveFilename();
}

const QString DOWNLOAD_REMOTE_FILE_DOMAIN = "DownloadRemoteFileDialog";

void DownloadRemoteFileDialog::sl_saveFilenameButtonClicked() {
    LastOpenDirHelper lod(DOWNLOAD_REMOTE_FILE_DOMAIN);
    QString filename = QFileDialog::getExistingDirectory(this, tr("Select directory to save"), lod.dir);
    if(!filename.isEmpty()) {
        ui->saveFilenameLineEdit->setText(filename);
        lod.url = filename;
    }
}

static const QString DEFAULT_FILENAME = "file.format";
void DownloadRemoteFileDialog::setSaveFilename() {
    QString dir = AppContext::getSettings()->getValue(SAVE_DIR, "").value<QString>();
    if(dir.isEmpty()) {
        dir = LoadRemoteDocumentTask::getDefaultDownloadDirectory();
        assert(!dir.isEmpty());
    }
    ui->saveFilenameLineEdit->setText(QDir::toNativeSeparators(dir));
}

QString DownloadRemoteFileDialog::getResourceId() const
{
    return ui->idLineEdit->text().trimmed();
}

QString DownloadRemoteFileDialog::getDBName() const
{   
    return ui->databasesBox->currentText();
}

QString DownloadRemoteFileDialog::getFullpath() const {
    return ui->saveFilenameLineEdit->text();
}

void DownloadRemoteFileDialog::accept()
{
    defaultDB = getDBName();
    
    QString resourceId = getResourceId();
    if( resourceId.isEmpty() ) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("Resource id is empty!"));
        ui->idLineEdit->setFocus();
        return;
    }
    QString fullPath = getFullpath();
    if( ui->saveFilenameLineEdit->text().isEmpty() ) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("No directory selected for saving file!"));
        ui->saveFilenameLineEdit->setFocus();
        return;
    }

    QString errorMessage;
    fullPath = GUrlUtils::prepareDirLocation(fullPath, errorMessage);

    if (fullPath.isEmpty()) {
        QMessageBox::critical(this, L10N::errorTitle(), errorMessage);
        ui->saveFilenameLineEdit->setFocus();
        return;
    }        
    
    Task* task = new LoadRemoteDocumentAndOpenViewTask(resourceId, getDBName(), fullPath);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
    QDialog::accept();
}

void DownloadRemoteFileDialog::sl_updateHint(const QString& dbName) {
    RemoteDBRegistry& registry = RemoteDBRegistry::getRemoteDBRegistry();
    QString hint = registry.getHint(dbName);
    ui->hintLabel->setText(hint);
    ui->idLineEdit->setToolTip(hint);
}

DownloadRemoteFileDialog::~DownloadRemoteFileDialog() {
    AppContext::getSettings()->setValue(SAVE_DIR, ui->saveFilenameLineEdit->text());
    delete ui;
}

} //namespace 
