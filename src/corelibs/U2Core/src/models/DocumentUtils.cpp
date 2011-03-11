#include "DocumentUtils.h"

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/GUrlUtils.h>

namespace U2 {

QSet<QString> DocumentUtils::getURLs(const QList<Document*>& docs) {
    QSet<QString> result;
    foreach(Document* d, docs) {
        result.insert(d->getURLString());
    }
    return result;
}

QSet<QString> DocumentUtils::getNewDocFileNameExcludesHint() {
    QSet<QString> excludeFileNames;
    Project* p = AppContext::getProject();
    if (p!=NULL) {
        excludeFileNames = DocumentUtils::getURLs(p->getDocuments());
    }
    return excludeFileNames;
    
}


static void placeOrderedByScore(QList<DocumentFormat*>& formats, QList<int>& scores, DocumentFormat* df, int score) {
    assert(formats.size() == scores.size());
    for (int i = 0; i < formats.length(); i++) {
        int scoreI = scores.at(i);
        if (scoreI < score) {
            scores.insert(i, score);
            formats.insert(i, df);
            return;
        }
    }
    formats.append(df);
    scores.append(score);
}

#define FORMAT_DETECTION_EXT_BONUS 3

// returns formats with FormatDetectionResult != Not matched sorted by FormatDetectionResult
// FormatDetectionResult is adjusted by +FORMAT_DETECTION_EXT_BONUS if extension is matched

QList< DocumentFormat* > DocumentUtils::detectFormat( const QByteArray& rawData, const QString& ext, const GUrl& url) {
    DocumentFormatRegistry* fr = AppContext::getDocumentFormatRegistry();
    QList< DocumentFormatId > allFormats = fr->getRegisteredFormats();

    QList<DocumentFormat*> formats;
    QList<int> scores;
    foreach( DocumentFormatId id, allFormats) {
        DocumentFormat* f = fr->getFormatById( id );
        int score = f->checkRawData(rawData, url);
        if (score ==  FormatDetection_NotMatched) {
            continue;
        }
        if (f->getSupportedDocumentFileExtensions().contains(ext)) {
            score+=FORMAT_DETECTION_EXT_BONUS;
        }
        placeOrderedByScore(formats, scores, f, score);
    }
    return formats;
}

QList<DocumentFormat*> DocumentUtils::detectFormat(const GUrl& url) {
    QList< DocumentFormat* > res;
    if( url.isEmpty() ) {
        return res;
    }
    QByteArray rawData = BaseIOAdapters::readFileHeader(url);
    if (rawData.isEmpty()) {
        return res;
    }
    QString ext = GUrlUtils::getUncompressedExtension(url);
    res = detectFormat(rawData, ext, url);
    return res;
}


QList< DocumentFormat* > DocumentUtils::detectFormat( IOAdapter *io ) {
    QList< DocumentFormat* > res;
    if( NULL == io || !io->isOpen() ) {
        return res;
    }
    QByteArray rawData = BaseIOAdapters::readFileHeader( io );
    QString ext = GUrlUtils::getUncompressedExtension(io->getURL());
    res = detectFormat( rawData, ext , io->getURL());
    return res;
}

} //namespace
