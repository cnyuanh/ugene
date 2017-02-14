/**
* UGENE - Integrated Bioinformatics Tools.
* Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QThread>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppResources.h>
#include <U2Core/ChromatogramUtils.h>
#include <U2Core/DNASequenceUtils.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GenbankFeatures.h>
#include <U2Core/L10n.h>
#include <U2Core/MultipleChromatogramAlignment.h>
#include <U2Core/MultipleChromatogramAlignmentImporter.h>
#include <U2Core/MultipleChromatogramAlignmentObject.h>

#include "BlastReadsSubTask.h"
#include "ComposeResultSubTask.h"

namespace U2 {
namespace Workflow {

namespace {
    qint64 calcMemUsageBytes(DbiDataStorage *storage, const SharedDbiDataHandler &seqId, U2OpStatus &os) {
        QScopedPointer<U2SequenceObject> object(StorageUtils::getSequenceObject(storage, seqId));
        CHECK_EXT(!object.isNull(), os.setError(L10N::nullPointerError("Sequence object")), 0);

        return object->getSequenceLength();
    }

    int toMb(qint64 bytes) {
        return 0.5 + (double(bytes) / (1024 * 1024));
    }
}

/************************************************************************/
/* ComposeResultSubTask */
/************************************************************************/
ComposeResultSubTask::ComposeResultSubTask(const SharedDbiDataHandler &reference,
                                           const QList<SharedDbiDataHandler> &reads,
                                           const QList<BlastAndSwReadTask*> subTasks,
                                           DbiDataStorage *storage)
    : Task(tr("Compose alignment"), TaskFlags_FOSE_COSC),
      reference(reference),
      reads(reads),
      subTasks(subTasks),
      storage(storage),
      mcaObject(NULL)
{
    tpm = Task::Progress_Manual;
}

ComposeResultSubTask::~ComposeResultSubTask() {
    delete mcaObject;
}

void ComposeResultSubTask::prepare() {
    qint64 memUsage = calcMemUsageBytes(storage, reference, stateInfo);
    CHECK_OP(stateInfo, );

    foreach (const SharedDbiDataHandler &read, reads) {
        memUsage += calcMemUsageBytes(storage, read, stateInfo);
        CHECK_OP(stateInfo, );
    }
    if (toMb(memUsage) > 0) {
        addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, toMb(memUsage), false));
    }
}

void ComposeResultSubTask::run() {
    createAlignmentAndAnnotations();
    CHECK_OP(stateInfo, );
}

const SharedDbiDataHandler& ComposeResultSubTask::getAnnotations() const {
    return annotations;
}

QList<GObject*> ComposeResultSubTask::getResult() {
    QList<GObject*> result;

    MultipleChromatogramAlignmentObject *alignment = mcaObject;
    mcaObject = NULL;
    if (alignment->thread() != QThread::currentThread()) {
        alignment->moveToThread(QThread::currentThread());
    }

    U2SequenceObject* reference = StorageUtils::getSequenceObject(storage, this->reference);
    CHECK_EXT(reference != NULL, setError(L10N::nullPointerError("Reference sequence")), result);

    result << alignment;
    result << reference ;

    return result;
}

