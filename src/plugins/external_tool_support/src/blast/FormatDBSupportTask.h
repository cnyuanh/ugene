#ifndef _U2_FORMATDB_SUPPORT_TASK_H
#define _U2_FORMATDB_SUPPORT_TASK_H

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include "utils/ExportTasks.h"

#include <U2Core/MAlignmentObject.h>

#include "ExternalToolRunTask.h"

namespace U2 {

/*Options for FormatDB
-t  Title for database file [String]  Optional
  -i  Input file(s) for formatting [File In]  Optional
  -l  Logfile name: [File Out]  Optional
    default = formatdb.log
  -p  Type of file
         T - protein
         F - nucleotide [T/F]  Optional
    default = T
  -o  Parse options
         T - True: Parse SeqId and create indexes.
         F - False: Do not parse SeqId. Do not create indexes.
 [T/F]  Optional
    default = F
  -a  Input file is database in ASN.1 format (otherwise FASTA is expected)
         T - True,
         F - False.
 [T/F]  Optional
    default = F
  -b  ASN.1 database in binary mode
         T - binary,
         F - text mode.
 [T/F]  Optional
    default = F
  -e  Input is a Seq-entry [T/F]  Optional
    default = F
  -n  Base name for BLAST files [String]  Optional
  -v  Database volume size in millions of letters [Integer]  Optional
    default = 4000
  -s  Create indexes limited only to accessions - sparse [T/F]  Optional
    default = F
  -V  Verbose: check for non-unique string ids in the database [T/F]  Optional
    default = F
  -L  Create an alias file with this name
        use the gifile arg (below) if set to calculate db size
        use the BLAST db specified with -i (above) [File Out]  Optional
  -F  Gifile (file containing list of gi's) [File In]  Optional
  -B  Binary Gifile produced from the Gifile specified above [File Out]  Optional
  -T  Taxid file to set the taxonomy ids in ASN.1 deflines [File In]  Optional
*/
//class FormatDBLogParser;
class FormatDBSupportTaskSettings {
public:
    FormatDBSupportTaskSettings() {reset();}
    void reset();

    QStringList     inputFilesPath;
    QString         outputPath;
    QString         databaseTitle;
    /*-p  Type of file
            T - protein
            F - nucleotide*/
    bool            typeOfFile;
};


class FormatDBSupportTask : public Task {
    Q_OBJECT
public:
    FormatDBSupportTask(const QString& name, const FormatDBSupportTaskSettings& settings);
    void prepare();
    Task::ReportResult report();
private:
    ExternalToolLogParser*      logParser;

    ExternalToolRunTask*        formatDBTask;
    QString                     toolName;
    FormatDBSupportTaskSettings settings;
};

//class FormatDBLogParser : public ExternalToolLogParser {
//public:
//    FormatDBLogParser(int countSequencesInMSA);
//    int getProgress();
//private:
//    int countSequencesInMSA;
//};

}//namespace
#endif // _U2_FORMATDB_SUPPORT_TASK_H
