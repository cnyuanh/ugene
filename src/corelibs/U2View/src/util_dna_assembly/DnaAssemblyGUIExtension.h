#ifndef _U2_DNA_ASSEMBLEY_GUI_EXTENSION_H_
#define _U2_DNA_ASSEMBLEY_GUI_EXTENSION_H_


#include <U2Core/global.h>
#include <U2Core/GUrl.h>
#include <U2Core/Task.h>

#include <QtGui/QWidget>

namespace U2 {


// These classes are intended for extending standard Dna Assembly dialog GUI
// with options specific to the assembly algorithm

class DnaAssemblyAlgorithmMainWidget : public QWidget {
public:
    DnaAssemblyAlgorithmMainWidget(QWidget* parent) : QWidget(parent) {}
    virtual QMap<QString,QVariant> getDnaAssemblyCustomSettings() = 0;
    virtual bool isIndexOk() = 0;
    virtual void buildIndexUrl(const GUrl& url) = 0;
};

class DnaAssemblyAlgorithmBuildIndexWidget : public QWidget {
public:
    DnaAssemblyAlgorithmBuildIndexWidget(QWidget* parent) : QWidget(parent) {}
    virtual QMap<QString,QVariant> getBuildIndexCustomSettings() = 0;
    virtual QString getIndexFileExtension() = 0;
};

class DnaAssemblyGUIExtensionsFactory {
public:
    virtual DnaAssemblyAlgorithmMainWidget* createMainWidget(QWidget* parent) = 0;
    virtual DnaAssemblyAlgorithmBuildIndexWidget* createBuildIndexWidget(QWidget* parent) = 0;
    virtual bool hasMainWidget() = 0;
    virtual bool hasBuildIndexWidget() = 0;
};

} // U2


#endif // _U2_DNA_ASSEMBLEY_GUI_EXTENSION_H_
