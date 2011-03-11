#include "BlastQuery.h"

#include <U2Designer/DelegateEditors.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/L10n.h>
#include <U2Core/FailTask.h>
#include <U2Lang/BaseTypes.h>

namespace U2 {

static const QString UNIT_ID = "CDD";

const QString EXPECT("evalue");
const QString MAX_HITS("max-hits");
const QString MIN_RES_LEN("min-length");
const QString MAX_RES_LEN("max-length");
static const QString QUAL_ATTR = "pattern";

QDCDDActor::QDCDDActor(QDActorPrototype const* proto) : QDActor(proto) {
    cfg->setAnnotationKey("CDD result");
    units[UNIT_ID] = new QDSchemeUnit(this);
}

int QDCDDActor::getMaxResultLen() const {
    return cfg->getParameter(MAX_RES_LEN)->getAttributeValue<int>();
}

int QDCDDActor::getMinResultLen() const {
    return cfg->getParameter(MIN_RES_LEN)->getAttributeValue<int>();
}

QString QDCDDActor::getText() const {
    return tr("Searches through the NCBI CDD database for annotations.");;
}

Task* QDCDDActor::getAlgorithmTask(const QVector<U2Region>& location) {
    DNASequenceObject* dna = scheme->getDNA();

    settings.dbChoosen = "cdd";
    settings.params = "db=cdd"; //to do: supply db choice

    int evalue = cfg->getParameter(EXPECT)->getAttributeValue<int>();
    int maxHits = 500;//cfg->getParameter(MAX_HITS)->getAttributeValue<int>();
    addParametr(settings.params, ReqParams::cdd_hits, maxHits);
    addParametr(settings.params, ReqParams::cdd_eValue, evalue);

    settings.retries = 60;

    DNAAlphabet* alph = dna->getAlphabet();
    settings.complT = GObjectUtils::findComplementTT(dna);
    settings.aminoT = NULL;
    if (!alph->isAmino()) {
        DNATranslationType tt;
        if (alph->getType() == DNAAlphabet_NUCL) {
            tt = DNATranslationType_NUCL_2_AMINO;
        } else {
            tt = DNATranslationType_RAW_2_AMINO;
        }
        QList<DNATranslation*> TTs = AppContext::getDNATranslationRegistry()->lookupTranslation(alph, tt);
        if (!TTs.isEmpty()) {
            settings.aminoT = TTs.first();
        } else {
            return new FailTask(tr("Bad sequence."));
        }
    }

    Task* t = new Task(tr("CDD Search"), TaskFlag_NoRun);
    foreach(const U2Region& r, location) {
        RemoteBLASTTaskSettings s(settings);
        QByteArray query(dna->getSequence().constData() + r.startPos, r.length);
        s.query = query;
        RemoteBLASTTask* sub = new RemoteBLASTTask(s);
        t->addSubTask(sub);
        offsetMap[sub] = r.startPos;
    }
    connect(new TaskSignalMapper(t),SIGNAL(si_taskFinished(Task*)),SLOT(sl_onAlgorithmTaskFinished()));

    return t;
}

void QDCDDActor::sl_onAlgorithmTaskFinished() {
    QList<SharedAnnotationData> res;
    QMapIterator<RemoteBLASTTask*, int> iter(offsetMap);
    while (iter.hasNext()) {
        iter.next();
        RemoteBLASTTask* rqt = iter.key();
        QList<SharedAnnotationData> annotations = rqt->getResultedAnnotations();
        //shift by offset
        int offset = offsetMap.value(rqt);
        QMutableListIterator<SharedAnnotationData> annIter(annotations);
        while (annIter.hasNext()) {
            QVector<U2Region>& location = annIter.next()->location->regions;
            U2Region::shift(offset, location);
        }
        res << annotations;
    }
    offsetMap.clear();

    int minLen = cfg->getParameter(MIN_RES_LEN)->getAttributeValue<int>();
    int maxLen = cfg->getParameter(MAX_RES_LEN)->getAttributeValue<int>();
    const QString& qualVal = cfg->getParameter(QUAL_ATTR)->getAttributeValue<QString>();
    foreach(const SharedAnnotationData& ad, res) {
        const U2Region& reg = ad->location->regions.first();
        if (reg.length < minLen || reg.length > maxLen) {
            continue;
        }
        foreach(const U2Qualifier& qual, ad->qualifiers) {
            if (qual.value.contains(qualVal)) {
                QDResultUnit ru(new QDResultUnitData);
                ru->strand = ad->getStrand();
                ru->quals = ad->qualifiers;
                ru->region = reg;
                ru->owner = units.values().first();
                QDResultGroup* g = new QDResultGroup(QDStrand_Both);
                g->add(ru);
                results.append(g);
                break;
            }
        }
    }
}

QDCDDActorPrototype::QDCDDActorPrototype() {
    descriptor.setId("CDD");
    descriptor.setDisplayName(QDCDDActor::tr("CDD"));
    descriptor.setDocumentation(QDCDDActor::tr("Finds annotations for DNA sequences in a remote database"));

    Descriptor evalue(EXPECT,QDCDDActor::tr("Expected value"),
        QDCDDActor::tr("This parameter specifies the statistical significance threshold of reporting matches against the database sequences."));
    Descriptor qual(QUAL_ATTR, QDCDDActor::tr("Pattern"), QDCDDActor::tr("Include results containing specified value"));
    Descriptor minResLen(MIN_RES_LEN, QDCDDActor::tr("Min length"), QDCDDActor::tr("Minimum result length"));
    Descriptor maxResLen(MAX_RES_LEN, QDCDDActor::tr("Max length"), QDCDDActor::tr("Maximum result length"));

    attributes << new Attribute(evalue, BaseTypes::STRING_TYPE(), false, 10);
    attributes << new Attribute(minResLen, BaseTypes::NUM_TYPE(), false, 50);
    attributes << new Attribute(maxResLen, BaseTypes::NUM_TYPE(), false, 5000);
    attributes << new Attribute(qual, BaseTypes::STRING_TYPE(), true);

    QMap<QString, PropertyDelegate*> delegates; 
    {
        QVariantMap m;
        m["1e-100"] = 1e-100;
        m["1e-10"] = 1e-10;
        m["1"] = 1;
        m["10"] = 10;
        m["100"] = 100;
        m["1000"] = 1000;
        delegates[EXPECT] = new ComboBoxDelegate(m);
    }

    {
        QVariantMap lenMap;
        lenMap["minimum"] = QVariant(0);
        lenMap["maximum"] = QVariant(INT_MAX); 
        lenMap["suffix"] = L10N::suffixBp();
        delegates[MIN_RES_LEN] = new SpinBoxDelegate(lenMap);
        delegates[MAX_RES_LEN] = new SpinBoxDelegate(lenMap);
    }

    editor = new DelegateEditor(delegates);
}

}//namespace
