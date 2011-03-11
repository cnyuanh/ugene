include (U2Core.pri)

# Input
HEADERS += src/cmdline/CMDLineCoreOptions.h \
           src/cmdline/CMDLineHelpProvider.h \
           src/cmdline/CMDLineRegistry.h \
           src/cmdline/CMDLineUtils.h \
           src/datatype/AnnotationData.h \
           src/datatype/AnnotationSettings.h \
           src/datatype/BioStruct3D.h \
           src/datatype/DIProperties.h \
           src/datatype/DNAAlphabet.h \
           src/datatype/DNAAlphabetRegistryImpl.h \
           src/datatype/DNAAlphabetUtils.h \
           src/datatype/DNAChromatogram.h \
           src/datatype/DNAInfo.h \
           src/datatype/DNAQuality.h \
           src/datatype/DNASequence.h \
           src/datatype/DNATranslation.h \
           src/datatype/DNATranslationImpl.h \
           src/datatype/FeatureColors.h \
           src/datatype/MAlignment.h \
           src/datatype/MAlignmentInfo.h \
           src/datatype/Matrix44.h \
           src/datatype/PFMatrix.h \
           src/datatype/PhyTree.h \
           src/datatype/PWMatrix.h \
           src/datatype/SMatrix.h \
           src/datatype/UIndex.h \
           src/datatype/Vector3D.h \
           src/dbi/DbiDocumentFormat.h \
           src/dbi/U2SqlHelpers.h \
           src/globals/AppContext.h \
           src/globals/AppGlobalObject.h \
           src/globals/AppResources.h \
           src/globals/AppSettings.h \
           src/globals/AutoAnnotationsSupport.h \
           src/globals/BaseDocumentFormats.h \
           src/globals/Counter.h \
           src/globals/DataBaseRegistry.h \
           src/globals/DBXRefRegistry.h \
           src/globals/DocumentFormatConfigurators.h \
           src/globals/ExternalToolRegistry.h \
           src/globals/GAutoDeleteList.h \
           src/globals/global.h \
           src/globals/Identifiable.h \
           src/globals/IdRegistry.h \
           src/globals/L10n.h \
           src/globals/Log.h \
           src/globals/LogCache.h \
           src/globals/NetworkConfiguration.h \
           src/globals/PluginModel.h \
           src/globals/ProjectService.h \
           src/globals/ResourceTracker.h \
           src/globals/ServiceModel.h \
           src/globals/ServiceTypes.h \
           src/globals/Settings.h \
           src/globals/Task.h \
           src/globals/Timer.h \
           src/globals/UserApplicationsSettings.h \
           src/globals/Version.h \
           src/gobjects/AnnotationTableObject.h \
           src/gobjects/AssemblyObject.h \
           src/gobjects/BioStruct3DObject.h \
           src/gobjects/DNAChromatogramObject.h \
           src/gobjects/DNASequenceObject.h \
           src/gobjects/GObjectRelationRoles.h \
           src/gobjects/GObjectTypes.h \
           src/gobjects/GObjectUtils.h \
           src/gobjects/MAlignmentObject.h \
           src/gobjects/PhyTreeObject.h \
           src/gobjects/TextObject.h \
           src/gobjects/UIndexObject.h \
           src/gobjects/UnloadedObject.h \
           src/io/GUrl.h \
           src/io/GUrlUtils.h \
           src/io/HttpFileAdapter.h \
           src/io/IOAdapter.h \
           src/io/LocalFileAdapter.h \
           src/io/RingBuffer.h \
           src/io/VFSAdapter.h \
           src/io/VirtualFileSystem.h \
           src/io/ZlibAdapter.h \
           src/models/DocumentModel.h \
           src/models/DocumentUtils.h \
           src/models/GHints.h \
           src/models/GObject.h \
           src/models/GObjectReference.h \
           src/models/ProjectModel.h \
           src/models/StateLockableDataModel.h \
           src/selection/AnnotationSelection.h \
           src/selection/DNASequenceSelection.h \
           src/selection/DocumentSelection.h \
           src/selection/GObjectSelection.h \
           src/selection/LRegionsSelection.h \
           src/selection/SelectionModel.h \
           src/selection/SelectionTypes.h \
           src/selection/SelectionUtils.h \
           src/selection/TextSelection.h \
           src/tasks/AddDocumentTask.h \
           src/tasks/AddPartToSequenceTask.h \
           src/tasks/AddSequencesToAlignmentTask.h \
           src/tasks/CopyDataTask.h \
           src/tasks/CreateAnnotationTask.h \
           src/tasks/CreateFileIndexTask.h \
           src/tasks/ExportToNewFileFromIndexTask.h \
           src/tasks/ExtractAnnotatedRegionTask.h \
           src/tasks/FailTask.h \
           src/tasks/GetDocumentFromIndexTask.h \
           src/tasks/LoadDocumentTask.h \
           src/tasks/LoadRemoteDocumentTask.h \
           src/tasks/MultiTask.h \
           src/tasks/RemoveDocumentTask.h \
           src/tasks/RemovePartFromSequenceTask.h \
           src/tasks/ReplacePartOfSequenceTask.h \
           src/tasks/SaveDocumentStreamingTask.h \
           src/tasks/SaveDocumentTask.h \
           src/tasks/ScriptTask.h \
           src/tasks/SequenceWalkerTask.h \
           src/tasks/TaskSignalMapper.h \
           src/tasks/TaskStarter.h \
           src/tasks/TLSTask.h \
           src/util_algorithm/MSAUtils.h \
           src/util_algorithm/QVariantUtils.h \
           src/util_algorithm/SequenceUtils.h \
           src/util_text/FormatUtils.h \
           src/util_text/TextUtils.h
