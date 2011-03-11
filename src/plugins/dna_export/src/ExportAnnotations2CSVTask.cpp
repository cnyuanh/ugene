#include "ExportAnnotations2CSVTask.h"

#include <U2Core/IOAdapter.h>
#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/L10n.h>
#include <U2Core/TextUtils.h>
#include <U2Core/AnnotationTableObject.h>

#include <memory>

namespace U2 {

ExportAnnotations2CSVTask::ExportAnnotations2CSVTask(const QList<Annotation*>& annotations, 
                                                     const QByteArray& sequence,
                                                     DNATranslation *complementTranslation,
                                                     bool exportSequence,
                                                     const QString& url, 
                                                     bool apnd)
: Task(tr("Export2CSV"), TaskFlag_None),
    annotations(annotations),
    sequence(sequence),
    complementTranslation(complementTranslation),
    exportSequence(exportSequence),
    url(url),
    append(apnd)
{
    GCOUNTER( cvar, tvar, "ExportAnnotattions2CSVTask" );
}

static void writeCSVLine(const QStringList& container, IOAdapter *ioAdapter) {
    bool first = true;
    foreach(QString value, container) {
        if (!first) {
            if (0 == ioAdapter->writeBlock(",")) {
                throw 0;
            }
        }
        if (0 == ioAdapter->writeBlock(("\"" + value.replace("\"","\"\"") + "\"").toLocal8Bit())) {
            throw 0;
        }
        first = false;
    }
    if (0 == ioAdapter->writeBlock("\n")) {
        throw 0;
    }
}

void ExportAnnotations2CSVTask::run() {
    try  {
        std::auto_ptr<IOAdapter> ioAdapter;
        {
            IOAdapterId ioAdapterId = BaseIOAdapters::url2io(url);
            IOAdapterFactory *ioAdapterFactory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(ioAdapterId);
            if(NULL == ioAdapterFactory) {
                stateInfo.setError(tr("No IO adapter found for URL: %1").arg(url));
                return;
            }
            ioAdapter.reset(ioAdapterFactory->createIOAdapter());
        }
        if(!ioAdapter->open(url, append ? IOAdapterMode_Append : IOAdapterMode_Write)) {
            stateInfo.setError(L10N::errorOpeningFileWrite(url));
            return;
        }
        QHash<QString, int> columnIndices;
        QStringList columnNames;
        {
            columnNames.append(tr("Group"));
            columnNames.append(tr("Name"));
            columnNames.append(tr("Start"));
            columnNames.append(tr("End"));
            columnNames.append(tr("Length"));
            columnNames.append(tr("Complement"));
            if (exportSequence) {
                columnNames.append(tr("Sequence"));
            }
            foreach(const Annotation* annotation, annotations) {
                foreach(const U2Qualifier& qualifier, annotation->getQualifiers()) {
                    const QString& qName = qualifier.name;
                    if(!columnIndices.contains(qName)) {
                        columnIndices.insert(qName, columnNames.size());
                        columnNames.append(qName);
                    }
                }
            }
            writeCSVLine(columnNames, ioAdapter.get());
        }
        foreach(const Annotation* annotation, annotations) {
            foreach(const U2Region& region, annotation->getRegions()) {
                QStringList values;
                values.append(annotation->getGroups().last()->getGroupPath());
                values.append(annotation->getAnnotationName());
                if (!annotation->getStrand().isDirect()) {
                    values.append(QString::number(region.startPos + 1));
                    values.append(QString::number(region.startPos + region.length));
                } else {
                    values.append(QString::number(region.startPos + region.length));
                    values.append(QString::number(region.startPos + 1));
                }
                values.append(QString::number(region.length));
                if (annotation->getStrand().isCompementary()) {
                    values.append(tr("yes"));
                } else {
                    values.append(tr("no"));
                }
                if (exportSequence) {
                    QByteArray sequencePart = sequence.mid(region.startPos, region.length);
                    if (annotation->getStrand().isCompementary()) {
                        complementTranslation->translate(sequencePart.data(), sequencePart.size());
                        TextUtils::reverse(sequencePart.data(), sequencePart.size());
                    }
                    values.append(sequencePart);
                }
                //add empty strings as default qualifier values
                while (values.size() < columnNames.size()) {
                    values.append(QString());
                }

                foreach(const U2Qualifier& qualifier, annotation->getQualifiers()) {
                    int qualifiedIndex = columnIndices[qualifier.name];
                    values[qualifiedIndex] = qualifier.value;
                }
                writeCSVLine(values, ioAdapter.get());
            }
        }
    } catch(int) {
        stateInfo.setError(L10N::errorWritingFile(url));
        return;
    }
}

} // namespace U2
