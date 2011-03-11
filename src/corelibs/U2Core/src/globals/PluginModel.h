#ifndef _U2_PLUGINMODEL_H_
#define _U2_PLUGINMODEL_H_

#include <U2Core/global.h>

#include <QtCore/QString>
#include <QtCore/QList>

namespace U2 {
//BUG:417: add API version check

class Service;
class ServiceRegistry;
class Plugin;
class PluginSupport;
class Task;

#define U2_PLUGIN_INIT_FUNC ugene_plugin_init
#define U2_PLUGIN_INIT_FUNC_NAME "ugene_plugin_init"

typedef Plugin*(*PLUG_INIT_FUNC) ();

enum PluginState {
    PluginState_Loaded,
    PluginState_FailedToLoad
};

class U2CORE_EXPORT Plugin : public QObject {
    Q_OBJECT
public:
    Plugin(const QString & _name, const QString& _desc, PluginState _state = PluginState_Loaded) 
        : name(_name), description(_desc), state(_state){}
    
    //plugin is deallocated by plugin_support service when it's removed or on application shutting down
    virtual ~Plugin(){}

    const QString& getName() const {return name;}

    const QString& getDescription() const {return description;}


    PluginState getState() const {return state;}

    // returns list of services provided by the plugin
    // after plugin is loaded all services from this list are automatically registered 
    const QList<Service*>& getServices() const {return services;}

protected:
    QString         name, description;
    QList<Service*> services;
    PluginState     state;
};


class U2CORE_EXPORT PluginSupport : public QObject {
    friend class LoadAllPluginsTask;
    Q_OBJECT

public:
    virtual const QList<Plugin*>&   getPlugins() = 0;

    virtual Task* addPluginTask(const QString& pathToPlugin) = 0;

    //plugin will not be removed from the plugin list during the next app run
    virtual void setRemoveFlag(Plugin* p, bool v) = 0;
    virtual bool getRemoveFlag(Plugin* p) const = 0;
    virtual bool isAllPluginsLoaded() const = 0;

signals:
    void si_pluginAdded(Plugin*);
    void si_pluginRemoveFlagChanged(Plugin*);
    void si_allStartUpPluginsLoaded();

};

}//namespace

#endif