SOURCES += src/cmdline/CMDLineCoreOptions.cpp \
           src/cmdline/CMDLineRegistry.cpp \
           src/cmdline/CMDLineUtils.cpp \
           src/datatype/AnnotationData.cpp \
           src/datatype/AnnotationSettings.cpp \
           src/datatype/BaseAlphabets.cpp \
           src/datatype/BaseTranslations.cpp \
           src/datatype/BioStruct3D.cpp \
           src/datatype/DNAAlphabet.cpp \
           src/datatype/DNAAlphabetRegistryImpl.cpp \
           src/datatype/DNAInfo.cpp \
           src/datatype/DNAQuality.cpp \
           src/datatype/DNASequence.cpp \
           src/datatype/DNATranslation.cpp \
           src/datatype/DNATranslationImpl.cpp \
           src/datatype/FeatureColors.cpp \
           src/datatype/MAlignment.cpp \
           src/datatype/MAlignmentInfo.cpp \
           src/datatype/Matrix44.cpp \
           src/datatype/PFMatrix.cpp \
           src/datatype/PhyTree.cpp \
           src/datatype/PWMatrix.cpp \
           src/datatype/SMatrix.cpp \
           src/datatype/U2Region.cpp \
           src/datatype/UIndex.cpp \
           src/datatype/Vector3D.cpp \
           src/dbi/DbiDocumentFormat.cpp \
           src/dbi/U2DbiRegistry.cpp \
           src/dbi/U2DbiUtils.cpp \
           src/dbi/U2SqlHelpers.cpp \
           src/globals/AppContext.cpp \
           src/globals/AppGlobalObject.cpp \
           src/globals/AppResources.cpp \
           src/globals/AutoAnnotationsSupport.cpp \
           src/globals/BaseDocumentFormats.cpp \
           src/globals/Counter.cpp \
           src/globals/DataBaseRegistry.cpp \
           src/globals/DBXRefRegistry.cpp \
           src/globals/DocumentFormatConfigurators.cpp \
           src/globals/ExternalToolRegistry.cpp \
           src/globals/GAutoDeleteList.cpp \
           src/globals/Log.cpp \
           src/globals/LogCache.cpp \
           src/globals/NetworkConfiguration.cpp \
           src/globals/PluginModel.cpp \
           src/globals/ProjectService.cpp \
           src/globals/ResourceTracker.cpp \
           src/globals/ServiceModel.cpp \
           src/globals/Task.cpp \
           src/globals/Timer.cpp \
           src/globals/UserApplicationsSettings.cpp \
           src/globals/Version.cpp \
           src/gobjects/AnnotationTableObject.cpp \
           src/gobjects/AssemblyObject.cpp \
           src/gobjects/BioStruct3DObject.cpp \
           src/gobjects/DNAChromatogramObject.cpp \
           src/gobjects/DNASequenceObject.cpp \
           src/gobjects/GObjectRelationRoles.cpp \
           src/gobjects/GObjectTypes.cpp \
           src/gobjects/GObjectUtils.cpp \
           src/gobjects/MAlignmentObject.cpp \
           src/gobjects/PhyTreeObject.cpp \
           src/gobjects/TextObject.cpp \
           src/gobjects/UIndexObject.cpp \
           src/gobjects/UnloadedObject.cpp \
           src/io/GUrl.cpp \
           src/io/GUrlUtils.cpp \
           src/io/HttpFileAdapter.cpp \
           src/io/IOAdapter.cpp \
           src/io/LocalFileAdapter.cpp \
           src/io/VFSAdapter.cpp \
           src/io/VirtualFileSystem.cpp \
           src/io/ZlibAdapter.cpp \
           src/models/DocumentModel.cpp \
           src/models/DocumentUtils.cpp \
           src/models/GHints.cpp \
           src/models/GObject.cpp \
           src/models/ProjectModel.cpp \
           src/models/StateLockableDataModel.cpp \
           src/selection/AnnotationSelection.cpp \
           src/selection/DocumentSelection.cpp \
           src/selection/GObjectSelection.cpp \
           src/selection/LRegionsSelection.cpp \
           src/selection/SelectionModel.cpp \
           src/selection/SelectionTypes.cpp \
           src/selection/SelectionUtils.cpp \
           src/tasks/AddDocumentTask.cpp \
           src/tasks/AddPartToSequenceTask.cpp \
           src/tasks/AddSequencesToAlignmentTask.cpp \
           src/tasks/CopyDataTask.cpp \
           src/tasks/CreateAnnotationTask.cpp \
           src/tasks/CreateFileIndexTask.cpp \
           src/tasks/ExportToNewFileFromIndexTask.cpp \
           src/tasks/ExtractAnnotatedRegionTask.cpp \
           src/tasks/GetDocumentFromIndexTask.cpp \
           src/tasks/LoadDocumentTask.cpp \
           src/tasks/LoadRemoteDocumentTask.cpp \
           src/tasks/MultiTask.cpp \
           src/tasks/RemoveDocumentTask.cpp \
           src/tasks/RemovePartFromSequenceTask.cpp \
           src/tasks/ReplacePartOfSequenceTask.cpp \
           src/tasks/SaveDocumentStreamingTask.cpp \
           src/tasks/SaveDocumentTask.cpp \
           src/tasks/ScriptTask.cpp \
           src/tasks/SequenceWalkerTask.cpp \
           src/tasks/TaskSignalMapper.cpp \
           src/tasks/TaskStarter.cpp \
           src/tasks/TLSTask.cpp \
           src/util_algorithm/MSAUtils.cpp \
           src/util_algorithm/QVariantUtils.cpp \
           src/util_algorithm/SequenceUtils.cpp \
           src/util_algorithm/U2AnnotationUtils.cpp \
           src/util_algorithm/U2AssemblyUtils.cpp \
           src/util_algorithm/U2Bits.cpp \
           src/util_text/FormatUtils.cpp \
           src/util_text/TextUtils.cpp
TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
