#include "AssemblyOverview.h"

#include <math.h>

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QLabel>

#include <U2Core/Log.h>
#include <U2Core/AppContext.h>

#include "AssemblyBrowser.h"
#include "AssemblyReadsArea.h"

namespace U2 {

//==============================================================================
// AssemblyOverview
//==============================================================================

AssemblyOverview::AssemblyOverview(AssemblyBrowserUi * ui_): ui(ui_), window(ui->getWindow()), model(ui_->getModel()), 
redrawSelection(true), bgrRenderer(model), scribbling(false)
{
    setFixedHeight(FIXED_HEIGHT);
    connectSlots();
    initSelectionRedraw();
    bgrRenderer.render(size());
}

void AssemblyOverview::connectSlots() {
    connect(&bgrRenderer, SIGNAL(si_rendered()), SLOT(sl_redraw()));
    connect(window, SIGNAL(si_zoomOperationPerformed()), SLOT(sl_visibleAreaChanged()));
    connect(window, SIGNAL(si_offsetsChanged()), SLOT(sl_redraw()));
}

void AssemblyOverview::initSelectionRedraw() {
    redrawSelection = true;
    cachedView = QPixmap(size());
}

void AssemblyOverview::drawAll() {
    if(!model->isEmpty()) {
        QImage bgr = bgrRenderer.getImage();
        if(bgr.isNull()) {
            cachedBackground = QPixmap(size());
            QPainter p(&cachedBackground);
            p.fillRect(cachedBackground.rect(), Qt::gray);
            p.drawText(cachedBackground.rect(), Qt::AlignCenter, tr("Background is rendering..."));
        } else {
            cachedBackground = QPixmap(size());
            cachedBackground = QPixmap::fromImage(bgr);
        }
        if (redrawSelection) {
            cachedView = cachedBackground;
            QPainter p(&cachedView);
            drawSelection(p);
            drawCoordLabels(p);
            redrawSelection = false;
        }
        QPainter p(this);
        p.drawPixmap(0, 0, cachedView);
    }
}

void AssemblyOverview::drawSelection(QPainter & p) {
    cachedSelection = calcCurrentSelection();
    p.fillRect(cachedSelection, QColor(230, 230, 230, 180));
    p.drawRect(cachedSelection.adjusted(0, 0, -1, -1));
}

namespace {
    void insertSpaceSeparators(QString & str) {
        for(int i = str.length()-3; i > 0; i-=3) {
            str.insert(i, " ");
        }
    }
}

void AssemblyOverview::drawCoordLabels(QPainter & p) {
    const static int xoffset = 4;
    const static int yoffset = 3;

    U2OpStatusImpl status;
    int modelLength = model->getModelLength(status);

    QString modelLengthText = QString::number(modelLength);
    insertSpaceSeparators(modelLengthText);

    QFont font;
    font.setStyleHint(QFont::SansSerif, QFont::PreferAntialias);
    QFontMetrics fontMetrics(font);

    QString globalRegionText = tr("1 to %1 bp").arg(modelLengthText); //TODO: custom start for custom overview
    QRect grtRect = QRect(0, 0, fontMetrics.width(globalRegionText), fontMetrics.height());
    grtRect.translate(xoffset, rect().height() - yoffset - grtRect.height());
    if(rect().contains(grtRect)) {
        p.setPen(Qt::gray);
        p.drawText(grtRect, Qt::AlignCenter, globalRegionText);
    }

    qint64 from = window->getXOffsetInAssembly();
    qint64 to = qMin(window->getXOffsetInAssembly() + window->basesCanBeVisible(), model->getModelLength(status));

    QString fromText = QString::number(from);
    QString toText = QString::number(to);
    QString diff = QString::number(to - from);

    insertSpaceSeparators(fromText);
    insertSpaceSeparators(toText);
    insertSpaceSeparators(diff);

    QString selectedRegionText = tr("%1 to %2 (%3 bp)").arg(fromText, toText, diff);
    QRect srtRect = QRect(0, 0, fontMetrics.width(selectedRegionText), fontMetrics.height());
    srtRect.translate(rect().width() - srtRect.width() - xoffset, rect().height() - yoffset - grtRect.height());
    if(rect().contains(srtRect) && !srtRect.intersects(grtRect)) {
        p.setPen(Qt::gray);
        p.drawText(srtRect, /*Qt::AlignCenter, */selectedRegionText);
    }
}

qint64 AssemblyOverview::calcXAssemblyCoord(int x) {
    U2OpStatusImpl status;
    qint64 result = double(model->getModelLength(status)) / width() * x + 0.5;
    return result;
}

qint64 AssemblyOverview::calcYAssemblyCoord(int y) {
    U2OpStatusImpl status;
    qint64 result = double(model->getModelHeight(status)) / height() * y + 0.5;
    return result;
}

QRect AssemblyOverview::calcCurrentSelection() const {
    U2OpStatusImpl status;
    int w = rect().width();
    int h = rect().height();

    int x_pix_start = double(w) / model->getModelLength(status) * window->getXOffsetInAssembly() + 0.5;
    int y_pix_start = double(h) / model->getModelHeight(status) * window->getYOffsetInAssembly() + 0.5;
    int pix_width = double(w) / model->getModelLength(status) * window->basesVisible() + 0.5;
    int pix_height = double(h) / model->getModelHeight(status) * window->rowsVisible() + 0.5;
    
    return QRect(x_pix_start, y_pix_start, pix_width, pix_height);
}

//prevents selection from crossing widget borders. 
//Tries to move selection center to pos.
void AssemblyOverview::moveSelectionToPos( QPoint pos, bool moveModel )
{
    const QRect & thisRect = rect();
    QRect newSelection(cachedSelection);
    newSelection.moveCenter(pos);
    
    int dy = 0;
    int dx = 0;

    if(!thisRect.contains(newSelection/*, true /*entirely inside*/)) {
        QRect uneeon = rect().united(newSelection);
        if(uneeon.bottom() > thisRect.height()) {
            dy = uneeon.bottom() - thisRect.height();
        } else if(uneeon.top() < 0) {
            dy = uneeon.top();
        }
        if(uneeon.right() > thisRect.right()) {
            dx = uneeon.right() - thisRect.right();
        } else if(uneeon.left() < 0) {
            dx = uneeon.left();
        }
        newSelection.translate(-dx, -dy);
    }

    U2OpStatusImpl status;
    qint64 newXoffset = 0;
    qint64 newYoffset = 0;
    if(dx) {
        newXoffset = (dx < 0) ? 0 : model->getModelLength(status) - window->basesVisible();
        moveModel = true;
    } else {
        newXoffset = calcXAssemblyCoord(newSelection.x());
    }
    if(dy) {
        newYoffset = (dy < 0) ? 0 : model->getModelHeight(status) - window->rowsVisible();
        moveModel = true;
    } else {
        newYoffset = calcYAssemblyCoord(newSelection.y());
    }
    
    if(moveModel) {
        window->setXOffsetInAssembly(newXoffset);
        window->setYOffsetInAssembly(newYoffset);
    } 
}

void AssemblyOverview::paintEvent(QPaintEvent * e) {
    drawAll();
    QWidget::paintEvent(e);
}

void AssemblyOverview::resizeEvent(QResizeEvent * e) {
    cachedSelection = calcCurrentSelection();
    moveSelectionToPos(cachedSelection.center(), false);
    bgrRenderer.render(size());
    sl_redraw();
    QWidget::resizeEvent(e);
}

void AssemblyOverview::mousePressEvent(QMouseEvent * me) {
    if (me->button() == Qt::LeftButton) {
        scribbling = true;
        moveSelectionToPos(me->pos());
    }

    QWidget::mousePressEvent(me);
}

void AssemblyOverview::mouseMoveEvent(QMouseEvent * me) {
    if((me->buttons() & Qt::LeftButton) && scribbling) {
        moveSelectionToPos(me->pos());
    }
    QWidget::mouseMoveEvent(me);
}

void AssemblyOverview::mouseReleaseEvent(QMouseEvent * me) {
    if(me->button() == Qt::LeftButton && scribbling) {
        scribbling = false;
    }
    QWidget::mouseReleaseEvent(me);
}

void AssemblyOverview::sl_visibleAreaChanged() {
    cachedSelection = calcCurrentSelection();
    moveSelectionToPos(cachedSelection.center(), false);
    sl_redraw();
}

void AssemblyOverview::sl_redraw() {
    initSelectionRedraw();
    update();
}


//==============================================================================
// AssemblyOverviewRenderTask
//==============================================================================

AssemblyOverviewRenderTask::AssemblyOverviewRenderTask(AssemblyModel * model_, QSize imageSize) :
Task(tr("Assembly overview renderer"), TaskFlag_None), model(model_), result(imageSize, QImage::Format_ARGB32_Premultiplied) {
    //TODO: progress
}

void AssemblyOverviewRenderTask::run() {
    QPainter p(&result);
    p.fillRect(result.rect(), Qt::white);

    U2OpStatusImpl status;
    quint64 alignmentLen = model->getModelLength(status);
    if(status.hasError()) {
        stateInfo.setError(status.getError());
        return;
    }

    quint64 widgetWidth = result.width();
    quint64 widgetHeight = result.height();

    //FIXME can be zero
    quint64 lettersPerXPixel = alignmentLen / widgetWidth;

    QVector<quint64> readsPerXPixels(widgetWidth);
    quint64 maxReadsPerXPixels = 0;
    quint64 start = 0; 
    //TODO progress
    for(int i = 0 ; i < widgetWidth; ++i) {
        if(stateInfo.cancelFlag) {
            return;
        }
        quint64 readsPerXPixel = model->countReadsInAssembly(0, U2Region(start, lettersPerXPixel), status);
        if(status.hasError()) {
            stateInfo.setError(status.getError());
            return;
        }
        readsPerXPixels[i] = readsPerXPixel;
        start += lettersPerXPixel;
        if(maxReadsPerXPixels < readsPerXPixel) {
            maxReadsPerXPixels = readsPerXPixel;
        }
    }

    //static double logMax = log((double)maxReadsPerXPixels);
    double readsPerYPixel = double(maxReadsPerXPixels) / widgetHeight; 
    //double readsPerYPixel = double(logMax) / widgetHeight; 

    for(int i = 0 ; i < widgetWidth; ++i) {
        quint64 columnPixels = qint64(double(readsPerXPixels[i]) / readsPerYPixel + 0.5);
        //quint64 columnPixels = qint64(double(log((double)readsPerXPixels[i])) / readsPerYPixel + 0.5);
        int grayCoeff = 255 - int(double(255) / maxReadsPerXPixels * readsPerXPixels[i] + 0.5);
        //int grayCoeff = 255 - int(double(255) / logMax * log((double)readsPerXPixels[i]) + 0.5);
        QColor c = QColor(grayCoeff, grayCoeff, grayCoeff);
        p.setPen(c);

        p.drawLine(i, 0, i, columnPixels);
    }
    p.setPen(Qt::gray);
    p.drawRect(result.rect().adjusted(0,0,-1,-1));
}

//==============================================================================
// BackgroundRenderer
//==============================================================================

BackgroundRenderer::BackgroundRenderer(AssemblyModel * model_) : renderTask(0), model(model_), redrawRunning(false), redrawNeeded(true) 
{
}

void BackgroundRenderer::render(const QSize & size_)  {
    size = size_;
    if(!model->isEmpty()) {
        if(redrawRunning) {
            assert(renderTask);
            redrawNeeded = true;
            return;
        }
        redrawRunning = true;
        redrawNeeded = false;
        renderTask = new AssemblyOverviewRenderTask(model, size);
        connect(renderTask, SIGNAL(si_stateChanged()), SLOT(sl_redrawFinished()));
        AppContext::getTaskScheduler()->registerTopLevelTask(renderTask);
    }
}

QImage BackgroundRenderer::getImage() const {
    if(redrawRunning) {
        return QImage();
    }
    assert(!renderTask);
    return result;
}

void BackgroundRenderer::sl_redrawFinished() {
    assert(renderTask == sender());
    if(Task::State_Finished != renderTask->getState()) {
        return;
    }
    assert(redrawRunning);
    redrawRunning = false;
    if(redrawNeeded) {
        render(size);
        redrawRunning = true;
        redrawNeeded = false;
    } else {
        result = renderTask->getResult();
        emit(si_rendered());
        renderTask = 0;
    }
}

} //ns
