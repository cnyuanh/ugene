#ifndef __ASSEMBLY_BROWSER_H__
#define __ASSEMBLY_BROWSER_H__

#include <assert.h>

#include <QtCore/QByteArray>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2Assembly.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/MainWindow.h>

namespace U2 {

class AssemblyModel {
public:
    AssemblyModel();

    bool isEmpty() const;

    QList<U2AssemblyRead> getReadsFromAssembly(int assIdx, const U2Region & r, qint64 minRow, qint64 maxRow, U2OpStatus & os);

    qint64 countReadsInAssembly(int assIdx, const U2Region & r, U2OpStatus & os);

    qint64 getModelLength(U2OpStatus & os);

    qint64 getModelHeight(U2OpStatus & os);

    void addAssembly(U2AssemblyRDbi * dbi, const U2Assembly & assm);

    bool hasReference() const;

    void setReference(U2SequenceRDbi * dbi, const U2Sequence & seq);

    QByteArray getReferenceRegion(const U2Region& region, U2OpStatus& os);

private:
    const static qint64 NO_VAL = -1;
    //TODO: track model changes and invalidate caches accordingly
    qint64 cachedModelLength;
    qint64 cachedModelHeight;

    U2Sequence reference;
    U2SequenceRDbi * referenceDbi;

    QList<U2Assembly> assemblies;
    QList<U2AssemblyRDbi *> assemblyDbis;
};

class AssemblyBrowserUi;

class AssemblyBrowserWindow : public MWMDIWindow {
    Q_OBJECT
public:
    AssemblyBrowserWindow();
    ~AssemblyBrowserWindow();

    virtual void setupMDIToolbar(QToolBar* tb);
    virtual void setupViewMenu(QMenu* n);

    int getCellWidth() const;
    qint64 calcPixelCoord(qint64 asmCoord) const;
    qint64 calcAsmCoord(qint64 pixCoord) const;

    qint64 basesCanBeVisible() const;
    qint64 rowsCanBeVisible() const;

    qint64 basesVisible() const;
    qint64 rowsVisible() const;

    bool areReadsVisible() const;
    bool areCellsVisible() const;
    bool areLettersVisible() const;

    inline AssemblyModel * getModel() const {return model;}
    inline double getZoomFactor() const {return zoomFactor;}
    inline QFont getFont() const {return font;}

    inline qint64 getXOffsetInAssembly() const {return xOffsetInAssembly; }
    inline qint64 getYOffsetInAssembly() const {return yOffsetInAssembly; }

    void setXOffsetInAssembly(qint64 x); 
    void setYOffsetInAssembly(qint64 y);

    void adjustOffsets(qint64 dx, qint64 dy);

signals:
    void si_zoomOperationPerformed();
    void si_offsetsChanged();

private slots:
    void sl_loadAssembly();
    void sl_assemblyLoaded();

public slots:
    void sl_zoomIn();
    void sl_zoomOut();

private:
    void createWidgets();
    void initFont();
    void setupActions();
    void updateActions();
    void clear();

    AssemblyBrowserUi * ui;

    U2Dbi * dbi; //TODO: remove
    AssemblyModel * model;

    double zoomFactor;
    QFont font;

    qint64 xOffsetInAssembly;
    qint64 yOffsetInAssembly;

    QAction * openAssemblyAction;
    QAction * zoomInAction;
    QAction * zoomOutAction;

    const static double ZOOM_MULT;
    const static int MAX_CELL_WIDTH = 300;
    const static int LETTER_VISIBLE_WIDTH = 7;
    const static int CELL_VISIBLE_WIDTH = 1;
}; 


class AssemblyOverview;
class AssemblyReferenceArea;
class AssemblyDensityGraph;
class AssemblyRuler;
class AssemblyReadsArea;

class AssemblyBrowserUi : public QWidget {
    Q_OBJECT
public:
    AssemblyBrowserUi(AssemblyBrowserWindow * window);

    inline AssemblyModel * getModel() const {return window->getModel();}
    inline AssemblyBrowserWindow * getWindow() const {return window;}

    inline AssemblyReadsArea * getReadsArea() const {return readsArea;}

private:
    AssemblyOverview * overview;        
    AssemblyReferenceArea * referenceArea;
    AssemblyDensityGraph * densityGraph;
    AssemblyRuler * ruler;
    AssemblyReadsArea * readsArea;
    
    AssemblyBrowserWindow * window;
};

/**
 * Dumps error to log and returns true if status contains an error.
 */
bool checkAndLogError(const U2OpStatusImpl & status);

/**
 * Returns read length calculated with respect to CIGAR.
 */
qint64 countReadLength(qint64 realLen, const QList<U2CigarToken> & cigar);

/**
 * Returns read sequence. If read has no embedded sequence -> gets it from dbi.
 */
QByteArray getReadSequence(U2Dbi * dbi, const U2AssemblyRead & read, U2OpStatus & os);

} //ns

#endif 
