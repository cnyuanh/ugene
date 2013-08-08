/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "ConvertFilesFormatWorker.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/FailTask.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>

namespace U2 {
namespace LocalWorkflow {

const QString ConvertFilesFormatWorkerFactory::ACTOR_ID("files-conversion");
static const QString SHORT_NAME( "cff" );
static const QString INPUT_PORT( "in-file" );
static const QString OUTPUT_PORT( "out-file" );
static const QString OUTPUT_SUBDIR( "Converted files/" );

QString ConvertFilesFormatPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(INPUT_PORT));
    const Actor* producer = input->getProducer(BaseSlots::URL_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);

    QString doc = tr("Convert sequences %1 to selected format").arg(producerName);
    return doc;
}

void getFormatsByType( const GObjectType &type, QVariantMap& m ) {
    DocumentFormatConstraints constr;
    constr.supportedObjectTypes.insert( type );
    constr.addFlagToSupport(DocumentFormatFlag_SupportWriting);

    const QList<DocumentFormatId> supportedFormats = AppContext::getDocumentFormatRegistry()->selectFormats( constr );
    foreach( const DocumentFormatId & fid, supportedFormats ) {
        m[fid] = fid;
    }
}

void ConvertFilesFormatWorkerFactory::init() {
    QList<PortDescriptor*> p; 
    Descriptor ind( INPUT_PORT, ConvertFilesFormatWorker::tr("File"), 
                                                ConvertFilesFormatWorker::tr("A file to perform format conversion"));
    Descriptor outd( OUTPUT_PORT, ConvertFilesFormatWorker::tr("File"), 
                                                ConvertFilesFormatWorker::tr("File of selected format"));
    QMap<Descriptor, DataTypePtr> inM;
    inM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
    p << new PortDescriptor(ind, DataTypePtr(new MapDataType(SHORT_NAME + ".input-url", inM)), true);
    QMap<Descriptor, DataTypePtr> outM;
    outM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
    p << new PortDescriptor(outd, DataTypePtr(new MapDataType(SHORT_NAME + ".output-url", outM)), false, true);

    QList<Attribute*> a; 
    a << new Attribute( BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE(), BaseTypes::STRING_TYPE(), false, "txt" );
    Descriptor desc( ACTOR_ID, ConvertFilesFormatWorker::tr("File conversion"),
               ConvertFilesFormatWorker::tr("Converts a file into selected format.") );
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);

    QVariantMap m;
    getFormatsByType( GObjectTypes::SEQUENCE, m );
    getFormatsByType( GObjectTypes::TEXT, m );
    getFormatsByType( GObjectTypes::ANNOTATION_TABLE, m );
    getFormatsByType( GObjectTypes::VARIANT_TRACK, m );
    getFormatsByType( GObjectTypes::CHROMATOGRAM, m );
    getFormatsByType( GObjectTypes::MULTIPLE_ALIGNMENT, m );
    getFormatsByType( GObjectTypes::PHYLOGENETIC_TREE, m );
    getFormatsByType( GObjectTypes::BIOSTRUCTURE_3D, m );
    getFormatsByType( GObjectTypes::ASSEMBLY, m );
    
    proto->setEditor(new DelegateEditor(QMap<QString, PropertyDelegate*>()));
    proto->getEditor()->addDelegate( new ComboBoxDelegate(m), BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId() );

    proto->setPrompter(new ConvertFilesFormatPrompter());
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CONVERTERS(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById( LocalDomainFactory::ID );
    localDomain->registerEntry( new ConvertFilesFormatWorkerFactory() );
}

void ConvertFilesFormatWorker::init() {
    inputUrlPort = ports.value(INPUT_PORT);
    outputUrlPort = ports.value(OUTPUT_PORT); 
    selectedFormat = actor->getParameter( BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId() )->getAttributeValue<QString>(context);
}