void ComposeResultSubTask::createAlignmentAndAnnotations() {
    MultipleChromatogramAlignment result("Aligned reads");

    DNASequence referenceSeq = getReferenceSequence();
    CHECK_OP(stateInfo, );
    result->setAlphabet(referenceSeq.alphabet);

    // add the reference row
    // TODO: store the reference as a separate sequence with gap model
//    result->addRow(referenceSeq.getName(), referenceSeq.seq, 0);
//    CHECK_OP(stateInfo, );

    U2MsaRowGapModel referenceGaps = getReferenceGaps();
    CHECK_OP(stateInfo, );

//    insertShiftedGapsIntoReference(result, referenceGaps);
//    CHECK_OP(stateInfo, );

    // initialize annotations table on reference
    QScopedPointer<AnnotationTableObject> annsObject(new AnnotationTableObject(referenceSeq.getName() + " features", storage->getDbiRef()));
    QList<SharedAnnotationData> anns;

    int rowsCounter = 0;
    for (int i = 0; i < reads.size(); i++) {
        BlastAndSwReadTask *subTask = getBlastSwTask(i);
        CHECK_OP(stateInfo, );
        if (!subTask->isReadAligned()) {
            continue;
        }

        // add the read row
        DNASequence readSeq = getReadSequence(i);
        CHECK_OP(stateInfo, );

        DNAChromatogram readChromatogram = getReadChromatogram(i);
        CHECK_OP(stateInfo, );

        result->addRow(subTask->getReadName(), readChromatogram, readSeq.seq);
        CHECK_OP(stateInfo, );

        foreach (const U2MsaGap &gap, subTask->getReadGaps()) {
            result->insertGaps(rowsCounter, gap.offset, gap.gap, stateInfo);
            CHECK_OP(stateInfo, );
        }

        // add reference gaps to the read
        insertShiftedGapsIntoRead(result, i, rowsCounter, referenceGaps);
        CHECK_OP(stateInfo, );

        // add read annotation to the reference
        const MultipleChromatogramAlignmentRow readRow = result->getMcaRow(rowsCounter);
        U2Region region = getReadRegion(readRow, referenceGaps);
        SharedAnnotationData ann(new AnnotationData);
        ann->location = getLocation(region, subTask->isComplement());
        ann->name = GBFeatureUtils::getKeyInfo(GBFeatureKey_misc_feature).text;
        ann->qualifiers << U2Qualifier("label", subTask->getReadName());
        anns.append(ann);

        ++rowsCounter;
    }
    if (rowsCounter == 0) {
        stateInfo.setError(tr("No read satisfy minimum identity criteria."));
        return;
    }
    result->trim(false); // just recalculates alignment len

    mcaObject = MultipleChromatogramAlignmentImporter::createAlignment(stateInfo, storage->getDbiRef(), U2ObjectDbi::ROOT_FOLDER, result);
    CHECK_OP(stateInfo, );
    // remove gap columns
    // TODO: implement the method and restoer code
//    mcaObject->deleteColumnWithGaps(stateInfo, GAP_COLUMN_ONLY);

    annsObject->addAnnotations(anns);
    annotations = storage->getDataHandler(annsObject->getEntityRef());
}

U2Region ComposeResultSubTask::getReadRegion(const MultipleChromatogramAlignmentRow &readRow, const U2MsaRowGapModel &referenceGapModel) const {
    U2Region region(0, readRow->getRowLengthWithoutTrailing());

    // calculate read start
    if (!readRow->getGapModel().isEmpty()) {
        U2MsaGap firstGap = readRow->getGapModel().first();
        if (0 == firstGap.offset) {
            region.startPos += firstGap.gap;
            region.length -= firstGap.gap;
        }
    }

    qint64 leftGap = 0;
    qint64 innerGap = 0;
    foreach (const U2MsaGap &gap, referenceGapModel) {
        qint64 endPos = gap.offset + gap.gap;
        if (gap.offset < region.startPos) {
            leftGap += gap.gap;
        } else if (endPos <= region.endPos()) {
            innerGap += gap.gap;
        } else {
            break;
        }
    }

    region.startPos -= leftGap;
    region.length -= innerGap;
    return region;
}

U2Location ComposeResultSubTask::getLocation(const U2Region &region, bool isComplement) {
    U2Location result;
    result->strand = isComplement ? U2Strand(U2Strand::Complementary) : U2Strand(U2Strand::Direct);

    if (region.startPos < 0) {
        // TODO: just trim the region because it is incorrect to make the annotation circular: the left (negative) part of the read is not aligned.
        // Fix it when the task can work with circular references.
        result->regions << U2Region(0, region.length + region.startPos);
    } else {
        result->regions << region;
    }

    return result;
}

BlastAndSwReadTask * ComposeResultSubTask::getBlastSwTask(int readNum) {
    CHECK_EXT(readNum < subTasks.size(), setError(L10N::internalError("Wrong reads number")), NULL);
    return subTasks[readNum];
}

