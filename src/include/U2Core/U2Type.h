#ifndef _U2_TYPE_H_
#define _U2_TYPE_H_

#include <U2Core/global.h>

namespace U2 {


/**
    UGENE built-in data-types
    Note: unsigned used to reveal all possible math ops with it.

    U2DataType - data types supported by U2Dbi
    U2DataId - object ids associated with Data by U2Dbi. 
    Note that U2DataId == 0 check must be suppoted by DBI to check empty fields.
*/
typedef quint16 U2DataType;
typedef quint64 U2DataId;


/** 
    Built in types 
    Note: Maximum value used for type must be <=4096
*/

class U2CORE_EXPORT U2Type {
public:
    /** Type is unknown. Default value. */
    static const U2DataType Unknown     = 0;

    /** Object types */
    static const U2DataType Sequence    = 1;
    static const U2DataType Msa         = 2;
    static const U2DataType PhyTree     = 3;
    static const U2DataType Assembly    = 4;
    static const U2DataType CrossDatabaseReference    = 999;

    /** SCO (non-object, non-root) types */
    static const U2DataType Annotation              = 1000;
    
    /**  Assembly read */
    static const U2DataType AssemblyRead            = 1100;

    /**  MSA */
    static const U2DataType MsaRow                  = 1200;

    /**  Attribute types. Note: we support only limited types of primitive attributes */
    static const U2DataType AttributeInt32            = 2002;
    static const U2DataType AttributeInt64            = 2003;
    static const U2DataType AttributeReal64           = 2011;
    static const U2DataType AttributeString           = 2020;
    static const U2DataType AttributeByteArray        = 2021;
    static const U2DataType AttributeDateTime         = 2021;
    static const U2DataType AttributeRangeInt32Stat   = 2032;
    static const U2DataType AttributeRangeReal64Stat  = 2041;

    static bool isObjectType(U2DataType type) {return type > 0 && type < 999;}

    static bool isAttributeType(U2DataType type) {return type >=2000 && type < 2100;}

};


/** 
    Cross database data reference
*/
class U2CORE_EXPORT U2DataRef {
public:
    U2DataRef() {}
    U2DataRef(const QString& _dbiId, const U2DataId& _entityId) : dbiId(_dbiId),entityId(_entityId){}

    /** database  id */
    QString         dbiId;

    /** DB local data reference */
    U2DataId        entityId;

    /** Object version number this reference is valid for */
    qint64          version;

};

/** 
    Base class for all data types that can be referenced by some ID
*/
class U2CORE_EXPORT U2Entity {
public:
    U2Entity(){}
    U2Entity(U2DataId _id) : id(_id){}
    virtual ~U2Entity(){}

    U2DataId id;
};


/** 
    Base marker class for all First-class-objects stored in the database
*/
class U2CORE_EXPORT U2Object : public U2Entity {
public:
    U2Object() : version(0){}
    U2Object(U2DataId id, const QString& _dbId, qint64 v) : U2Entity(id), dbiId(_dbId), version(v) {}
    
    /** Source of the object: database id */
    QString     dbiId;

    /** Version of the object. Same as modification count of the object */
    qint64      version;
};


/** 
    If database keeps annotations/attributes for data entity stored in another database
    U2CrossDatabaseReference used as a parent object for all local data
*/
class U2CORE_EXPORT U2CrossDatabaseReference : public U2Object {
public:

    // remote data element id;
    U2DataRef   dataRef; 


    /** 
        Version(mod-count) of the content stored for remote object in this database 
        For example: for a sequence object this value can be increased with any new annotation modification
    */
    qint64          localContentVersion;

};


} //namespace

#endif