void ConvertFilesFormatWorker::getWorkingDir( QString &workingDir ) {
    QString destinationURL;
    if( context->hasWorkingDir() ) {
        destinationURL.append( context->workingDir() );
    } else {
        QString tempDir = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath();
        destinationURL.append( tempDir );
    }
    if( !destinationURL.endsWith("/") ) {
        destinationURL.append("/");
    }
    destinationURL.append( OUTPUT_SUBDIR );
    
    QDir dir(destinationURL);
    if( !dir.exists( destinationURL ) ) {
        dir.mkdir( destinationURL );
    }
    workingDir = destinationURL;
}

Task* ConvertFilesFormatWorker::tick() {
    if( inputUrlPort->hasMessage() ) {
         const Message inputMessage = getMessageAndSetupScriptValues(inputUrlPort);
         if( inputMessage.isEmpty() ) {
             outputUrlPort->transit();
             return NULL;
         }
         const QVariantMap qm = inputMessage.getData().toMap();
         const GUrl sourceURL( qm.values().at(0).toString() );
         QList<FormatDetectionResult> formats = DocumentUtils::detectFormat( sourceURL );
         if( formats.empty() ) {
             monitor()->addError( "Undefined file format", getActorId() );
             return NULL;
         }
         if( formats.first().getFormatOrImporterName().toUpper() == selectedFormat.toUpper() ) {
             const Message resultMessage( BaseTypes::STRING_TYPE(), sourceURL.getURLString() );
             outputUrlPort->put( resultMessage );
             return NULL;
         } 

         QString workingDir = QString();
         getWorkingDir( workingDir );

         Task *t = new ConvertFilesFormatTask( sourceURL, selectedFormat, workingDir );
         connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
         return t;
    } else if( inputUrlPort->isEnded() ) {
        setDone();
        outputUrlPort->setEnded();
    }
    return NULL;
}

void ConvertFilesFormatWorker::cleanup() {
}

void ConvertFilesFormatWorker::sl_taskFinished( Task *task ) {
    QList <Task*> subTasks = task->getSubtasks();
    if( subTasks.empty() ) {
        return;
    }
    SaveDocumentTask *sdt = qobject_cast<SaveDocumentTask*>( subTasks.last() );
    QFile outputFile( sdt->getURL().getURLString() );
    if( !outputFile.open(QIODevice::ReadOnly) ) {
        monitor()->addError( "Can not open converted file. Check format is correct", getActorId() );
        return;
    }
    if( !outputFile.size() ) {
        monitor()->addError( "Converted file is empty. Check format is correct", getActorId() );
        return;
    }
    Message resultMessage( BaseTypes::STRING_TYPE(), sdt->getURL().getURLString() );
    outputUrlPort->put(resultMessage);
    monitor()->addOutputFile( sdt->getURL().getURLString(), getActorId() );
}

void ConvertFilesFormatTask::prepare() {
    LoadDocumentTask *t = LoadDocumentTask::getDefaultLoadDocTask( sourceURL );
    this->addSubTask( t );
    isTaskLoadDocument = true;
}

QList<Task*> ConvertFilesFormatTask::onSubTaskFinished( Task *subTask ) {
    if( !isTaskLoadDocument ) {
        QList<Task*> taskList;
        return taskList;
    }
    LoadDocumentTask *ldt = qobject_cast<LoadDocumentTask*>(subTask);
    bool mainThread = false;
    Document *srcDoc = ldt->getDocument( mainThread );

    IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( IOAdapterUtils::url2io( srcDoc->getURL() ) );
    DocumentFormatRegistry *dfr =  AppContext::getDocumentFormatRegistry();
    DocumentFormat *df = dfr->getFormatById(selectedFormat);

    Document *dstDoc = srcDoc->getSimpleCopy( df, iof, srcDoc->getURL() );
    QString fileName = srcDoc->getName();
    fileName.append("." + selectedFormat);
    QString destinationURL = workingDir + fileName;
    
    Task *sdt = new SaveDocumentTask(dstDoc, iof, destinationURL);
    isTaskLoadDocument = false;
    QList<Task*> taskList;
    taskList << sdt;
    return taskList;
}

} //LocalWorkflow
} //U2
