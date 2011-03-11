#ifndef _U2_DOT_PLOT_WIDGET_H_
#define _U2_DOT_PLOT_WIDGET_H_

#include <U2View/ADVSplitWidget.h>
#include <U2View/PanView.h>
#include <U2Core/U2Region.h>

#include <QtGui/QMenu>
#include <QtGui/QToolButton>

#include <QtCore/QTimer>

namespace U2 {

class Task;
class ADVSequenceObjectContext;
class ADVSequenceWidget;
class GObjectView;
class LRegionsSelection;

class DotPlotResultsListener;
class DotPlotMiniMap;
struct DotPlotResults;
class GSequenceLineView;


class DotPlotWidget : public ADVSplitWidget {
    Q_OBJECT

public:
    DotPlotWidget(AnnotatedDNAView* dnaView);
    ~DotPlotWidget();

    virtual bool acceptsGObject(GObject*) {return false;}
    virtual void updateState(const QVariantMap&) {}
    virtual void saveState(QVariantMap&) {}

    void buildPopupMenu(QMenu *) const;

    AnnotatedDNAView* getDnaView() const {return dnaView;}

    void setShiftZoom(ADVSequenceObjectContext*, ADVSequenceObjectContext*, float, float, const QPointF&);
    bool hasSelection();

    void setIgnorePanView(bool);
    void setKeepAspectRatio(bool);
    void zoomIn();
    void zoomOut();
    void zoomReset();

    bool canZoomOut();
    bool canZoomIn();

    void setSelActive(bool state);

    QString getXSequenceName();
    QString getYSequenceName();

signals:
    void si_removeDotPlot();
    void si_dotPlotChanged(ADVSequenceObjectContext*, ADVSequenceObjectContext*, float, float, QPointF);
    void si_dotPlotSelecting();

public slots:
    bool sl_showSettingsDialog();

private slots:
    void sl_taskFinished(Task*);
    void sl_showSaveImageDialog();
    bool sl_showSaveFileDialog();
    bool sl_showLoadFileDialog();
    void sl_showDeleteDialog();

    void sl_onSequenceSelectionChanged(LRegionsSelection*, const QVector<U2Region>& , const QVector<U2Region>&);

    void sl_sequenceWidgetRemoved(ADVSequenceWidget*);
    void sl_panViewChanged();
    void sl_timer();

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

    void wheelEvent(QWheelEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void focusInEvent(QFocusEvent* fe);
    void focusOutEvent(QFocusEvent* fe);
    bool event(QEvent *event);
private:

    AnnotatedDNAView* dnaView;

    QCursor cursor;

    bool selecting, shifting, miniMapLooking, selActive, nearestSelecting;
    LRegionsSelection *selectionX, *selectionY;
    ADVSequenceObjectContext *sequenceX, *sequenceY;
    bool direct, inverted, nearestInverted, ignorePanView, keepAspectRatio;

    QPointF zoom;
    float shiftX, shiftY;
    int minLen, identity;

    bool pixMapUpdateNeeded, deleteDotPlotFlag;

    Task *dotPlotTask;
    QPixmap *pixMap;
    DotPlotMiniMap *miniMap;

    const DotPlotResults *nearestRepeat;

    QTimer *timer;
    QToolButton *exitButton;

    QPointF clickedFirst, clickedSecond;

    DotPlotResultsListener *dotPlotDirectResultsListener, *dotPlotInverseResultsListener;

    QAction *showSettingsDialogAction;
    QAction *saveImageAction;
    QAction *saveDotPlotAction;
    QAction *loadDotPlotAction;
    QAction *deleteDotPlotAction;

    int textSpace;
    static const int defaultTextSpace = 30;
    static const int rulerNotchSize = 2;

    int w;
    int h;

    QColor dotPlotBGColor;
    QColor dotPlotDirectColor;
    QColor dotPlotInvertedColor;
    QColor dotPlotNearestRepeatColor;

    QByteArray sharedSeqX, sharedSeqY;
    PanView::ZoomUseObject zoomUseX, zoomUseY;

    void pixMapUpdate();

    void initActionsAndSignals();
    void connectSequenceSelectionSignals();

    void drawAll(QPainter&);
    void drawNames(QPainter&) const;
    void drawAxises(QPainter&) const;
    void drawDots(QPainter&);
    void drawSelection(QPainter&) const;
    void drawRulers(QPainter&) const;
    void drawMiniMap(QPainter&) const;
    void drawNearestRepeat(QPainter&) const;
    void drawFocus(QPainter& p) const;

    void sequencesMouseSelection(const QPointF &, const QPointF &);
    void sequencesCoordsSelection(const QPointF &, const QPointF &);
    void sequenceClearSelection();
    void clearRepeatSelection();

    void selectNearestRepeat(const QPointF &);
    const DotPlotResults* findNearestRepeat(const QPoint &); // sets nearestInverted

    void calcZooming(const QPointF &oldzoom, const QPointF &newzoom, const QPoint &p, bool emitSignal = true);
    void multZooming(float multzoom);

    void resetZooming();
    void checkShift(bool emitSignal = true);

    void updateCursor();

    QString makeToolTipText() const;

    QPointF zoomTo(Qt::Axis axis, const U2Region &lr, bool emitSignal = true);
    U2Region getVisibleRange(Qt::Axis axis);
    int getLrDifference(const U2Region &a, const U2Region &b);

    void miniMapShift();

    void drawRectCorrect(QPainter &p, QRectF rect) const;

    QPoint toInnerCoords(int x, int y) const;
    QPoint toInnerCoords(const QPoint &p) const;

    QPointF unshiftedUnzoomed(const QPointF &p) const;
    QPoint sequenceCoords(const QPointF &c) const;

    QString getRoundedText(QPainter& p, int num, int size) const;
    bool getLineToDraw(const DotPlotResults &r, QLine *line, float ratioX, float ratioY, bool invert = false) const;

    void addCloseDotPlotTask();
    void cancelRepeatFinderTask();
};

} // namespace

#endif // _U2_DOT_PLOT_WIDGET_H_
