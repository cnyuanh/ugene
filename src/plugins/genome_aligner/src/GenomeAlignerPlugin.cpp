#include "GenomeAlignerPlugin.h"
#include "GenomeAlignerTask.h"
#include "GenomeAlignerSettingsWidget.h"
#include "BuildSArraySettingsWidget.h"

#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#include <U2Algorithm/DnaAssemblyAlgRegistry.h>
#include <U2Lang/WorkflowEnv.h>

#include "GenomeAlignerTask.h"
#include "GenomeAlignerWorker.h"

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin * U2_PLUGIN_INIT_FUNC() {
    GenomeAlignerPlugin * plug = new GenomeAlignerPlugin();
    return plug;
}

const QString GenomeAlignerPlugin::GENOME_ALIGNER_INDEX_TYPE_ID("gai");

DataTypePtr GenomeAlignerPlugin::GENOME_ALIGNER_INDEX_TYPE()
{
    DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    static bool startup = true;
    if (startup)
    {
        dtr->registerEntry(DataTypePtr(new DataType(GENOME_ALIGNER_INDEX_TYPE_ID, QString("Genome aligner index"), QString("Index for genome aligner"))));
        startup = false;
    }
    return dtr->getById(GENOME_ALIGNER_INDEX_TYPE_ID);
}

class GenomeAlignerGuiExtFactory : public DnaAssemblyGUIExtensionsFactory {
public:
    GenomeAlignerGuiExtFactory(){};
    DnaAssemblyAlgorithmMainWidget* createMainWidget(QWidget* parent) {return new GenomeAlignerSettingsWidget(parent);}
    DnaAssemblyAlgorithmBuildIndexWidget* createBuildIndexWidget(QWidget* parent) {assert(0); return NULL;}
    bool hasMainWidget() {return true;}
    bool hasBuildIndexWidget() {return false;}
};

GenomeAlignerPlugin::GenomeAlignerPlugin() : Plugin( tr("UGENE genome aligner"), tr("Assembly DNA to reference sequence") ) {
    // Register GenomeAligner algorithm
    DnaAssemblyAlgRegistry* registry = AppContext::getDnaAssemblyAlgRegistry();
    
    bool guiMode = AppContext::getMainWindow();
    DnaAssemblyGUIExtensionsFactory* guiFactory = guiMode ? new GenomeAlignerGuiExtFactory(): NULL;
    DnaAssemblyAlgorithmEnv* algo = new DnaAssemblyAlgorithmEnv("UGENE genome aligner", new GenomeAlignerTask::Factory, guiFactory, true);
    bool res = registry->registerAlgorithm(algo);
    Q_UNUSED(res);
    assert(res);

    LocalWorkflow::GenomeAlignerWorkerFactory::init();
    LocalWorkflow::GenomeAlignerBuildWorkerFactory::init();
    LocalWorkflow::GenomeAlignerIndexReaderWorkerFactory::init();
}

GenomeAlignerPlugin::~GenomeAlignerPlugin() {
    DnaAssemblyAlgRegistry* registry = AppContext::getDnaAssemblyAlgRegistry();
    DnaAssemblyAlgorithmEnv* algo = registry->unregisterAlgorithm(GenomeAlignerTask::taskName);
    assert(algo!=NULL);
    delete algo;
}


} //namespace
