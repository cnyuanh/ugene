#include <QtCore/QFileInfo>

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/Counter.h>
#include <U2Core/Log.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentObject.h>
#include <gobject/uHMMObject.h>

#include "uhmm3build.h"
#include "uHMM3BuildTask.h"

#define UHMM3_BUILD_LOG_CAT "hmm3_build_log_category"

using namespace U2;

static QList< GObject* > getDocObjects( const QList< P7_HMM* >& hmms ) {
    QList< GObject* > res;
    foreach( P7_HMM* hmm, hmms ) {
        res.append( new UHMMObject( hmm, QString( hmm->name ) ) );
    }
    return res;
}

static Document * getSavingDocument( const QList< P7_HMM* >& hmms, const QString & outfile ) {
    assert( !hmms.isEmpty() );
    QList< GObject* > docObjects = getDocObjects( hmms );
    UHMMFormat* hmmFrmt = qobject_cast< UHMMFormat* >
        ( AppContext::getDocumentFormatRegistry()->getFormatById( UHMMFormat::UHHMER_FORMAT_ID ) );
    assert( NULL != hmmFrmt );
    
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( BaseIOAdapters::url2io( outfile ) );
    assert( NULL != iof );
    
    return new Document( hmmFrmt, iof, outfile, docObjects, QVariantMap() );
}

