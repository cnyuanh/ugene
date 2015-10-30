/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _HI_GUI_GTSCROLLBAR_H_
#define _HI_GUI_GTSCROLLBAR_H_

#include <QtCore/QPoint>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QScrollBar>
#include <QtGui/QStyleOptionSlider>
#else
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStyleOptionSlider>
#endif
#include <drivers/GTMouseDriver.h>
#include <drivers/GTKeyboardDriver.h>


namespace HI {

class HI_EXPORT GTScrollBar {
public:
    static QScrollBar* getScrollBar(U2::U2OpStatus &os, const QString &scrollBarSysName);

    static void pageUp(U2::U2OpStatus &os, QScrollBar *scrollbar, GTGlobals::UseMethod useMethod);
    static void pageDown(U2::U2OpStatus &os, QScrollBar *scrollbar, GTGlobals::UseMethod useMethod);

    static void lineUp(U2::U2OpStatus &os, QScrollBar *scrollbar, GTGlobals::UseMethod useMethod); //does not necessarily move one line up (for example, moves cursor in text editors)
    static void lineDown(U2::U2OpStatus &os, QScrollBar *scrollbar, GTGlobals::UseMethod useMethod); //does not necessarily move one line down (for example, moves cursor in text editors)

    static void moveSliderWithMouseUp(U2::U2OpStatus &os, QScrollBar *scrollbar, int nPix);
    static void moveSliderWithMouseDown(U2::U2OpStatus &os, QScrollBar *scrollbar, int nPix);

    static void moveSliderWithMouseWheelUp(U2::U2OpStatus &os, QScrollBar *scrollbar, int nScrolls); //first moves the cursor to the slider and clicks it, then starts scrolling
    static void moveSliderWithMouseWheelDown(U2::U2OpStatus &os, QScrollBar *scrollbar, int nScrolls); //first moves the cursor to the slider and clicks it, then starts scrolling

    static QPoint getSliderPosition(U2::U2OpStatus &os, QScrollBar *scrollbar);
    static QPoint getUpArrowPosition(U2::U2OpStatus &os, QScrollBar *scrollbar);
    static QPoint getDownArrowPosition(U2::U2OpStatus &os, QScrollBar *scrollbar);
    static QPoint getAreaUnderSliderPosition(U2::U2OpStatus &os, QScrollBar *scrollbar);
    static QPoint getAreaOverSliderPosition(U2::U2OpStatus &os, QScrollBar *scrollbar);

private:
    static QStyleOptionSlider initScrollbarOptions(U2::U2OpStatus &os, QScrollBar *scrollbar);
};

} //namespace

#endif
