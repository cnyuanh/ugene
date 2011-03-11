#include "AssemblyRuler.h"

#include <math.h>

#include <QtCore/QLine>
#include <QtGui/QPainter>
#include "AssemblyBrowser.h"

#include "AssemblyReadsArea.h"
namespace U2 {

AssemblyRuler::AssemblyRuler(AssemblyBrowserUi * ui) :
ui(ui), window(ui->getWindow()), model(ui->getModel())
{
    setFixedHeight(FIXED_HEIGHT);
    connectSlots();
    sl_redraw();
}

void AssemblyRuler::connectSlots() {
    connect(window, SIGNAL(si_zoomOperationPerformed()), SLOT(sl_redraw()));
    connect(window, SIGNAL(si_offsetsChanged()), SLOT(sl_redraw()));
}

void AssemblyRuler::drawAll() {
    if(!model->isEmpty()) {
        if (redraw) {
            cachedView.fill(Qt::transparent);
            QPainter p(&cachedView);
            redraw = false;
            drawRuler(p);
        }
        QPainter p(this);
        p.drawPixmap(0, 0, cachedView);
    }
}

namespace {
int numOfDigits(qint64 n) {
    assert(n >= 0);
    return QString::number(n).length();
}
}

void AssemblyRuler::drawRuler(QPainter & p) {
    //TODO just rewrite it all
    p.setPen(Qt::gray);
    {
        int lineLevel = height() / 4;
        int lowBorder = height() - 5;


        p.drawLine(0, lineLevel, width(), lineLevel);

        p.drawLine(0, 2, 0, lowBorder);
        p.drawLine(width()-1, 2, width()-1, lowBorder);
    }

    int lettersPerZ = window->calcAsmCoord(50);
    int interval = pow((double)10, (int)numOfDigits(lettersPerZ)-1);
    int pixInterval = window->calcPixelCoord(interval);
    
    int globalOffset = window->getXOffsetInAssembly();
    int firstLetterToMark = (globalOffset + interval)/ interval * interval;
    int distToFLTM = firstLetterToMark - globalOffset;

    int end = window->basesCanBeVisible();
    int z = interval * 10;
    int halfCell = window->getCellWidth() / 2;
    for(int i = distToFLTM; i < end; i+=interval) {
        int lowBorder = (height() - 5) / 2;
        int y = 5;
        int x_pix = ui->getReadsArea()->calcPainterOffset(i) + halfCell;
        if((globalOffset + i) % z == 0) {
            lowBorder = lowBorder * 2 - 2;
            y = 3;
        }
        p.drawLine(x_pix, y, x_pix, lowBorder);
    }
}

void AssemblyRuler::paintEvent(QPaintEvent * e) {
    drawAll();
    QWidget::paintEvent(e);
}

void AssemblyRuler::resizeEvent(QResizeEvent * e) {
    sl_redraw();
    QWidget::resizeEvent(e);
}

void AssemblyRuler::mouseMoveEvent(QMouseEvent * e) {
    QWidget::mouseMoveEvent(e);
}

void AssemblyRuler::sl_redraw() {
    cachedView = QPixmap (size());
    redraw = true;
    update();
}

}