namespace U2 {

static Logger log( UHMM3_BUILD_LOG_CAT );

/**********************************
 * UHMM3BuildTask
 ***********************************/

UHMM3BuildTask::UHMM3BuildTask( const UHMM3BuildSettings& aset, const MAlignment & amsa )
: Task("", TaskFlag_None), settings( aset ), msa( amsa ), hmm( NULL ) {
    GCOUNTER( cvar, tvar, "UHMM3BuildTask" );
    setTaskName( tr( "Build HMM profile from %1 alignment" ).arg( msa.getName() ) );
    checkMsa();
    
    // work with task resources
    float msaSzInMB = ( msa.getLength() * msa.getNumRows() ) / ( 1024.0 * 1024 );
    int power = ( 0 <= msaSzInMB && msaSzInMB <= 0.5 ) ? 20 : ( 0.5 < msaSzInMB && msaSzInMB <= 1 ) ? 10 : 
        ( 1 < msaSzInMB && msaSzInMB <= 10 ) ? 7 : ( 10 < msaSzInMB && msaSzInMB <= 30 ) ? 5 : 4;
    int howManyMem = qMax( 1, (int)( power * msaSzInMB ) );
    taskResources.append( TaskResourceUsage( RESOURCE_MEMORY, howManyMem ) );
    log.trace( QString( "%1 needs %2 of memory" ).arg( getTaskName() ).arg( howManyMem ) );
}

bool UHMM3BuildTask::checkMsa() {
    if( msa.getNumRows() == 0 ) {
        stateInfo.setError( tr("multiple_alignment_is_empty") );
        return false;
    } else if ( msa.getLength() == 0 ) {
        stateInfo.setError(  tr("multiple_alignment_is_0_len") );
        return false;
    }
    return true;
}

void UHMM3BuildTask::delHmm() {
    if( NULL != hmm ) {
        p7_hmm_Destroy( hmm );
    }
    hmm = NULL;
}

UHMM3BuildTask::~UHMM3BuildTask() {
    delHmm();
}

P7_HMM * UHMM3BuildTask::getHMM() const {
    return hmm;
}

P7_HMM * UHMM3BuildTask::takeHMM() {
    P7_HMM * ret = hmm;
    hmm = NULL;
    return ret;
}

void UHMM3BuildTask::run() {
    hmm = UHMM3Build::build( msa, settings, stateInfo );
    if( stateInfo.hasErrors() ) {
        delHmm();
    }
}

/**********************************
* UHMM3BuildTaskSettings
***********************************/

UHMM3BuildTaskSettings::UHMM3BuildTaskSettings( const QString& out ) : outFile( out ) {
    setDefaultUHMM3BuildSettings( &inner );
}

/**********************************
* UHMM3BuildToFileTask
***********************************/

static QList< MAlignment > getMalignments( const QList< GObject* >& objList );

UHMM3BuildToFileTask::UHMM3BuildToFileTask( const UHMM3BuildTaskSettings& s, const QList< MAlignment >& m )
: Task( "", TaskFlags_NR_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled ),
  settings( s ), msas( m ), loadTask( NULL ), saveHmmFileTask( NULL ), savingDocument( NULL ) {
    
    setTaskName( tr( "Build HMM profile to '%1'" ).arg( QFileInfo( settings.outFile ).fileName() ) );
    
    if( settings.outFile.isEmpty() ) {
        stateInfo.setError( tr( "no_output_file_given" ) );
        return;
    }
    
    if( msas.isEmpty() ) {
        stateInfo.setError( tr( "empty_msa_list_given" ) );
        return;
    }
    
    createBuildSubtasks();
    addBuildSubTasks();
}

UHMM3BuildToFileTask::UHMM3BuildToFileTask( const UHMM3BuildTaskSettings& set, const MAlignment& ma )
: Task( "", TaskFlags_NR_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled ), 
  settings( set ), loadTask( NULL ), saveHmmFileTask( NULL ), savingDocument( NULL ) {
    
    setTaskName( tr( "Build HMM profile to '%1'" ).arg( QFileInfo( settings.outFile ).fileName() ) );
    
    if( settings.outFile.isEmpty() ) {
        stateInfo.setError( tr( "no_output_file_given" ) );
        return;
    }

    msas.append( ma );
    createBuildSubtasks();
    addBuildSubTasks();
}


UHMM3BuildToFileTask::UHMM3BuildToFileTask( const UHMM3BuildTaskSettings& set, const QString& _inFile ) 
: Task( "", TaskFlags_NR_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled ),
settings( set ), inFile( _inFile ), loadTask( NULL ), saveHmmFileTask( NULL ), savingDocument( NULL ) {

    setTaskName( tr( "Build HMM profile '%1' -> '%2'" ).arg( QFileInfo( inFile ).fileName() ).arg( QFileInfo( settings.outFile ).fileName() ) );

    if( inFile.isEmpty() ) {
        stateInfo.setError( tr( "no_input_file_given" ) );
        return;
    }
    if( settings.outFile.isEmpty() ) {
        stateInfo.setError( tr( "no_output_file_given" ) );
        return;
    }

    DocumentFormatConstraints constr;
	constr.supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
	constr.checkRawData = true;
    constr.rawData = BaseIOAdapters::readFileHeader( inFile );
    QList<DocumentFormatId> formats = AppContext::getDocumentFormatRegistry()->selectFormats( constr );
    if( formats.isEmpty() ) {
        stateInfo.setError( tr( "input_format_error" ) );
        return;
    }
    DocumentFormatId alnFormat = formats.first();
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( BaseIOAdapters::url2io( inFile ) );

    if( NULL == iof ) {
        stateInfo.setError( tr( "cannot_create_io_adapter_for_%1_file" ).arg( inFile ) );
        return;
    }

    loadTask = new LoadDocumentTask( alnFormat, inFile, iof, QVariantMap() );
    addSubTask( loadTask );
}

void UHMM3BuildToFileTask::createBuildSubtasks() {
    foreach( const MAlignment & ma, msas ) {
        UHMM3BuildTask * curTask = new UHMM3BuildTask( settings.inner, ma );
        buildTasks << curTask;
    }
}

void UHMM3BuildToFileTask::addBuildSubTasks() {
    assert( !buildTasks.isEmpty() );
    foreach( UHMM3BuildTask * cur, buildTasks ) {
        addSubTask( cur );
    }
}

QList< Task* > UHMM3BuildToFileTask::onSubTaskFinished( Task* sub ) {
    QMutexLocker locker( &mtx );
    QList< Task* > res;
    
    assert( NULL != sub );
    if( hasErrors() ) {
        return res;
    }
    if( sub->hasErrors() ) {
        stateInfo.setError( sub->getError() );
        return res;
    }
    
    if( loadTask == sub ) {
        assert( msas.isEmpty() );
        
        Document* doc = loadTask->getDocument();
        QList< GObject* > msaObjs = doc->findGObjectByType( GObjectTypes::MULTIPLE_ALIGNMENT );
        if( msaObjs.isEmpty() ) {
            stateInfo.setError( tr( "alignment_objects_not_found_in_document" ) );
            return res;
        }
        
        msas = getMalignments( msaObjs );
        createBuildSubtasks();
        foreach( UHMM3BuildTask * cur, buildTasks ) {
            assert( NULL != cur );
            res << cur;
        }
        return res;
    } else if( buildTasks.contains( qobject_cast< UHMM3BuildTask* >(sub) ) ) {
        UHMM3BuildTask * curBuildTask = qobject_cast< UHMM3BuildTask* >( sub );
        assert( NULL != curBuildTask );
        int howMany = buildTasks.removeAll( curBuildTask );
        assert( 1 == howMany );
        
        if( curBuildTask->hasErrors() || curBuildTask->isCanceled() ) {
            return res; /* nothing to do */
        }
        
        P7_HMM * hmm = curBuildTask->takeHMM();
        assert( NULL != hmm );
        hmms.append( hmm );
        
        if( buildTasks.isEmpty() ) { /* all build tasks had finished */
            assert( !hmms.isEmpty() );
            savingDocument = getSavingDocument( hmms, settings.outFile );
            saveHmmFileTask = new SaveDocumentTask( savingDocument );    
            res << saveHmmFileTask;
        }
        return res;
    } else if( saveHmmFileTask == sub ) {
        assert( NULL != savingDocument );
        delete savingDocument;
    } else {
        assert( 0 );
    }
    
    return res;
}

QString UHMM3BuildToFileTask::generateReport() const {
    QString res;
    
    res += "<table>";
    if( !inFile.isEmpty() ) {
        res += "<tr><td width=200><b>" + tr("Source alignment") + "</b></td><td>" + inFile + "</td></tr>";
    }
    res += "<tr><td><b>" + tr("Profile name") + "</b></td><td>" + settings.outFile + "</td></tr>";
    
    const UHMM3BuildSettings & bldSettings = settings.inner;
    
    res += "<tr><td><b>" + tr( "Options:" ) + "</b></td></tr>";
    res += "<tr><td><b>" + tr( "Model construction strategies" ) + "</b></td><td>";
    switch( bldSettings.archStrategy ) {
        case p7_ARCH_FAST: res += "fast"; break;
        case p7_ARCH_HAND: res += "hand"; break;
        default: assert( false );
    }
    res += "</td></tr>";
    
    res += "<tr><td><b>" + tr( "Relative model construction strategies" ) + "</b></td><td>";
    switch( bldSettings.wgtStrategy ) {
        case p7_WGT_GSC:    res += tr("Gerstein/Sonnhammer/Chothia tree weights"); break;
        case p7_WGT_BLOSUM: res += tr("Henikoff simple filter weights" ); break;
        case p7_WGT_PB:     res += tr("Henikoff position-based weights" ); break;
        case p7_WGT_NONE:   res += tr("No relative weighting; set all to 1" ); break;
        case p7_WGT_GIVEN:  res += tr("Weights given in MSA file" ); break;
        default: assert( false );
    }
    res += "</td></tr>";
    
    res += "<tr><td><b>" + tr( "Effective sequence weighting strategies" ) + "</b></td><td>";
    switch( bldSettings.effnStrategy ) {
        case p7_EFFN_ENTROPY:   res += tr( "adjust effective sequence number to achieve relative entropy target" ); break;
        case p7_EFFN_CLUST:     res += tr( "effective sequence number is number of single linkage clusters" ); break;
        case p7_EFFN_NONE:      res += tr( "no effective sequence number weighting: just use number of sequences" ); break;
        case p7_EFFN_SET:       res += tr( "set effective sequence number for all models to: %1" ).arg( bldSettings.eset ); break;
        default: assert( false );
    }
    res += "</td></tr>";

    if( hasErrors() ) {
        res += "<tr><td width=200><b>" + tr( "Task finished with error: '%1'" ).arg( getError() ) + "</b></td><td></td></tr>";
    }
    res += "</table>";
    
    return res;
}

static QList< MAlignment > getMalignments( const QList< GObject* >& objList ) {
    QList< MAlignment > res;
    
    foreach( GObject* obj, objList ) {
        MAlignmentObject* msaObj = qobject_cast< MAlignmentObject* >( obj );
        assert( NULL != msaObj );
        res << msaObj->getMAlignment();
    }
    return res;
}

} // U2
