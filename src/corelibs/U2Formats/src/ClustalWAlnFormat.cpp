#include "ClustalWAlnFormat.h"

#include <U2Formats/DocumentFormatUtils.h>

#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/MSAUtils.h>

#include <U2Algorithm/MSAConsensusUtils.h>
#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>
#include <U2Algorithm/BuiltInConsensusAlgorithms.h>

#include <memory>

namespace U2 {

/* TRANSLATOR U2::ClustalWAlnFormat */    
/* TRANSLATOR U2::IOAdapter */    

const QByteArray ClustalWAlnFormat::CLUSTAL_HEADER = "CLUSTAL";

ClustalWAlnFormat::ClustalWAlnFormat(QObject* p) : DocumentFormat(p, DocumentFormatFlags_SW, QStringList("aln")) 
{
    formatName = tr("CLUSTALW");
    supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
}


void ClustalWAlnFormat::load(IOAdapter* io, QList<GObject*>& objects, TaskStateInfo& ti) {
    static int READ_BUFF_SIZE = 1024;
    QByteArray readBuffer(READ_BUFF_SIZE, '\0');
    char* buff  = readBuffer.data();

    const QBitArray& LINE_BREAKS = TextUtils::LINE_BREAKS;
    const QBitArray& WHITES = TextUtils::WHITES;
    MAlignment al( io->getURL().baseFileName());
    bool lineOk = false;
    bool firstBlock = true;
    int sequenceIdx = 0;
    int valStartPos = 0;
    int valEndPos = 0;

    //1 skip first line
    int len = io->readUntil(buff, READ_BUFF_SIZE, LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
    if (!lineOk || !readBuffer.startsWith( CLUSTAL_HEADER )) {
        ti.setError( ClustalWAlnFormat::tr("Illegal header line"));
    }

    //read data
    while (!ti.cancelFlag && (len = io->readUntil(buff, READ_BUFF_SIZE, LINE_BREAKS, IOAdapter::Term_Include, &lineOk)) > 0) {
        if( QByteArray::fromRawData( buff, len ).startsWith( CLUSTAL_HEADER ) ) {
            io->skip( -len );
            break;
        }
        int numNs = 0;
        while(len > 0 && LINE_BREAKS[(uchar)buff[len-1]]) {
            if ( buff[len-1] =='\n') {
                numNs++;
            }
            len--;
        }
        if (len == 0) {
            if (al.getNumRows() == 0) {
                continue;//initial empty lines
            }
            ti.setError( ClustalWAlnFormat::tr("Error parsing file"));
            break;
        }
        
        QByteArray line = QByteArray::fromRawData( buff, len );
        if (valStartPos == 0) {
            int spaceIdx = line.indexOf(' ');
            int valIdx = spaceIdx + 1;
            while (valIdx < len && WHITES[(uchar)buff[valIdx]]) {
                valIdx++;
            }
            if (valIdx <= 0 || valIdx >= len-1) {
                ti.setError( ClustalWAlnFormat::tr("Invalid alignment format"));
                break;
            }
            valStartPos = valIdx;
        }

        valEndPos = valStartPos + 1; //not inclusive
        while (valEndPos < len && !WHITES[(uchar)buff[valEndPos]]) {
            valEndPos++;
        }
        if (valEndPos!=len) { //there were numbers trimmed -> trim spaces now
            while (valEndPos > valStartPos && buff[valEndPos] == ' ') {
                valEndPos--;
            }
            valEndPos++; //leave non-inclusive
        }

        QByteArray name = line.left(valStartPos).trimmed();
        QByteArray value = line.mid(valStartPos, valEndPos - valStartPos);
                
        int seqsInModel = al.getNumRows();
        bool lastBlockLine = (!firstBlock && sequenceIdx == seqsInModel) 
            || numNs >=2
            || name.isEmpty()
            || value.contains(' ') || value.contains(':') || value.contains('.');

        if (firstBlock) {
            if (lastBlockLine && name.isEmpty()) { //if name is not empty -> this is a sequence but consensus (for Clustal files without consensus)
                // this is consensus line - skip it
            } else {
                assert(al.getNumRows() == sequenceIdx);
                al.addRow(MAlignmentRow(name, value));
            }
        } else {
            int rowIdx = -1;
            if (sequenceIdx < seqsInModel) { 
                rowIdx = sequenceIdx;
            } else if (sequenceIdx == seqsInModel) {
                assert(lastBlockLine);
                // consensus line
            } else {
                ti.setError( ClustalWAlnFormat::tr("Incorrect number of sequences in block"));
                break;
            } 
            if (rowIdx != -1) {
                const MAlignmentRow& row = al.getRow(rowIdx);
                if (row.getName() != name) {
                    ti.setError( ClustalWAlnFormat::tr("Sequence names are not matched"));
                    break;
                }
                al.appendChars(rowIdx, value.constData(), value.size());
            }
        }
        if (lastBlockLine) {
            firstBlock = false;
            if (!MSAUtils::checkPackedModelSymmetry(al, ti)) {
                break;
            }
            sequenceIdx = 0;
        } else {
            sequenceIdx++;
        }

        ti.progress = io->getProgress();
    }
    MSAUtils::checkPackedModelSymmetry(al, ti);
    if (ti.hasErrors()) {
        return;
    }
    DocumentFormatUtils::assignAlphabet(al);
    if (al.getAlphabet() == NULL) {
        ti.setError( ClustalWAlnFormat::tr("Alphabet is unknown"));
        return;
    }

    MAlignmentObject* obj = new MAlignmentObject(al);
    objects.append(obj);
}

Document* ClustalWAlnFormat::loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode) {
    QList<GObject*> objects;
    load(io, objects, ti);
    
    if (ti.hasErrors()) {
        qDeleteAll( objects );
        return NULL;
    }
    assert(objects.size() == 1);
    return new Document(this, io->getFactory(), io->getURL(), objects, fs);
}

#define MAX_LINE_LEN    80
#define MAX_NAME_LEN    39
#define SEQ_ALIGNMENT    5

void ClustalWAlnFormat::save(IOAdapter* io, Document* d, TaskStateInfo& ti) {
    const MAlignmentObject* obj = NULL;
    if( (d->getObjects().size() != 1)
        || ((obj = qobject_cast<const MAlignmentObject*>(d->getObjects().first())) == NULL)) {
            ti.setError("No data to write;");
            return;
    }
    const MAlignment& ma = obj->getMAlignment();
    
    //write header
    QByteArray header("CLUSTAL W 2.0 multiple sequence alignment\n\n");
    int len = io->writeBlock(header);
    if (len != header.length()) {
        ti.setError(L10N::errorWritingFile(d->getURL()));
        return;
    }

    //precalculate seq writing params
    int maxNameLength = 0;
    foreach(const MAlignmentRow& row, ma.getRows()) {
        maxNameLength = qMax(maxNameLength, row.getName().length());
    }
    maxNameLength = qMin(maxNameLength, MAX_NAME_LEN);

    int aliLen = ma.getLength();
    QByteArray consensus(aliLen, MAlignment_GapChar);

    MSAConsensusAlgorithmFactory* algoFactory = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(BuiltInConsensusAlgorithms::CLUSTAL_ALGO);
    std::auto_ptr<MSAConsensusAlgorithm> algo(algoFactory->createAlgorithm(ma));
    MSAConsensusUtils::updateConsensus(ma, consensus, algo.get());
    
    int maxNumLength  = 1 + (aliLen < 10 ? 1 : (int)log10((double)aliLen));

    int seqStart = maxNameLength + 2; //+1 for space separator
    if (seqStart % SEQ_ALIGNMENT != 0) {
        seqStart = seqStart + SEQ_ALIGNMENT - (seqStart % SEQ_ALIGNMENT);
    }
    int seqEnd = MAX_LINE_LEN - maxNumLength - 1;
    if (seqEnd % SEQ_ALIGNMENT != 0) {
        seqEnd = seqEnd - (seqEnd % SEQ_ALIGNMENT);
    }
    assert(seqStart % SEQ_ALIGNMENT == 0 && seqEnd % SEQ_ALIGNMENT == 0 && seqEnd > seqStart);

    int seqPerPage = seqEnd - seqStart;
    const char* spaces = TextUtils::SPACE_LINE.constData();

    //write sequence
    for(int i = 0; i < aliLen; i+=seqPerPage) {
        int partLen = i + seqPerPage > aliLen ? aliLen - i : seqPerPage;
        foreach(const MAlignmentRow& row, ma.getRows()) {
            QByteArray line = row.getName().toAscii();
            if (line.length() > MAX_NAME_LEN) {
                line = line.left(MAX_NAME_LEN);
            }
            TextUtils::replace(line.data(), line.length(), TextUtils::WHITES, '_');
            line.append(QByteArray::fromRawData(spaces, seqStart - line.length()));
            line.append(row.mid(i, partLen).toByteArray(partLen));
            line.append(' ');
            line.append(QString::number(qMin(i+seqPerPage, aliLen)));
            assert(line.length() <= MAX_LINE_LEN);
            line.append('\n');

            len = io->writeBlock(line);
            if (len != line.length()) {
                ti.setError(L10N::errorWritingFile(d->getURL()));
                return;
            }
        }
        //write consensus
        QByteArray line = QByteArray::fromRawData(spaces, seqStart);
        line.append(consensus.mid(i, partLen));
        line.append("\n\n");
        len = io->writeBlock(line);
        if (len != line.length()) {
            ti.setError(L10N::errorWritingFile(d->getURL()));
            return;
        }
    }
}

void ClustalWAlnFormat::storeDocument( Document* d, TaskStateInfo& ti, IOAdapter* io ) {
    if( NULL == d ) {
        ti.setError(L10N::badArgument("doc"));
        return;
    }
    if( NULL == io || !io->isOpen() ) {
        ti.setError(L10N::badArgument("IO adapter"));
        return;
    }
    save(io, d, ti);
}

FormatDetectionResult  ClustalWAlnFormat::checkRawData(const QByteArray& data, const GUrl&) const {
    if (TextUtils::contains(TextUtils::BINARY, data.constData(), data.size())) {
        return FormatDetection_NotMatched;
    }
    if (!data.startsWith(CLUSTAL_HEADER)) {
        return FormatDetection_NotMatched;
    }
    QTextStream s(data);
    QString line = s.readLine();
    if ( (line == CLUSTAL_HEADER) || (line.endsWith("multiple sequence alignment")) ) {
        return FormatDetection_Matched;
    }
    return FormatDetection_AverageSimilarity;
}

}//namespace
