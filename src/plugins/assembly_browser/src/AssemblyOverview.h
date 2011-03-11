#ifndef __ASSEMBLY_OVERVIEW_H__
#define __ASSEMBLY_OVERVIEW_H__

#include <QtGui/QWidget>
#include <QtGui/QPixmap>

#include <U2Core/Task.h>

namespace U2 {

class AssemblyModel;

class AssemblyOverviewRenderTask: public Task {
    Q_OBJECT
public:
    AssemblyOverviewRenderTask(AssemblyModel * model, QSize imageSize);
    virtual void run();
    inline QImage getResult() const {return result;};
private:
    AssemblyModel * model;
    QImage result;
};

class BackgroundRenderer: public QObject {
    Q_OBJECT
public:
    BackgroundRenderer(AssemblyModel * model_);
    void render(const QSize & size_);
    QImage getImage() const;
signals:
    void si_rendered();
private slots:
    void sl_redrawFinished();
private:
    AssemblyOverviewRenderTask * renderTask;
    QImage result;
    AssemblyModel * model;
    QSize size;
    bool redrawRunning;
    bool redrawNeeded;
};

class AssemblyBrowserUi;
class AssemblyBrowserWindow;
class AssemblyOverviewRenderTask;

class AssemblyOverview: public QWidget {
    Q_OBJECT
public:
    AssemblyOverview(AssemblyBrowserUi * ui);

protected:
    void paintEvent(QPaintEvent * e);
    void resizeEvent(QResizeEvent * e);
    void mousePressEvent(QMouseEvent * me);
    void mouseMoveEvent(QMouseEvent * me);
    void mouseReleaseEvent(QMouseEvent * me);

private slots:
    void sl_visibleAreaChanged();
    void sl_redraw();

private:
    qint64 calcXAssemblyCoord(int x);
    qint64 calcYAssemblyCoord(int y);

    QRect calcCurrentSelection() const;
    void moveSelectionToPos(QPoint pos, bool moveModel = true);

    void connectSlots();
    void initSelectionRedraw();

    void drawAll();
    void drawSelection(QPainter & p);
    void drawCoordLabels(QPainter & p);

private:
    AssemblyBrowserUi * ui;
    AssemblyBrowserWindow * window;
    AssemblyModel * model;

    QRect cachedSelection;

    QPixmap cachedView;
    bool redrawSelection;

    QPixmap cachedBackground;
    BackgroundRenderer bgrRenderer;

    bool scribbling;

    const static int FIXED_HEIGHT = 100;
};

} //ns

#endif 
