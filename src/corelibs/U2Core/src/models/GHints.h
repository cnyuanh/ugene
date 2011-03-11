#ifndef _U2_GHINTS_H_
#define _U2_GHINTS_H_

#include <U2Core/global.h>

#include <QtCore/QVariantMap>

namespace U2 {

class U2CORE_EXPORT GHints {
public:
    virtual ~GHints(){};

    virtual QVariantMap getMap() const  = 0;

    virtual void setMap(const QVariantMap& map) = 0;

    virtual QVariant get(const QString& key) const = 0;

    virtual void set(const QString& key, const QVariant& val) = 0;

    virtual int remove(const QString& key) = 0;
};

class U2CORE_EXPORT GHintsDefaultImpl : public GHints {
public:
    GHintsDefaultImpl(const QVariantMap& _map = QVariantMap()) : map(_map) {}
    virtual QVariantMap getMap() const {return map;}

    virtual void setMap(const QVariantMap& _map) {map = _map;}

    virtual QVariant get(const QString& key) const  {return map.value(key);}

    virtual void set(const QString& key, const QVariant& val) {map[key] = val;}

    virtual int remove(const QString& key) {return map.remove(key);}

protected:
    QVariantMap map;
};


class StateLockableTreeItem;

class U2CORE_EXPORT ModTrackHints : public GHintsDefaultImpl {
public:
    ModTrackHints(StateLockableTreeItem* _p, const QVariantMap& _map, bool _topParentMode) 
        : GHintsDefaultImpl(_map), p(_p), topParentMode(_topParentMode){}

    virtual void setMap(const QVariantMap& _map);

    virtual void set(const QString& key, const QVariant& val);

    virtual int remove(const QString& key);

private:
    void                    setModified();
    StateLockableTreeItem*  p;
    bool                    topParentMode;
};

}//namespace

#endif
