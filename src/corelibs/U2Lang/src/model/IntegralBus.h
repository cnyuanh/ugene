#ifndef _U2_WORKFLOW_INTEGRAL_BUS_H_
#define _U2_WORKFLOW_INTEGRAL_BUS_H_

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowTransport.h>

namespace U2 {

namespace Workflow {

/**
 * represents communication channel for support passing data between actors
 * connected in transitive closure of schema graph
 * 
 * is a container of communications with other actors
 */
class U2LANG_EXPORT IntegralBus : public QObject, public CommunicationSubject, public CommunicationChannel {
    Q_OBJECT
public:
    IntegralBus(Port* peer);
    
    // reimplemented from CommunicationSubject
    virtual bool addCommunication(const QString& id, CommunicationChannel* ch);
    virtual CommunicationChannel* getCommunication(const QString& id);
    
    // reimplemented from CommunicationChannel
    virtual Message get();
    virtual Message look() const;
    virtual void put(const Message& m);
    virtual int hasMessage() const;
    virtual int takenMessages() const;
    virtual int hasRoom(const DataType* t = NULL) const;
    virtual bool isEnded() const;
    virtual void setEnded();
    virtual int capacity() const {return 1;}
    virtual void setCapacity(int) {}
    
    virtual QVariantMap getContext() const {return context;}
    virtual void setContext(const QVariantMap& m) {context = m;}
    
    virtual void addComplement(IntegralBus* b) {assert(!complement);complement = b;}
    
    QString getPortId() const {return portId;}
    
protected:
    virtual Message composeMessage(const Message&);

protected:
    // type of port integral bus is binded to
    DataTypePtr busType;
    // communications with other ports
    QMap<QString, CommunicationChannel*> outerChannels;
    // busmap of port integral bus is binded to
    QStrStrMap busMap;
    // 
    QMap<QString, QStringList> listMap;
    // 
    QVariantMap context;
    // 
    IntegralBus* complement;
    // integral bus is binded to port with this id
    QString portId;
    //
    int takenMsgs;
    
}; // IntegralBus

}//Workflow namespace

}//GB2 namespace

#endif // _U2_WORKFLOW_INTEGRAL_BUS_H_