DNASequence ComposeResultSubTask::getReadSequence(int readNum) {
    BlastAndSwReadTask *subTask = getBlastSwTask(readNum);
    CHECK_OP(stateInfo, DNASequence());

    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, subTask->getRead()));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), DNASequence());
    DNASequence seq = readObject->getWholeSequence(stateInfo);
    CHECK_OP(stateInfo, DNASequence());
    if (subTask->isComplement()) {
        seq.seq = DNASequenceUtils::reverseComplement(seq.seq);
    }
    return seq;
}

DNAChromatogram ComposeResultSubTask::getReadChromatogram(int readNum) {
    BlastAndSwReadTask *subTask = getBlastSwTask(readNum);
    CHECK_OP(stateInfo, DNAChromatogram());

    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, subTask->getRead()));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), DNAChromatogram());

    const U2EntityRef chromatogramRef = ChromatogramUtils::getChromatogramIdByRelatedSequenceId(stateInfo, readObject->getEntityRef());
    CHECK_OP(stateInfo, DNAChromatogram());
    CHECK_EXT(chromatogramRef.isValid(), setError(tr("The related chromatogram not found")), DNAChromatogram());

    DNAChromatogram chromatogram = ChromatogramUtils::exportChromatogram(stateInfo, chromatogramRef);
    CHECK_OP(stateInfo, DNAChromatogram());

    if (subTask->isComplement()) {
        chromatogram = ChromatogramUtils::reverseComplement(chromatogram);
    }
    return chromatogram;

}

DNASequence ComposeResultSubTask::getReferenceSequence() {
    QScopedPointer<U2SequenceObject> refObject(StorageUtils::getSequenceObject(storage, reference));
    CHECK_EXT(!refObject.isNull(), setError(L10N::nullPointerError("Reference sequence")), DNASequence());
    DNASequence seq = refObject->getWholeSequence(stateInfo);
    CHECK_OP(stateInfo, DNASequence());
    return seq;
}

namespace {
    bool compare(const U2MsaGap &gap1, const U2MsaGap &gap2) {
        return gap1.offset < gap2.offset;
    }
}

U2MsaRowGapModel ComposeResultSubTask::getReferenceGaps() {
    U2MsaRowGapModel result;

    for (int i=0; i<reads.size(); i++) {
        result << getShiftedGaps(i);
        CHECK_OP(stateInfo, result);
    }
    qSort(result.begin(), result.end(), compare);
    return result;
}

U2MsaRowGapModel ComposeResultSubTask::getShiftedGaps(int rowNum) {
    U2MsaRowGapModel result;

    BlastAndSwReadTask *subTask = getBlastSwTask(rowNum);
    CHECK_OP(stateInfo, result);

    qint64 wholeGap = 0;
    foreach (const U2MsaGap &gap, subTask->getReferenceGaps()) {
        result << U2MsaGap(gap.offset - wholeGap, gap.gap);
        wholeGap += gap.gap;
    }
    return result;
}

void ComposeResultSubTask::insertShiftedGapsIntoReference(MultipleChromatogramAlignment &alignment, const U2MsaRowGapModel &gaps) {
    for (int i = gaps.size() - 1; i >= 0; i--) {
        U2MsaGap gap = gaps[i];
        alignment->insertGaps(0, gap.offset, gap.gap, stateInfo);
        CHECK_OP(stateInfo, );
    }
}

void ComposeResultSubTask::insertShiftedGapsIntoRead(MultipleChromatogramAlignment &alignment, int readNum, int rowNum, const U2MsaRowGapModel &gaps) {
    U2MsaRowGapModel ownGaps = getShiftedGaps(readNum);
    CHECK_OP(stateInfo, );

    qint64 globalOffset = 0;
    foreach (const U2MsaGap &gap, gaps) {
        if (ownGaps.contains(gap)) { // task own gaps into account but don't insert them
            globalOffset += gap.gap;
            ownGaps.removeOne(gap);
            continue;
        }
        alignment->insertGaps(rowNum, globalOffset + gap.offset, gap.gap, stateInfo);
        CHECK_OP(stateInfo, );
        globalOffset += gap.gap;
    }
}

} // namespace Workflow
} // namespace U2
