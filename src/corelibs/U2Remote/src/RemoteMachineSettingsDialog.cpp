
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>

#include <U2Core/AppContext.h>

#include "RemoteMachineSettingsDialog.h"

namespace U2 {

RemoteMachineSettingsDialog::RemoteMachineSettingsDialog(QWidget* parent, RemoteMachineSettings* settings) : QDialog(parent), machineSettings( settings ), currentUi( NULL ) {
    setupUi( this );
    
    ProtocolInfoRegistry * pir = AppContext::getProtocolInfoRegistry();
    assert( NULL != pir );
    
    QList< ProtocolInfo* > protoInfos = pir->getProtocolInfos();
    assert(protoInfos.size() > 0);
       
    ProtocolInfo* pi = protoInfos.first();    
    currentUi = pi->getProtocolUI();
    QVBoxLayout * topLayout = qobject_cast< QVBoxLayout* >( layout() );
    topLayout->insertWidget( 0, currentUi );
    protoId = pi->getId();

    if (machineSettings != NULL) {
        currentUi->initializeWidget(machineSettings);
    } 
    
    connect( cancelPushButton, SIGNAL( clicked() ), SLOT( reject() ) );
    connect( okPushButton, SIGNAL( clicked() ), SLOT( sl_okPushButtonClicked() ) );
    
}

RemoteMachineSettingsDialog::~RemoteMachineSettingsDialog() {
    
    if( NULL != currentUi ) {
        QVBoxLayout * topLayout = qobject_cast< QVBoxLayout* >( layout() );
        assert( NULL != topLayout );
        Q_UNUSED(topLayout);
        currentUi->setParent( NULL );
    }
}

RemoteMachineSettings * RemoteMachineSettingsDialog::getMachineSettings() const {
    return machineSettings;
}




void RemoteMachineSettingsDialog::sl_okPushButtonClicked() {
    
    QString error = currentUi->validate();
    if( !error.isEmpty() ) {
        QMessageBox::critical( this, tr( "Error!" ), error );
        return;
    }
    
    createMachineSettings();
    
    QDialog::accept();
}


void RemoteMachineSettingsDialog::showErrorLabel( const QString& msg )
{
    assert( !msg.isEmpty() );
    QLabel * errorLabel = new QLabel( msg, this );
    QVBoxLayout * topLayout = qobject_cast< QVBoxLayout* >( layout() );
    assert( NULL != topLayout );
    topLayout->insertWidget( 0, errorLabel );


}

void RemoteMachineSettingsDialog::createMachineSettings()
{
    assert( !protoId.isEmpty() );

    machineSettings = currentUi->createMachine();
    if( NULL == machineSettings ) {
        QMessageBox::critical( this, tr( "Error!" ), tr( "Sorry! Cannot create remote machine" ) );
    }  

}

} // U2
