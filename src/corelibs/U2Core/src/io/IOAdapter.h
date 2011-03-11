#ifndef _U2_IOADAPTER_H_
#define _U2_IOADAPTER_H_

#include <U2Core/global.h>
#include <U2Core/GUrl.h>

#include <QtCore/QtCore>

namespace U2 {

enum IOAdapterMode {
    IOAdapterMode_Read,
    IOAdapterMode_Write,
    IOAdapterMode_Append
};

class IOAdapter;

class U2CORE_EXPORT IOAdapterFactory : public QObject {
public:
    IOAdapterFactory(QObject* p) : QObject(p){}

    virtual IOAdapter* createIOAdapter()  = 0;

    virtual IOAdapterId getAdapterId() const = 0;

    virtual const QString& getAdapterName() const = 0;

    virtual bool isIOModeSupported(IOAdapterMode m) const = 0;

    virtual TriState isResourceAvailable(const GUrl& url) const = 0;

};



class U2CORE_EXPORT IOAdapter : public QObject {
    Q_OBJECT
public:
    IOAdapter(IOAdapterFactory* f, QObject* o = NULL) : QObject(o), factory(f){}
    virtual ~IOAdapter(){}

    IOAdapterId getAdapterId() const {return factory->getAdapterId();}

    virtual const QString& getAdapterName() const  {return factory->getAdapterName();}

    virtual bool isIOModeSupported(IOAdapterMode m) const  {return factory->isIOModeSupported(m);}

    IOAdapterFactory* getFactory() const {return factory;}

    virtual bool open(const GUrl& url, IOAdapterMode m) = 0;

    virtual bool isOpen() const = 0;

    virtual void close() = 0;

    enum TerminatorHandling {
        Term_Exclude,   // stop before terminators
        Term_Include,   // include all terminators into result
        Term_Skip       // do not include terminators to the result, but skip to after last terminator
    };

    //return 0 if at the end of file, -1 if error
    virtual qint64 readUntil(char* buff, qint64 maxSize, const QBitArray& readTerminators, 
        TerminatorHandling th, bool* terminatorFound = 0);

    virtual bool getChar(char* buff) {return 1 == readBlock(buff, 1);}

    //If an error occurs, this function returns -1
    virtual qint64 readBlock(char* buff, qint64 maxSize) = 0;

    //read a single line of text and skips one EOL, returns length of line w/o terminator or -1
    virtual qint64 readLine(char* buff, qint64 maxSize, bool* terminatorFound = 0);

    virtual qint64 writeBlock(const char* buff, qint64 size) = 0;

    qint64 writeBlock(const QByteArray& a) {return writeBlock(a.data(), a.size());}

    /**
     * Both positive and negative values are accepted.
     * Implementations should support skipping backwards within 32K of total read data.
     */
    virtual bool skip(qint64 nBytes) = 0;

    //returns -1 if not supported
    virtual qint64 left() const = 0;

    /* Percent values in range 0..100, negative if unknown. */
    virtual int getProgress() const = 0;
    
    bool isEof();
    
    virtual qint64 bytesRead() const { return -1; }
    
    virtual GUrl getURL() const = 0;

    virtual QString toString() const {return getURL().getURLString();}
    
private:
    IOAdapterFactory* factory;
};

class U2CORE_EXPORT IOAdapterRegistry  : public QObject {
public:
    IOAdapterRegistry(QObject* p = NULL) : QObject(p) {}

    virtual bool registerIOAdapter(IOAdapterFactory* io) = 0;

    virtual bool unregisterIOAdapter(IOAdapterFactory* io) = 0;

    virtual const QList<IOAdapterFactory*>& getRegisteredIOAdapters() const = 0;

    virtual IOAdapterFactory* getIOAdapterFactoryById(IOAdapterId id) const = 0;
};

class U2CORE_EXPORT BaseIOAdapters {
public:
    static const IOAdapterId LOCAL_FILE;
    static const IOAdapterId GZIPPED_LOCAL_FILE;
    static const IOAdapterId HTTP_FILE;
    static const IOAdapterId GZIPPED_HTTP_FILE;
    static const IOAdapterId VFS_FILE;

    static IOAdapterId url2io(const GUrl& url);

    static QByteArray readFileHeader(const GUrl& url, int size = 1024);
    
    // io - opened ioadapter. before and after the call pos in file the same
    static QByteArray readFileHeader( IOAdapter* io, int size = 1024 );
};


}//namespace

#endif

