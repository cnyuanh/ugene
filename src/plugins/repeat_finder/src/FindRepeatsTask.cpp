#include "FindRepeatsTask.h"
#include "RFBase.h"
#include "RFDiagonal.h"
#include "RFConstants.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/Log.h>
#include <U2Core/Timer.h>
#include <U2Core/Counter.h>

#include <U2Core/GObjectUtils.h>

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/CreateAnnotationTask.h>
#include <U2Core/TextUtils.h>

namespace U2 {

RevComplSequenceTask::RevComplSequenceTask(const DNASequence& s, const U2Region& reg) 
: Task(tr("Reverse complement sequence"), TaskFlag_None), sequence(s), region(reg)
{
}

void RevComplSequenceTask::run() {
    DNATranslation* complT = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(sequence.alphabet);
    if (complT == NULL) {
        stateInfo.setError(tr("Can't find complement translation for alphabet: %1").arg(sequence.alphabet->getId()));
        return;
    }
    complementSequence.alphabet = complT->getDstAlphabet();
    complementSequence.seq.resize(region.length);
    const char* src = sequence.constData();
    char* dst = complementSequence.seq.data();
    
    complT->translate(src + region.startPos, region.length, dst, region.length);
    TextUtils::reverse(dst, region.length);
}

void RevComplSequenceTask::cleanup() {
    sequence.seq.clear();
    complementSequence.seq.clear();
}


FindRepeatsTask::FindRepeatsTask(const FindRepeatsTaskSettings& s, const DNASequence& seq, const DNASequence& seq2) 
: Task(tr("Find repeats in a single sequence"), TaskFlags_FOSCOE), settings(s), directSequence(seq), directSequence2(seq2)
{
    GCOUNTER( cvar, tvar, "FindRepeatsTask" );
    if (settings.seqRegion.length == 0) {
        settings.seqRegion= U2Region(0, directSequence.length());
    }
    if (seq.constData() == seq2.constData()) {
        settings.seq2Region = settings.seqRegion;
    }
    else {
        settings.seq2Region = U2Region(0, directSequence2.length());
    }

    if (settings.maxDist == 0) {
        settings.maxDist = seq.length();
    }
    revComplTask = NULL;
    rfTask = NULL;
    startTime = GTimer::currentTimeMicros();
    if (settings.inverted) {
        stateInfo.setStateDesc(tr("Rev-complementing sequence"));
        assert(directSequence.alphabet->isNucleic());
        revComplTask = new RevComplSequenceTask(directSequence, settings.seqRegion);
        revComplTask->setSubtaskProgressWeight(0);
        addSubTask(revComplTask);
    } else {
        rfTask = createRFTask();
        addSubTask(rfTask);
    }
}

QList<Task*> FindRepeatsTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if (hasErrors() || isCanceled()) {
        return res;
    }
    if (subTask == revComplTask) {
        startTime = GTimer::currentTimeMicros();
        rfTask = createRFTask();
        res.append(rfTask);
    } 
    return res;
}

RFAlgorithmBase* FindRepeatsTask::createRFTask() {
    stateInfo.setStateDesc(tr("Searching repeats ..."));

    const char* seqX = directSequence.constData() + settings.seqRegion.startPos;
    const char* seqY = revComplTask == NULL ? seqX : revComplTask->complementSequence.constData();
    int seqXLen = settings.seqRegion.length;
    int seqYLen = settings.seqRegion.length;

    if (directSequence.constData() != directSequence2.constData()) {
        seqY = directSequence2.constData();
        seqYLen = directSequence2.length();
    }
    RFAlgorithmBase* t = RFAlgorithmBase::createTask(this, seqX, seqXLen, seqY, seqYLen,
        directSequence.alphabet, settings.minLen, settings.mismatches, settings.algo, settings.nThreads);

    t->setReportReflected(settings.reportReflected);
    return t;
}

void FindRepeatsTask::run() {
    if (settings.filterNested) {
        stateInfo.setStateDesc(tr("Filtering nested results"));
        filterNestedRepeats();
    }
}

