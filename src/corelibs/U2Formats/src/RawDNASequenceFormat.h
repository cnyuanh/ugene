#ifndef _U2_RAW_DNA_SEQUENCE_FORMAT_H_
#define _U2_RAW_DNA_SEQUENCE_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class U2FORMATS_EXPORT RawDNASequenceFormat : public DocumentFormat {
    Q_OBJECT
public:
    RawDNASequenceFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::RAW_DNA_SEQUENCE;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    virtual void storeDocument(Document* d, TaskStateInfo& ts, IOAdapter* io);

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

private:
    QString formatName;
};

}//namespace

#endif
