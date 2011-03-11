#ifndef _DOCK_MANAGER_IMPL_H_
#define _DOCK_MANAGER_IMPL_H_

#include <U2Gui/MainWindow.h>

#include <QtGui/QtGui>

namespace U2 {

class MainWindowImpl;

//used to save/restore geometry of the dock
class DockWrapWidget : public QWidget {
public:
    DockWrapWidget(QWidget* w);
    ~DockWrapWidget();

    virtual QSize sizeHint() const {return hint.isNull() ? w->sizeHint() : hint;}
    QWidget* w;
    QSize hint;
};

class DockData {
public:
    DockData() {area = MWDockArea_Left; label = NULL; dock = NULL; wrapWidget = NULL; action = NULL;}
    MWDockArea          area;
    QLabel*             label;
    QDockWidget*        dock;
    DockWrapWidget*     wrapWidget;
    QIcon               dockIcon;
    QAction*            action;
};

class MWDockManagerImpl : public MWDockManager {
    Q_OBJECT
public:
    MWDockManagerImpl(MainWindowImpl* _mw);
    ~MWDockManagerImpl();

    virtual QAction* registerDock(MWDockArea area, QWidget* w, const QKeySequence& ks = QKeySequence());

    virtual QWidget* findWidget(const QString& widgetObjName);

    virtual QWidget* getActiveWidget(MWDockArea a);

    virtual QWidget* activateDock(const QString& widgetObjName);

    virtual QWidget* toggleDock(const QString& widgetObjName);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void sl_dockVisibilityChanged(bool);
    void sl_widgetDestroyed();
    void sl_toggleDock();
    void sl_toggleDocks();
    
private:
	
    QToolBar* getDockBar(MWDockArea a) const;
    DockData* findDockByLabel(QLabel* l) const ;
    DockData* findDockByName(const QString& objName) const ;
    DockData* findDockByDockWidget(QDockWidget* d) const ;
    void toggleDock(DockData* d);
    void closeDock(DockData* d);
    void openDock(DockData* d);
    void destroyDockData(DockData* d);
    
    void readLastActiveDocksState();
    void saveLastActiveDocksState();
	void saveDockGeometry(DockData* dd);
    void restoreDockGeometry(DockData* dd);
	
    void updateTB(MWDockArea a);
	
    DockData* getActiveDock(MWDockArea area) const;
    
    MainWindowImpl* mwImpl;
    QMainWindow*    mw;
    DockData*       activeDocks[MWDockArea_MaxDocks];
    DockData*       toggleDockState[MWDockArea_MaxDocks];

    QList<DockData*>    docks;

    QToolBar* dockLeft;
    QToolBar* dockRight;
    QToolBar* dockBottom;
    QAction*  statusBarAction;

    QString lastActiveDocksState[MWDockArea_MaxDocks];
};

}//namespace

#endif