Task::ReportResult FindRepeatsTask::report() {
    stateInfo.setStateDesc("");
    if (hasErrors()) {
        return ReportResult_Finished;
    }
    quint64 endTime = GTimer::currentTimeMicros();
    perfLog.details(tr("Repeat search time %1 sec").arg((endTime-startTime)/(1000*1000.0)));
    return ReportResult_Finished;
}

void FindRepeatsTask::filterNestedRepeats() {
    //if one repeats fits into another repeat -> filter it
    quint64 t1 = GTimer::currentTimeMicros();

    qSort(results);
    bool changed = false;
    int extraLen = settings.mismatches; //extra len added to repeat region to search for duplicates
    for (int i=0, n = results.size(); i < n; i++) {
        RFResult& ri = results[i];
        if (ri.l == -1) { //this result was filtered
            continue;
        }
        for (int j=i+1; j < n; j++) {
            RFResult& rj = results[j];
            assert(rj.x >= ri.x);
            if (rj.l == -1) {//was filtered 
                continue;
            }
            if (rj.x > ri.x + ri.l) { //no more intersection will found with later repeats in first region
                break;
            }

            U2Region ri1(ri.x, ri.l), ri2(ri.y, ri.l), rj1(rj.x, rj.l), rj2(rj.y, rj.l);
            bool filteri = false, filterj = false;
            if (rj.l > ri.l) {
                rj1.startPos-=extraLen; rj1.length+=2*extraLen;
                rj2.startPos-=extraLen; rj2.length+=2*extraLen;
                filteri = rj1.contains(ri1) && rj2.contains(ri2);
            } else {
                ri1.startPos-=extraLen; ri1.length+=2*extraLen;
                ri2.startPos-=extraLen; ri2.length+=2*extraLen;
                filterj = ri1.contains(rj1) && ri2.contains(rj2);
            }
            if (filteri || filterj) {
                changed = true;
                if (filteri) {
                    ri.l = -1;
                    break;
                } else {
                    rj.l = -1;
                }
            }
        }
    }
    int nBefore = results.size();
    if (changed) {
        QVector<RFResult> prev = results;
        results.clear();
        foreach(const RFResult& r, prev) {
            if (r.l!=-1) {
                results.append(r);
            }
        }
    }
    int nAfter = results.size();
    quint64 t2 = GTimer::currentTimeMicros();
    perfLog.details(tr("Nested repeats filtering time %1 sec, results before: %2, filtered: %3, after %4")
        .arg(double((t2-t1))/(1000*1000)).arg(nBefore).arg(nBefore - nAfter).arg(nAfter));
}

void FindRepeatsTask::cleanup() {
    directSequence.seq.clear();
    results.clear();
}

void FindRepeatsTask::addResult(const RFResult& r) {
    int x = r.x + settings.seqRegion.startPos;
    int y = settings.inverted ? settings.seqRegion.endPos() - r.y - r.l : r.y + settings.seq2Region.startPos;
    assert(x >= settings.seqRegion.startPos && x + r.l <= settings.seqRegion.endPos());
    assert(y >= settings.seq2Region.startPos && y + r.l <= settings.seq2Region.endPos());

    int dist = qAbs(x - y) - r.l;
    if (dist < settings.minDist || dist > settings.maxDist) {
        return;
    }

    if (settings.reportReflected || x <= y) {
        results.append(RFResult(x, y, r.l));
    } else {
        results.append(RFResult(y, x, r.l));
    }
}

void FindRepeatsTask::onResult(const RFResult& r) {
    if (settings.hasRegionFilters() && isFilteredByRegions(r)) {
        return;
    }
    QMutexLocker ml(&resultsLock);
    addResult(r);
}

