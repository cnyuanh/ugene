
#ifndef _U2_REMOTE_MACHINE_MONITOR_H_
#define _U2_REMOTE_MACHINE_MONITOR_H_

#include <QtCore/QList>
#include <QtCore/QVariant>

#include <U2Core/global.h>
#include "RemoteMachine.h"

namespace U2 {

struct RemoteMachineMonitorItem {
    RemoteMachineSettings * machine;
    bool selected;
    
    RemoteMachineMonitorItem( RemoteMachineSettings * s, bool se ) 
        : machine( s ), selected( se ) {
        assert( NULL != machine );
    }
    
    RemoteMachineMonitorItem() 
        : machine( NULL ), selected( false ) {
    }
    
}; // RemoteMachineMonitorItem

/*
 * Stores info about all remote machines registered in system
 * Can be accessed by AppContext::getRemoteMachineMonitor()
 */
class U2REMOTE_EXPORT RemoteMachineMonitor {
public:
    static const QString REMOTE_MACHINE_MONITOR_SETTINGS_TAG;
    
public:
    RemoteMachineMonitor();
    ~RemoteMachineMonitor();
    /* RemoteMachineMonitor takes ownership of this machine */
    bool addMachine( RemoteMachineSettings * machine, bool selected );
    void removeMachine( RemoteMachineSettings * machine );
    void setSelected( RemoteMachineSettings * machine, bool selected );
    RemoteMachineSettings* findMachine(const QString& serializedSettings) const;

    QList< RemoteMachineSettings * > getMachinesList(); /* function not const because we can call initialize() here */
    QList< RemoteMachineMonitorItem > getRemoteMachineMonitorItems();
    QList< RemoteMachineSettings* > getSelectedMachines();

    void saveSettings();
    
private:
    RemoteMachineMonitor( const RemoteMachineMonitor & );
    RemoteMachineMonitor & operator=( const RemoteMachineMonitor & );
    
    QVariant serializeMachines() const;
    bool deserializeMachines( const QVariant & data );
    
    void ensureInitialized();
    void initialize();
    bool hasMachineInMonitor( RemoteMachineSettings * machine ) const;
    
private:
    QList< RemoteMachineMonitorItem >   items;
    bool                                initialized;
    
}; // RemoteMachineMonitor

} // U2

#endif // _U2_REMOTE_MACHINE_MONITOR_H_
