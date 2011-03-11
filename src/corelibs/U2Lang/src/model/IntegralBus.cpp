#include "IntegralBus.h"
#include "IntegralBusType.h"

#include <U2Core/Log.h>
#include <limits.h>

namespace U2 {
namespace Workflow {

static QMap<QString, QStringList> getListMappings(const QStrStrMap& bm, const Port* p) {
    assert(p->isInput());    
    DataTypePtr dt = p->getType();
    QMap<QString, QStringList> res;
    if (dt->isList()) {
        if (bm.contains(p->getId())) {
            res.insert(p->getId(), bm.value(p->getId()).split(";"));
        }
    } else if (dt->isMap()) {
        foreach(Descriptor d, dt->getAllDescriptors()) {
            if (dt->getDatatypeByDescriptor(d)->isList() && bm.contains(d.getId())) {
                res.insert(d.getId(), bm.value(d.getId()).split(";"));
            }
        }
    }
    return res;
}


IntegralBus::IntegralBus(Port* p) : busType(p->getType()), complement(NULL), portId(p->getId()), takenMsgs(0) {
    QString name = p->owner()->getLabel() + "[" + p->owner()->getId()+"]";
    if (p->isInput()) {
        Attribute* a = p->getParameter(IntegralBusPort::BUS_MAP_ATTR_ID);
        if(a == NULL) {
            assert(false);
            return;
        }
        
        busMap = a->getAttributeValue<QStrStrMap>();
        assert(!busMap.isEmpty());
        QMapIterator<QString, QString> it(busMap);
        while (it.hasNext()) {
            it.next();
            coreLog.trace(QString("%1 - input bus map key=%2 val=%3").arg(name).arg(it.key()).arg(it.value()));
        }
        listMap = getListMappings(busMap, p);

    } else { // p is output
        IntegralBusPort* bp = qobject_cast<IntegralBusPort*>(p);
        DataTypePtr t = bp ? bp->getOwnType() : p->getType();
        if (t->isMap()) {
            foreach(Descriptor d, t->getAllDescriptors()) {
                QString key = d.getId();
                QString val = IntegralBusType::assignSlotDesc(d, p).getId();
                busMap.insert(key, val);
            }
        } else {
            QString key = p->getId();
            QString val = IntegralBusType::assignSlotDesc(*p, p).getId();
            busMap.insert(key, val);
        }
        QMapIterator<QString, QString> it(busMap);
        while (it.hasNext()) {
            it.next();
            coreLog.trace(QString("%1 - output bus map key=%2 val=%3").arg(name).arg(it.key()).arg(it.value()));
        }
    }
}

bool IntegralBus::addCommunication(const QString& id, CommunicationChannel* ch) {
    outerChannels.insertMulti(id, ch); 
    return true;
}

CommunicationChannel * IntegralBus::getCommunication(const QString& id) {
    return outerChannels.value(id);
}

Message IntegralBus::get() {
    QVariantMap result;
    context.clear();
    foreach (CommunicationChannel* ch, outerChannels) {
        Message m = ch->get();
        assert(m.getData().type() == QVariant::Map);
        QVariantMap imap = m.getData().toMap();
        context.unite(imap);
        foreach(QString ikey, imap.uniqueKeys()) {
            QVariant ival = imap.value(ikey);
            foreach(QString rkey, busMap.keys(ikey)) {
                coreLog.trace("reducing bus from key="+ikey+" to="+rkey);
                result[rkey] = ival;
            }
            QMapIterator<QString,QStringList> lit(listMap);
            while (lit.hasNext())
            {
                lit.next();
                QString rkey = lit.key();
                assert(!lit.value().isEmpty());
                if (lit.value().contains(ikey)) {
                    QVariantList vl = result[rkey].toList();
                    if (m.getType()->getDatatypeByDescriptor(ikey)->isList()) {
                        vl += ival.toList();
                        coreLog.trace("reducing bus key="+ikey+" to list of "+rkey);
                    } else {
                        vl.append(ival);
                        coreLog.trace("reducing bus key="+ikey+" to list element of "+rkey);
                    }
                    result[rkey] = vl;
                }
            }
        }
    }
    //assert(busType->isMap() || result.size() == 1);
    QVariant data;
    if (busType->isMap()) {
        data.setValue(result);
    } else if (result.size() == 1) {
        data = result.values().at(0);
    }
    if (complement) {
        complement->setContext(context);
    }
    
    takenMsgs++;
    return Message(busType, data);
}

Message IntegralBus::look() const {
    QVariantMap result;
    foreach(CommunicationChannel* channel, outerChannels) {
        assert(channel != NULL);
        Message message = channel->look();
        assert(message.getData().type() == QVariant::Map);
        result.unite(message.getData().toMap());
    }
    return Message(busType, result);
}

Message IntegralBus::composeMessage(const Message& m) {
    QVariantMap data(getContext());
    if (m.getData().type() == QVariant::Map) {
        QMapIterator<QString, QVariant> it(m.getData().toMap());
        while (it.hasNext()) {
            it.next();
            QString key = busMap.value(it.key());
            coreLog.trace("putting key="+key+" remapped from="+it.key());
            data.insert(key, it.value());
        }
    } else {
        assert(busMap.size() == 1);
        data.insert(busMap.values().first(), m.getData());
    }
    return Message(busType, data);
}

void IntegralBus::put(const Message& m) {
    Message busMessage = composeMessage(m);
    foreach(CommunicationChannel* ch, outerChannels) {
        ch->put(busMessage);
    }
}

int IntegralBus::hasMessage() const {
    if (outerChannels.isEmpty()) {
        return 0;
    }
    int num = INT_MAX;
    foreach(CommunicationChannel* ch, outerChannels) {
        num = qMin(num, ch->hasMessage());
    }
    return num;
}

int IntegralBus::takenMessages() const {
    return takenMsgs;
}

int IntegralBus::hasRoom(const DataType*) const {
    if (outerChannels.isEmpty()) {
        return 0;
    }
    int num = INT_MAX;
    foreach(CommunicationChannel* ch, outerChannels) {
        num = qMin(num, ch->hasRoom());
    }
    return num;
}

bool IntegralBus::isEnded() const {
    foreach(CommunicationChannel* ch, outerChannels) {
        if (ch->isEnded()) {
#ifdef _DEBUG
            foreach(CommunicationChannel* dbg, outerChannels) {
                assert(dbg->isEnded());
            }
#endif
            return true;
        }
    }
    return false;
}

void IntegralBus::setEnded() {
    foreach(CommunicationChannel* ch, outerChannels) {
        ch->setEnded();
    }
}

}//namespace Workflow
}//namespace U2