void FindRepeatsTask::onResults(const QVector<RFResult>& results) {
    QVector<RFResult> filteredResults = results;
    if (settings.hasRegionFilters()) {
        filteredResults.clear();
        foreach(const RFResult& r, results) {
            if (!isFilteredByRegions(r)) {
                filteredResults.append(r);
            }
        }
    }
    QMutexLocker ml(&resultsLock);
    foreach(const RFResult& r, filteredResults) {
        addResult(r);
    }
}

bool FindRepeatsTask::isFilteredByRegions(const RFResult& r) {
    int x1 = r.x + settings.seqRegion.startPos;
    int y1 = settings.inverted ? settings.seqRegion.endPos() - r.y - 1 : r.y + settings.seqRegion.startPos;
    
    if (x1 > y1) {
        qSwap(x1, y1);
    }
    int x2 = x1 + r.l;
    int y2 = y1 + r.l;

    //check mid range includes
    if (!settings.midRegionsToInclude.isEmpty()) {
        bool checkOk = false;
        foreach(const U2Region& r, settings.midRegionsToInclude) {
            if (r.startPos >= x2 && r.endPos() <= y1) {
                checkOk = true;
                break;
            }
        }
        if (!checkOk) {
            return true;
        }
    }
    
    //check mid range excludes
    if (!settings.midRegionsToExclude.isEmpty()) {
        foreach(const U2Region& r, settings.midRegionsToExclude) {
            if (r.intersects(U2Region(x1, y2-x1))) {
                return true;
            }
        }
    }
    
    //check allowed regions 
    if (!settings.allowedRegions.isEmpty()) {
        bool checkOk = false;
        foreach(const U2Region& r, settings.allowedRegions) {
            if (r.startPos <= x1 && r.endPos() >= y2) {
                checkOk = true;
                break;
            }
        }
        if (!checkOk) {
            return true;
        }
    }
    return false;
}

FindRepeatsToAnnotationsTask::FindRepeatsToAnnotationsTask(const FindRepeatsTaskSettings& s, const DNASequence& seq, 
                             const QString& _an, const QString& _gn, const GObjectReference& _aor)
: Task(tr("Find repeats to annotations"), TaskFlags_NR_FOSCOE), annName(_an), annGroup(_gn), annObjRef(_aor), findTask(NULL)
{
    setVerboseLogMode(true);
    if (annObjRef.isValid()) {
        LoadUnloadedDocumentTask::addLoadingSubtask(this, 
            LoadDocumentTaskConfig(true, annObjRef, new LDTObjectFactory(this)));
    }
    addSubTask(findTask = new FindRepeatsTask(s, seq, seq));
}

QList<Task*> FindRepeatsToAnnotationsTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if (hasErrors() || isCanceled()) {
        return res;
    }

    if (subTask == findTask && annObjRef.isValid()) {
        QList<SharedAnnotationData> annotations = importAnnotations();
        if (!annotations.isEmpty()) {
            algoLog.info(tr("Found %1 repeat regions").arg(annotations.size()));
            Task* createTask = new CreateAnnotationsTask(annObjRef, annGroup, annotations);
            createTask->setSubtaskProgressWeight(0);
            res.append(createTask);
        }
    }
    return res;
}

QList<SharedAnnotationData> FindRepeatsToAnnotationsTask::importAnnotations() {
    QList<SharedAnnotationData> res;
    foreach(const RFResult& r, findTask->getResults()) {
        SharedAnnotationData ad(new AnnotationData());
        ad->name = annName;
        U2Region l1(r.x, r.l);
        U2Region l2(r.y, r.l);
        if (l1.startPos <= l2.startPos) {
            ad->location->regions << l1 << l2;
        } else {
            ad->location->regions << l2 << l1;
        }
        int dist = qAbs(r.x - r.y) - r.l;
        ad->qualifiers.append(U2Qualifier("repeat_len", QString::number(r.l)));
            ad->qualifiers.append(U2Qualifier("repeat_dist", QString::number(dist)));
        if (findTask->getSettings().inverted) {
            ad->qualifiers.append(U2Qualifier("rpt_type", "inverted"));
        }
        res.append(ad);
    }
    return res;
}

}//namespace

