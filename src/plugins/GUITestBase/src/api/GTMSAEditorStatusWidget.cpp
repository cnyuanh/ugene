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

#include "GTMSAEditorStatusWidget.h"
#include <primitives/GTWidget.h>

namespace U2 {

#define GT_CLASS_NAME "GTMSAEditorStatusWidget"

#define GT_METHOD_NAME "length"
int GTMSAEditorStatusWidget::length(U2OpStatus& os, QWidget* w) {

    QLabel* label = qobject_cast<QLabel*>(GTWidget::findWidget(os, "Column", w));
    GT_CHECK_RESULT(label != NULL, "label is NULL", -1);

    QString labelText = label->text();
    QString lengthString = labelText.section('/', -1, -1);

    bool ok = false;
    int lengthInt = lengthString.toInt(&ok);
    GT_CHECK_RESULT(ok == true, "toInt returned false", -1);

    return lengthInt;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "count"
int GTMSAEditorStatusWidget::getSequencesCount(U2OpStatus &os, QWidget *w) {
    QLabel* label = GTWidget::findExactWidget<QLabel *>(os, "Line", w);
    GT_CHECK_RESULT(label != NULL, "label is NULL", -1);

    QString labelText = label->text();
    QString countString = labelText.section('/', -1, -1);

    bool ok = false;
    int countInt = countString.toInt(&ok);
    GT_CHECK_RESULT(ok == true, "toInt returned false", -1);

    return countInt;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
