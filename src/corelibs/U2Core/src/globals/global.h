#ifndef _U2_COREAPI_H_
#define _U2_COREAPI_H_

#include <assert.h>
#include <QtCore/qglobal.h>
#include <QtCore/QVariantMap>
#include <QtCore/QObject>

#ifdef BUILDING_U2CORE_DLL
#   define U2CORE_EXPORT Q_DECL_EXPORT
#else
#   define U2CORE_EXPORT Q_DECL_IMPORT
#endif
#ifdef BUILDING_U2FORMATS_DLL
#   define U2FORMATS_EXPORT Q_DECL_EXPORT
#else
#   define U2FORMATS_EXPORT Q_DECL_IMPORT
#endif
#ifdef BUILDING_U2ALGORITHM_DLL
#   define U2ALGORITHM_EXPORT Q_DECL_EXPORT
#else
#   define U2ALGORITHM_EXPORT Q_DECL_IMPORT
#endif
#ifdef BUILDING_U2TEST_DLL
#   define U2TEST_EXPORT Q_DECL_EXPORT
#else
#   define U2TEST_EXPORT Q_DECL_IMPORT
#endif
#ifdef BUILDING_U2LANG_DLL
#   define U2LANG_EXPORT Q_DECL_EXPORT
#else
#   define U2LANG_EXPORT Q_DECL_IMPORT
#endif
#ifdef BUILDING_U2REMOTE_DLL
#   define U2REMOTE_EXPORT Q_DECL_EXPORT
#else
#   define U2REMOTE_EXPORT Q_DECL_IMPORT
#endif
#ifdef BUILDING_U2GUI_DLL
#   define U2GUI_EXPORT Q_DECL_EXPORT
#else
#   define U2GUI_EXPORT Q_DECL_IMPORT
#endif
#ifdef BUILDING_U2VIEW_DLL
#   define U2VIEW_EXPORT Q_DECL_EXPORT
#else
#   define U2VIEW_EXPORT Q_DECL_IMPORT
#endif
#ifdef BUILDING_U2MISC_DLL
#   define U2MISC_EXPORT Q_DECL_EXPORT
#else
#   define U2MISC_EXPORT Q_DECL_IMPORT
#endif
#ifdef BUILDING_U2DESIGNER_DLL
#   define U2DESIGNER_EXPORT Q_DECL_EXPORT
#else
#   define U2DESIGNER_EXPORT Q_DECL_IMPORT
#endif

//global
#define GLOBAL_SETTINGS QString("global/")

#define ENV_UGENE_DEV "UGENE_DEV"

#define PATH_PREFIX_DATA "data"
#define PATH_PREFIX_SCRIPTS "scripts"

typedef QMap<QString, QString> QStrStrMap;

namespace U2 {

//internal types:
typedef QString                 DocumentFormatId;
typedef QString                 GObjectType;
typedef QString                 IOAdapterId;
typedef QString                 GObjectViewFactoryId;
typedef QString                 GTestFormatId;
typedef int                     ServiceType;

enum TriState {
    TriState_Unknown,
    TriState_Yes,
    TriState_No
};

enum UnloadedObjectFilter { //used as a separate type but not 'bool' to improve readability
    UOF_LoadedAndUnloaded,
    UOF_LoadedOnly
};

}

enum DNAAlphabetType {
    DNAAlphabet_RAW,
    DNAAlphabet_NUCL,
    DNAAlphabet_AMINO
};



#endif
