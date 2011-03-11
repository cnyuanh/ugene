#ifndef _U2_FINDALGORITHM_H_
#define _U2_FINDALGORITHM_H_

#include <U2Core/U2Region.h>
#include <U2Core/AnnotationData.h>

#include <QtCore/QList>

namespace U2 {


class U2ALGORITHM_EXPORT FindAlgorithmResult {
public:
    FindAlgorithmResult() : err(0) {}
    FindAlgorithmResult(const U2Region& _r, bool t, U2Strand s, int _err) 
        : region(_r), translation(t), strand(s), err(_err){}
    
    void clear() {region.startPos = 0; region.length = 0; translation = false; strand = U2Strand::Direct; err = 0;}
    
    bool isEmpty() const {return region.startPos == 0 && region.length == 0;}

    bool operator ==(const FindAlgorithmResult& o) const {
        return region == o.region && err == o.err && strand == o.strand && translation == o.translation;
    }

    SharedAnnotationData toAnnotation(const QString& name) const {
        SharedAnnotationData data;
        data = new AnnotationData;
        data->name = name;
        data->location->regions << region;
        data->setStrand(strand);
        data->qualifiers.append(U2Qualifier("error", QString::number(err)));
        return data;
    }

    U2Region    region;
    bool        translation;
    U2Strand    strand;
    int         err;

    static QList<SharedAnnotationData> toTable(const QList<FindAlgorithmResult>& res, const QString& name)
    {
        QList<SharedAnnotationData> list;
        foreach (const FindAlgorithmResult& f, res) {
            list.append(f.toAnnotation(name));
        }
        return list;
    }

};

class DNATranslation;

class U2ALGORITHM_EXPORT FindAlgorithmResultsListener {
public:
    virtual ~FindAlgorithmResultsListener(){}
    virtual void onResult(const FindAlgorithmResult& r) = 0;
};

enum FindAlgorithmStrand {
    FindAlgorithmStrand_Both,
    FindAlgorithmStrand_Direct,
    FindAlgorithmStrand_Complement
};

class U2ALGORITHM_EXPORT FindAlgorithmSettings {
public:
    FindAlgorithmSettings(const QByteArray& pattern = QByteArray(),
        FindAlgorithmStrand strand = FindAlgorithmStrand_Direct,
        DNATranslation* complementTT = NULL,
        DNATranslation* proteinTT = NULL,
        const U2Region& searchRegion = U2Region(),
        bool singleShot = false,
        int maxErr = 0,
        bool insDelAlg = false
) : pattern(pattern), strand(strand), complementTT(complementTT), proteinTT(proteinTT),
searchRegion(searchRegion), singleShot(singleShot), maxErr(maxErr), insDelAlg(insDelAlg){}

    QByteArray          pattern;
    FindAlgorithmStrand strand;
    DNATranslation*     complementTT;
    DNATranslation*     proteinTT;
    U2Region             searchRegion;
    bool                singleShot;
    int                 maxErr;
    bool                insDelAlg;
};


class U2ALGORITHM_EXPORT FindAlgorithm {
public:
    // Note: pattern is never affected by either aminoTT or complTT
    
    static void find(
        FindAlgorithmResultsListener* rl, 
        DNATranslation* aminoTT, // if aminoTT!=NULL -> pattern must contain amino data and sequence must contain DNA data
        DNATranslation* complTT, // if complTT!=NULL -> sequence is complemented before comparison with pattern
        FindAlgorithmStrand strand, // if not direct there complTT must not be NULL
        bool insDel,
        const char* sequence, 
        int seqLen, 
        const U2Region& range,  
        const char* pattern, 
        int patternLen, 
        bool singleShot, 
        int maxErr, 
        int& stopFlag, 
        int& percentsCompleted, 
        int& currentPos); 

    static void find(
        FindAlgorithmResultsListener* rl,
        const FindAlgorithmSettings& config,
        const char* sequence, 
        int seqLen, 
        int& stopFlag, 
        int& percentsCompleted, 
        int& currentPos) {
            find(rl,
                config.proteinTT,
                config.complementTT,
                config.strand,
                config.insDelAlg,
                sequence,
                seqLen,
                config.searchRegion,
                config.pattern.constData(),
                config.pattern.length(),
                config.singleShot,
                config.maxErr,
                stopFlag, 
                percentsCompleted, 
                currentPos);
    }

};

}//namespace

#endif
