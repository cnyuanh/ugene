/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2020 UniPro <ugene@unipro.ru>
 * http://ugene.net
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

#include "FindEnzymesTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/CreateAnnotationTask.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GenbankFeatures.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Settings.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2SafePoints.h>

#include "EnzymesIO.h"

namespace U2 {

/* TRANSLATOR U2::FindEnzymesTask */

//////////////////////////////////////////////////////////////////////////
// enzymes -> annotations

FindEnzymesToAnnotationsTask::FindEnzymesToAnnotationsTask(AnnotationTableObject *aobj, const U2EntityRef &seqRef, const QList<SEnzymeData> &enzymes, const FindEnzymesTaskConfig &config)
    : Task(tr("Find and store enzymes"), TaskFlags_NR_FOSCOE), dnaSeqRef(seqRef), enzymes(enzymes), annotationObject(aobj), cfg(config), findTask(nullptr) {
    GCOUNTER(cvar, "FindEnzymesToAnnotationsTask");
}

void FindEnzymesToAnnotationsTask::prepare() {
    CHECK_EXT(!enzymes.isEmpty(), stateInfo.setError(tr("No enzymes selected.")), );
    U2Region searchRegion = cfg.searchRegion;
    if (cfg.searchRegion.isEmpty()) {
        U2SequenceObject sequenceObject("sequence", dnaSeqRef);
        searchRegion = U2Region(0, sequenceObject.getSequenceLength());
    }
    findTask = new FindEnzymesTask(dnaSeqRef, searchRegion, enzymes, cfg.maxResults, cfg.circular, cfg.excludedRegions);
    addSubTask(findTask);
}

QList<Task *> FindEnzymesToAnnotationsTask::onSubTaskFinished(Task *subTask) {
    QList<Task *> result;

    CHECK(subTask == findTask, result);
    CHECK_OP(stateInfo, result);
    CHECK_EXT(!annotationObject.isNull(), stateInfo.setError(tr("Annotation table does not exist")), result);
    CHECK_EXT(!annotationObject->isStateLocked(), stateInfo.setError(tr("Annotation table is read-only")), result);

    bool useSubgroups = enzymes.size() > 1 || cfg.groupName.isEmpty();
    QMap<QString, QList<SharedAnnotationData>> annotationsByGroupMap;
    for (const SEnzymeData &enzyme : enzymes) {
        QList<SharedAnnotationData> resultAnnotationList = findTask->getResultsAsAnnotations(enzyme->id);
        if (resultAnnotationList.size() >= cfg.minHitCount && resultAnnotationList.size() <= cfg.maxHitCount) {
            QString group = useSubgroups ? cfg.groupName + "/" + enzyme->id : cfg.groupName;
            annotationsByGroupMap[group].append(resultAnnotationList);
        }
    }

    result << new CreateAnnotationsTask(annotationObject, annotationsByGroupMap);
    return result;
}

Task::ReportResult FindEnzymesToAnnotationsTask::report() {
    CHECK_OP(stateInfo, ReportResult_Finished);

    if (!annotationObject.isNull() && annotationObject->getAnnotations().isEmpty() && !cfg.isAutoAnnotationUpdateTask) {
        // no results found -> delete empty annotation document
        Project *proj = AppContext::getProject();
        if (proj != nullptr) {
            Document *toDelete = nullptr;
            QList<Document *> docs = proj->getDocuments();
            for (Document *doc : docs) {
                if (doc->getObjects().contains(annotationObject)) {
                    toDelete = doc;
                    break;
                }
            }
            if (toDelete != nullptr) {
                proj->removeDocument(toDelete);
            }
        }
        annotationObject = nullptr;
        stateInfo.setError("Enzymes selection is not found");
    }

    return ReportResult_Finished;
}

//////////////////////////////////////////////////////////////////////////
// find multiple enzymes task
FindEnzymesTask::FindEnzymesTask(const U2EntityRef &seqRef, const U2Region &region, const QList<SEnzymeData> &enzymes, int mr, bool circular, QVector<U2Region> excludedRegions)
    : Task(tr("Find Enzymes"), TaskFlags_NR_FOSCOE),
      maxResults(mr),
      excludedRegions(excludedRegions),
      isCircular(circular),
      seqlen(0),
      countOfResultsInMap(0) {
    U2SequenceObject seq("sequence", seqRef);
    SAFE_POINT(seq.getAlphabet()->isNucleic(), tr("Alphabet is not nucleic."), );
    seqlen = seq.getSequenceLength();
    //for every enzymes in selection create FindSingleEnzymeTask
    foreach (const SEnzymeData &enzyme, enzymes) {
        addSubTask(new FindSingleEnzymeTask(seqRef, region, enzyme, this, circular));
    }
}

void FindEnzymesTask::onResult(int pos, const SEnzymeData &enzyme, const U2Strand &strand) {
    if (pos > seqlen) {
        pos %= seqlen;
    }
    foreach (const U2Region &r, excludedRegions) {
        if (U2Region(pos, enzyme->seq.length()).intersects(r)) {
            return;
        }
    }

    QMutexLocker locker(&resultsLock);
    if (countOfResultsInMap > maxResults) {
        if (!isCanceled()) {
            stateInfo.setError(tr("Number of results exceed %1, stopping").arg(maxResults));
            cancel();
        }
        return;
    }
    searchResultMap[enzyme->id] << FindEnzymesAlgResult(enzyme, pos, strand);
    countOfResultsInMap++;
}

QList<SharedAnnotationData> FindEnzymesTask::getResultsAsAnnotations(const QString &enzymeId) const {
    QList<SharedAnnotationData> res;
    if (hasError() || isCanceled()) {
        return res;
    }
    QString cutStr;
    QString dbxrefStr;
    QList<FindEnzymesAlgResult> searchResultList = searchResultMap.value(enzymeId);
    for (const FindEnzymesAlgResult &searchResult : searchResultList) {
        const SEnzymeData &enzyme = searchResult.enzyme;
        if (!enzyme->accession.isEmpty()) {
            QString accession = enzyme->accession;
            if (accession.startsWith("RB")) {
                accession = accession.mid(2);
            }
            dbxrefStr = "REBASE:" + accession;
        } else if (!enzyme->id.isEmpty()) {
            dbxrefStr = "REBASE:" + enzyme->id;
        }
        if (enzyme->cutDirect != ENZYME_CUT_UNKNOWN) {
            cutStr = QString::number(enzyme->cutDirect);
            if (enzyme->cutComplement != ENZYME_CUT_UNKNOWN && enzyme->cutComplement != enzyme->cutDirect) {
                cutStr += "/" + QString::number(enzyme->cutComplement);
            }
        }
        break;
    }

    for (const FindEnzymesAlgResult &searchResult : searchResultList) {
        const SEnzymeData &enzyme = searchResult.enzyme;
        if (isCircular && searchResult.pos + enzyme->seq.size() > seqlen) {
            if (seqlen < searchResult.pos) {
                continue;
            }
            SharedAnnotationData ad(new AnnotationData);
            ad->type = U2FeatureTypes::RestrictionSite;
            ad->name = enzyme->id;
            qint64 firstRegionLength = seqlen - searchResult.pos;
            if (firstRegionLength != 0) {
                ad->location->regions << U2Region(searchResult.pos, firstRegionLength);
            }
            ad->location->regions << U2Region(0, enzyme->seq.size() - firstRegionLength);
            ad->setStrand(searchResult.strand);
            if (!dbxrefStr.isEmpty()) {
                ad->qualifiers.append(U2Qualifier("db_xref", dbxrefStr));
            }
            if (!cutStr.isEmpty()) {
                ad->qualifiers.append(U2Qualifier(GBFeatureUtils::QUALIFIER_CUT, cutStr));
            }
            res.append(ad);
        } else {
            SharedAnnotationData ad(new AnnotationData);
            ad->type = U2FeatureTypes::RestrictionSite;
            ad->name = enzyme->id;
            ad->location->regions << U2Region(searchResult.pos, enzyme->seq.size());
            ad->setStrand(searchResult.strand);
            if (!dbxrefStr.isEmpty()) {
                ad->qualifiers.append(U2Qualifier("db_xref", dbxrefStr));
            }
            if (!cutStr.isEmpty()) {
                ad->qualifiers.append(U2Qualifier(GBFeatureUtils::QUALIFIER_CUT, cutStr));
            }
            res.append(ad);
        }
    }
    return res;
}

Task::ReportResult FindEnzymesTask::report() {
    if (!hasError() && !isCanceled()) {
        algoLog.info(tr("Found %1 restriction sites").arg(countOfResultsInMap));
    }
    return ReportResult_Finished;
}

void FindEnzymesTask::cleanup() {
    if (hasError()) {
        searchResultMap.clear();
    }
}

//////////////////////////////////////////////////////////////////////////
// find single enzyme task
FindSingleEnzymeTask::FindSingleEnzymeTask(const U2EntityRef &sequenceObjectRef, const U2Region &region, const SEnzymeData &enzyme, FindEnzymesAlgListener *l, bool isCircular, int maxResults)
    : Task(tr("Find enzyme '%1'").arg(enzyme->id), TaskFlag_NoRun),
      sequenceObjectRef(sequenceObjectRef),
      region(region),
      enzyme(enzyme),
      maxResults(maxResults),
      resultListener(l),
      isCircular(isCircular) {
    U2SequenceObject dnaSeq("sequence", sequenceObjectRef);

    SAFE_POINT(dnaSeq.getAlphabet()->isNucleic(), tr("Alphabet is not nucleic."), );
    if (resultListener == nullptr) {
        resultListener = this;
    }

    const int BLOCK_READ_FROM_DB = 128000;
    static const int chunkSize = BLOCK_READ_FROM_DB;

    SequenceDbiWalkerConfig sequenceWalkerConfig;
    sequenceWalkerConfig.seqRef = sequenceObjectRef;
    sequenceWalkerConfig.range = region;
    sequenceWalkerConfig.chunkSize = qMax(enzyme->seq.size(), chunkSize);
    sequenceWalkerConfig.lastChunkExtraLen = sequenceWalkerConfig.chunkSize / 2;
    sequenceWalkerConfig.overlapSize = enzyme->seq.size() - 1;
    sequenceWalkerConfig.walkCircular = isCircular;
    sequenceWalkerConfig.walkCircularDistance = sequenceWalkerConfig.overlapSize;

    addSubTask(new SequenceDbiWalkerTask(sequenceWalkerConfig, this, tr("Find enzyme '%1' parallel").arg(enzyme->id)));
}

void FindSingleEnzymeTask::onResult(int pos, const SEnzymeData &enzyme, const U2Strand &strand) {
    if (isCircular && pos >= region.length) {
        return;
    }
    QMutexLocker locker(&resultsLock);
    if (resultList.size() > maxResults) {
        if (!isCanceled()) {
            stateInfo.setError(FindEnzymesTask::tr("Number of results exceed %1, stopping").arg(maxResults));
            cancel();
        }
        return;
    }
    resultList.append(FindEnzymesAlgResult(enzyme, pos, strand));
}

void FindSingleEnzymeTask::onRegion(SequenceDbiWalkerSubtask *t, TaskStateInfo &ti) {
    if (enzyme->seq.isEmpty()) {
        return;
    }
    U2SequenceObject dnaSequenceObject("sequence", sequenceObjectRef);
    qint64 sequenceLen = dnaSequenceObject.getSequenceLength();
    if (sequenceLen < enzyme->seq.length()) {
        return;
    }
    SAFE_POINT(enzyme->alphabet != nullptr, tr("No enzyme alphabet"), );
    if (!enzyme->alphabet->isNucleic()) {
        algoLog.info(tr("Non-nucleic enzyme alphabet: %1, enzyme: %2, skipping..").arg(enzyme->alphabet->getId()).arg(enzyme->id));
        return;
    }

    const DNAAlphabet *seqAlphabet = dnaSequenceObject.getAlphabet();
    SAFE_POINT(seqAlphabet != nullptr, tr("Failed to get sequence alphabet"), );

    bool useExtendedComparator = enzyme->alphabet->getId() == BaseDNAAlphabetIds::NUCL_DNA_EXTENDED() || seqAlphabet->getId() == BaseDNAAlphabetIds::NUCL_DNA_EXTENDED() || seqAlphabet->getId() == BaseDNAAlphabetIds::NUCL_RNA_DEFAULT() || seqAlphabet->getId() == BaseDNAAlphabetIds::NUCL_RNA_EXTENDED();

    U2Region chunkRegion = t->getGlobalRegion();
    DNASequence dnaSeq;
    if (U2Region(0, sequenceLen).contains(chunkRegion)) {
        dnaSeq = dnaSequenceObject.getSequence(chunkRegion, ti);
    } else {
        U2Region partOne = U2Region(0, sequenceLen).intersect(chunkRegion);
        dnaSeq = dnaSequenceObject.getSequence(partOne, ti);
        CHECK_OP(ti, );
        U2Region partTwo = U2Region(0, chunkRegion.endPos() % sequenceLen);
        dnaSeq.seq.append(dnaSequenceObject.getSequence(partTwo, ti).seq);
    }
    CHECK_OP(ti, );

    // Note that enzymes algorithm filters N symbols in sequence by itself
    if (useExtendedComparator) {
        FindEnzymesAlgorithm<ExtendedDNAlphabetComparator> algo;
        algo.run(dnaSeq, U2Region(0, chunkRegion.length), enzyme, resultListener, ti, chunkRegion.startPos);
    } else {
        FindEnzymesAlgorithm<ExactDNAAlphabetComparatorN1M_N2M> algo;
        algo.run(dnaSeq, U2Region(0, chunkRegion.length), enzyme, resultListener, ti, chunkRegion.startPos);
    }
}

void FindSingleEnzymeTask::cleanup() {
    resultList.clear();
}

//////////////////////////////////////////////////////////////////////////
// find enzymes auto annotation updater

FindEnzymesAutoAnnotationUpdater::FindEnzymesAutoAnnotationUpdater()
    : AutoAnnotationsUpdater(tr("Restriction Sites"), ANNOTATION_GROUP_ENZYME) {
}

Task *FindEnzymesAutoAnnotationUpdater::createAutoAnnotationsUpdateTask(const AutoAnnotationObject *annotationObject) {
    QList<SEnzymeData> defaultEnzymeList = EnzymesIO::getDefaultEnzymesList();
    QString selectedEnzymesString = AppContext::getSettings()->getValue(EnzymeSettings::LAST_SELECTION).toString();
    if (selectedEnzymesString.isEmpty()) {
        selectedEnzymesString = EnzymeSettings::COMMON_ENZYMES;
    }

    QStringList selectedEnzymeIdList = selectedEnzymesString.split(ENZYME_LIST_SEPARATOR);
    QList<SEnzymeData> selectedEnzymes;
    for (const QString &id : selectedEnzymeIdList) {
        for (const SEnzymeData &enzyme : defaultEnzymeList) {
            if (id == enzyme->id) {
                selectedEnzymes.append(enzyme);
            }
        }
    }

    FindEnzymesTaskConfig cfg;
    cfg.circular = annotationObject->getSequenceObject()->isCircular();
    cfg.groupName = getGroupName();
    cfg.isAutoAnnotationUpdateTask = true;
    cfg.minHitCount = AppContext::getSettings()->getValue(EnzymeSettings::MIN_HIT_VALUE, 1).toInt();
    cfg.maxHitCount = AppContext::getSettings()->getValue(EnzymeSettings::MAX_HIT_VALUE, INT_MAX).toInt();
    cfg.maxResults = AppContext::getSettings()->getValue(EnzymeSettings::MAX_RESULTS, 500000).toInt();

    U2Region savedSearchRegion = AppContext::getSettings()->getValue(EnzymeSettings::SEARCH_REGION, QVariant::fromValue(U2Region())).value<U2Region>();

    U2SequenceObject *sequenceObject = annotationObject->getSequenceObject();
    U2Region wholeSequenceRegion(0, sequenceObject->getSequenceLength());
    if (cfg.circular) {
        //In circular mode the region can have an overflow to handle end/start positions correctly
        cfg.searchRegion = U2Region(savedSearchRegion.startPos, qMin(savedSearchRegion.length, wholeSequenceRegion.length));
    } else {
        cfg.searchRegion = savedSearchRegion.intersect(wholeSequenceRegion);
    }
    if (cfg.searchRegion.isEmpty()) {
        cfg.searchRegion = wholeSequenceRegion;
    }

    QVector<U2Region> excludedRegions =
        AppContext::getSettings()->getValue(EnzymeSettings::EXCLUDED_REGION, QVariant::fromValue(QVector<U2Region>())).value<QVector<U2Region>>();

    if (!excludedRegions.isEmpty()) {
        cfg.excludedRegions = excludedRegions;
    }

    AnnotationTableObject *annotationTableObject = annotationObject->getAnnotationObject();
    const U2EntityRef &sequenceObjectRef = annotationObject->getSequenceObject()->getEntityRef();
    return new FindEnzymesToAnnotationsTask(annotationTableObject, sequenceObjectRef, selectedEnzymes, cfg);
}

bool FindEnzymesAutoAnnotationUpdater::checkConstraints(const AutoAnnotationConstraints &constraints) {
    return constraints.alphabet == nullptr ? false : constraints.alphabet->isNucleic();
}

}    // namespace U2
