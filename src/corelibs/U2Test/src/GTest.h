#ifndef _U2_UGENE_TEST_FRAMEWORK_H_
#define _U2_UGENE_TEST_FRAMEWORK_H_

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtXml/QtXml>

#include <U2Core/Task.h>

namespace U2 {


class U2TEST_EXPORT GTestEnvironment {
public:
    void setVar(const QString& varName, const QString& val) {vars[varName] = val;}
    
    QString getVar(const QString& varName) const {return vars.value(varName);}
    
    QMap<QString, QString> getVars() const {return vars;}

    bool containsEmptyVars() const {return vars.values().contains(QString(""));}

private:
    QMap<QString, QString> vars;
};


class U2TEST_EXPORT GTest : public Task {
    Q_OBJECT
public:
    GTest(const QString& taskName, GTest* cp, const GTestEnvironment* env, 
        TaskFlags flags, const QList<GTest*>& subtasks = QList<GTest*>());

    QObject* getContext(const QString& name) const;
    
    void addContext(const QString& name, QObject* v);

    void removeContext(const QString& name);

    template <class T>
    static T* getContext(const GTest* test, const QString& name) { 
        QObject* o = test->getContext(name);
        T* res = qobject_cast<T*>(o);
        return res;
    }

const GTestEnvironment* getEnv() const {return env;}

protected:
    GTest* getContextProvider() const {return contextProvider;}

    void failMissingValue(const QString&);
    
    GTest* contextProvider;
    const GTestEnvironment* env;
    QMap<QString, QObject*> subtestsContext;
};


class U2TEST_EXPORT GTestFormat : public QObject {
public:
    GTestFormat(const QString& _formatId) : formatId(_formatId){};

    const QString getFormatId() const {return formatId;}

    virtual GTest* createTest(const QString& name, GTest* cp, const GTestEnvironment* env, const QByteArray& testData, QString& err) = 0;

protected:
    QString formatId;
};

class GTestSuite;
class U2TEST_EXPORT GTestRef {
public:
    GTestRef(const QString& _url, const QString& sName, const GTestFormatId f) 
        : url(_url), shortName(sName), formatId(f){}

    const QString& getURL() const {return url;}

    const QString& getShortName() const {return shortName;}

    const GTestFormatId getFormatId() {return formatId;}

    void setSuite( GTestSuite * _suite ) { suite = _suite; }

    GTestSuite * getSuite() const {return suite;}

private:
    QString             url;
    QString             shortName;
    GTestFormatId       formatId;
    GTestSuite *        suite;
};

class U2TEST_EXPORT GTestState : public QObject {
    Q_OBJECT
public:
    GTestState(GTestRef* ref) : testRef(ref), state(TriState_Unknown) {}
    
    GTestRef* getTestRef() const {return testRef;}

    bool isPassed() const {return state == TriState_Yes;}

    bool isFailed() const {return state == TriState_No;}

    bool isNew() const {return state == TriState_Unknown;}

    const QString& getErrorMessage() const {return errMessage;}

    void clearState();

    void setFailed(const QString& err);

    void setPassed();

signals:
    void si_stateChanged(GTestState* ts);

private:
    GTestRef*           testRef;
    TriState            state;
    QString             errMessage;
};


class U2TEST_EXPORT GTestSuite  : public QObject {
    Q_OBJECT
public:
    virtual ~GTestSuite();

    GTestEnvironment* getEnv() {return &env;}

    const QList<GTestRef*>& getTests() const {return tests;}
    const QList<GTestRef*>& getExcludedTests() const {return excluded;}

    const QString& getURL() const {return url;}

    const QString& getName() const {return name;}

    static GTestSuite* readTestSuite(const QString& url, QString& err);
    static QList<GTestSuite*> readTestSuiteList(const QString& url, QStringList& err);

protected:
    GTestSuite() {}
    
    QString url;

    QString name;
    
    QList<GTestRef*> tests;
    QList<GTestRef*> excluded;

    GTestEnvironment env;
};


}//namespace

#endif
