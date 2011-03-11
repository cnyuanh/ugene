#ifndef _U2_DNA_SEQUENCE_OBJECT_H_
#define _U2_DNA_SEQUENCE_OBJECT_H_

#include <U2Core/GObject.h>
#include <U2Core/U2Region.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequence.h>

namespace U2 {

class  U2CORE_EXPORT DNASequenceObject: public GObject {
    Q_OBJECT
public:
    DNASequenceObject(const QString& name, const DNASequence& seq, const QVariantMap& hintsMap = QVariantMap());

    const U2Region& getSequenceRange() const {return seqRange;}

    const QByteArray& getSequence() const {return dnaSeq.seq;}

    DNAAlphabet* getAlphabet() const {return dnaSeq.alphabet;}
    
    const DNAQuality& getQuality() const { return dnaSeq.quality; }

    const DNASequence& getDNASequence() const {return dnaSeq;}

    int getSequenceLen() const {return dnaSeq.length();}

    const QString getSequenceName() const {return dnaSeq.getName(); }

    virtual GObject* clone() const;

    void setBase(int pos, char base);

    virtual bool checkConstraints(const GObjectConstraints* c) const;

    void setSequence(DNASequence seq);

    void setQuality(const DNAQuality& quality);

    bool isCircular() const {return dnaSeq.circular;}

    void setCircular(bool val);

signals:
    void si_sequenceChanged();

protected:
    DNASequence     dnaSeq;
    U2Region         seqRange;
};

class U2CORE_EXPORT DNASequenceObjectConstraints : public GObjectConstraints   {
    Q_OBJECT
public:
    DNASequenceObjectConstraints(QObject* p = NULL);
    int exactSequenceSize;
    DNAAlphabetType alphabetType;
};


}//namespace


#endif
