#include "KalignPlugin.h"
#include "KalignTask.h"
#include "KalignConstants.h"
#include "KalignDialogController.h"
#include "KalignSettingsWidget.h"
#include "KalignWorker.h"
#include "kalign_tests/KalignTests.h"

#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorFactory.h>
#include <U2View/MSAAlignUtils.h>
#include <U2View/MSAAlignDialog.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Algorithm/MSAAlignAlgRegistry.h>
#include <U2Lang/WorkflowSettings.h>

#include <U2Core/GAutoDeleteList.h>
#include <U2Gui/GUIUtils.h>
#include <U2Misc/DialogUtils.h>

#include <U2Test/GTestFrameworkComponents.h>

#include <QtGui/QDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QMainWindow>

namespace U2 {

class MSAAlignDialog;

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    KalignPlugin * plug = new KalignPlugin();
    return plug;
}

class KalignGuiExtFactory : public MSAAlignGUIExtensionsFactory {
public:
    KalignGuiExtFactory(){};
    MSAAlignAlgorithmMainWidget* createMainWidget(QWidget* parent) {return new KalignSettingsWidget(parent);}
    bool hasMainWidget() {return true;}
};

KalignPlugin::KalignPlugin() 
    : Plugin(tr("Kalign"), 
    tr("A port of Kalign package for multiple sequence alignment. Check http://msa.sbc.su.se for the original version")),
    ctx(NULL)
{

    bool guiMode = AppContext::getMainWindow()!=NULL;
    MSAAlignAlgRegistry* registry = AppContext::getMSAAlignAlgRegistry();

    MSAAlignGUIExtensionsFactory* guiFactory = guiMode ? new KalignGuiExtFactory(): NULL;
    MSAAlignAlgorithmEnv* algo = new MSAAlignAlgorithmEnv(KalignMainTask::taskName, new KalignMainTask::Factory(), guiFactory);
    bool res = registry->registerAlgorithm(algo);
    Q_UNUSED(res);
    assert(res);    

    if (guiMode) {
        ctx = new KalignMSAEditorContext(this);
        ctx->init();

        QAction* kalignAction = new QAction(tr("Kalign..."), this);
        kalignAction->setIcon(QIcon(":kalign/images/kalign_16.png"));
        QMenu* tools = AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS);
        QMenu* toolsSubmenu = tools->findChild<QMenu*>(MWMENU_TOOLS_MALIGN);
        if (toolsSubmenu == NULL){
            toolsSubmenu = tools->addMenu(tr("Multiple alignment"));
            toolsSubmenu->setObjectName(MWMENU_TOOLS_MALIGN);
        }
        toolsSubmenu->addAction(kalignAction);
        connect(kalignAction,SIGNAL(triggered()),SLOT(sl_runKalignTask()));
    }
    
    LocalWorkflow::KalignWorkerFactory::init(); //TODO
    //TODO:
    //Kalign Test

    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = KalignTests ::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }
}

void KalignPlugin::sl_runKalignTask() {
    //Call select input file and setup settings dialog
    MSAAlignAlgRegistry* registry = AppContext::getMSAAlignAlgRegistry();    
    MSAAlignDialog dlg(KalignMainTask::taskName, QApplication::activeWindow());
    if (dlg.exec()) {
        MSAAlignTaskSettings s;
        s.algName = dlg.getAlgorithmName();
        s.resultFileName = dlg.getResultFileName();
        assert(!s.resultFileName.isEmpty());
        s.setCustomSettings(dlg.getCustomSettings());
        s.loadResultDocument = true;
        Task* alignTask = new MSAAlignFileTask(s);
        AppContext::getTaskScheduler()->registerTopLevelTask(alignTask);
    }
}

KalignPlugin::~KalignPlugin() {
    //nothing to do
}

MSAEditor* KalignAction::getMSAEditor() const {
    MSAEditor* e = qobject_cast<MSAEditor*>(getObjectView());
    assert(e!=NULL);
    return e;
}

void KalignAction::sl_lockedStateChanged() {
    StateLockableItem* item = qobject_cast<StateLockableItem*>(sender());
    assert(item!=NULL);
    setEnabled(!item->isStateLocked());
}

KalignMSAEditorContext::KalignMSAEditorContext(QObject* p) : GObjectViewWindowContext(p, MSAEditorFactory::ID) {
}

void KalignMSAEditorContext::initViewContext(GObjectView* view) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(view);
    assert(msaed!=NULL);
    if (msaed->getMSAObject() == NULL) {
        return;
    }

    bool objLocked = msaed->getMSAObject()->isStateLocked();
    KalignAction* alignAction = new KalignAction(this, view, tr("Align with Kalign..."), 2000);
    alignAction->setIcon(QIcon(":kalign/images/kalign_16.png"));
    alignAction->setEnabled(!objLocked);
    connect(alignAction, SIGNAL(triggered()), SLOT(sl_align()));
    connect(msaed->getMSAObject(), SIGNAL(si_lockedStateChanged()), alignAction, SLOT(sl_lockedStateChanged()));
    addViewAction(alignAction);
}

void KalignMSAEditorContext::buildMenu(GObjectView* v, QMenu* m) {
    QList<GObjectViewAction *> actions = getViewActions(v);
    QMenu* alignMenu = GUIUtils::findSubMenu(m, MSAE_MENU_ALIGN);
    assert(alignMenu!=NULL);
    foreach(GObjectViewAction* a, actions) {
        a->addToMenuWithOrder(alignMenu);
    }    
}

void KalignMSAEditorContext::sl_align() {
    KalignAction* action = qobject_cast<KalignAction*>(sender());
    assert(action!=NULL);
    MSAEditor* ed = action->getMSAEditor();
    MAlignmentObject* obj = ed->getMSAObject(); 
    if (obj == NULL)
        return;
    assert(!obj->isStateLocked());
    
    KalignTaskSettings s;
    KalignDialogController dlg(ed->getWidget(), obj->getMAlignment(), s);
    
    int rc = dlg.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    
    Task * kalignTask = NULL;
    if(WorkflowSettings::runInSeparateProcess() && !WorkflowSettings::getCmdlineUgenePath().isEmpty()) {
        kalignTask = new KalignGObjectRunFromSchemaTask(obj, s);
    } else {
        kalignTask = new KalignGObjectTask(obj, s);
    }
    AppContext::getTaskScheduler()->registerTopLevelTask( kalignTask );
}

}//namespace
