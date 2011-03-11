#ifndef _U2_ENZYMES_TESTS_H_
#define _U2_ENZYMES_TESTS_H_

#include <U2Core/GObject.h>
#include <U2Core/U2Region.h>
#include <U2Test/XMLTestUtils.h>

#include <QtCore/QMultiMap>
#include <QtXml/QDomElement>

#include "DNAFragment.h"

namespace U2 {

class LoadEnzymeFileTask;
class AnnotationTableObject;
class DNASequenceObject;

//cppcheck-suppress noConstructor
class GTest_FindEnzymes : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_FindEnzymes, "find-enzymes");

    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);
    ReportResult report();
    void cleanup();
    
private:
    int minHits;
    int maxHits;
    QString                 enzymesUrl;
    QString                 seqObjCtx;
    QString                 aObjName;
    DNASequenceObject*      seqObj;
    QVector<U2Region>         excludedRegions;
    QStringList             enzymeNames;
    AnnotationTableObject*  aObj;
    LoadEnzymeFileTask*     loadTask;
    bool                    contextIsAdded;
    QMultiMap<QString, U2Region> resultsPerEnzyme;
};

//cppcheck-suppress noConstructor
class GTest_DigestIntoFragments : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_DigestIntoFragments, "digest-into-fragments");

    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);
     
private:
    QString                 seqObjCtx;
    QString                 aObjCtx;
    QString                 enzymesUrl;
    QStringList             enzymeNames;
    bool                    searchForEnzymes;
    AnnotationTableObject*  aObj;
    DNASequenceObject*      seqObj;
    LoadEnzymeFileTask*     loadTask;
};


class LigateFragmentsTask;

//cppcheck-suppress noConstructor
class GTest_LigateFragments : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_LigateFragments, "ligate-fragments");

    void cleanup();
    void prepare();
    ReportResult report();
    void prepareFragmentsList();

private:
    QStringList             seqObjNames;
    QStringList             annObjNames;
    QStringList             fragmentNames;
    QString                 resultDocName;
    QList<GObject*>         sObjs, aObjs;
    QList<DNAFragment>      targetFragments;
    bool                    makeCircular;
    bool                    checkOverhangs;
    bool                    contextAdded;
    LigateFragmentsTask*    ligateTask;
};


class EnzymeTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

} //namespace

#endif
