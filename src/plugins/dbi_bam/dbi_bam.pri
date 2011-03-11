# include (dbi_bam.pri)

PLUGIN_ID=dbi_bam
PLUGIN_NAME=BAM file based DBI implementation
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

use_bundled_zlib() {
    INCLUDEPATH += ../../libs_3rdparty/zlib/src
}

LIBS += -lsqlite3

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        LIBS -= -lsqlite3
        LIBS += -lsqlite3d
    }
}

INCLUDEPATH += ../../libs_3rdparty/sqlite3/src
