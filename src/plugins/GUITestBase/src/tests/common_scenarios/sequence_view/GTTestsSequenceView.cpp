/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "GTTestsSequenceView.h"
#include "api/GTGlobals.h"
#include "api/GTWidget.h"
#include "api/GTMenu.h"
#include "GTUtilsDialog.h"
#include "GTUtilsProject.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsMdi.h"

/*
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTMenu.h"
#include "api/GTFileDialog.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsApp.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsMdi.h"
#include <U2Core/DocumentModel.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>
#include <U2View/ADVConstants.h>
*/
#include <U2Core/AppContext.h>
#include <U2View/ADVConstants.h>

namespace U2 {

namespace GUITest_common_scenarios_sequence_view {

GUI_TEST_CLASS_DEFINITION(test_0005) {

    class AllActionsPopupChooser : public GTUtilsDialog::PopupChooser {
    public:
        AllActionsPopupChooser(U2OpStatus &_os) : GTUtilsDialog::PopupChooser(_os, QStringList()) {}
        void run() {
            GTGlobals::sleep(100);
            QMenu* activePopupMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());

            int i = 0;
            QList<QAction*> act = activePopupMenu->actions();
            foreach (QAction* a, act) {
                QString s = a->objectName();
                if (i == 1) {
                    GTMenu::clickMenuItem(os, activePopupMenu, s, GTGlobals::UseMouse);
                }
                GTGlobals::sleep(100);
                i++;
            }
        }
    } allActionsPopupChooser(os);

    GTUtilsDialog::SequenceReadingModeSelectorDialogFiller dialog(os);
    GTUtilsDialog::preWaitForDialog(os, &dialog, GUIDialogWaiter::Modal);
    GTUtilsProject::openFiles(os, testDir + "_common_data/fasta/multy_fa.fa");
    GTUtilsDocument::checkDocument(os, "multy_fa.fa");

    QWidget* toggleAutoAnnotationsButton = GTWidget::findWidget(os, "toggleAutoAnnotationsButton");
    CHECK_SET_ERR(toggleAutoAnnotationsButton != NULL, "toggleAutoAnnotationsButton is NULL");
    CHECK_SET_ERR(toggleAutoAnnotationsButton->isEnabled() == false, "toggleAutoAnnotationsButton is enabled, expected disabled");


    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsDocument::checkDocument(os, "human_T1.fa");

    toggleAutoAnnotationsButton = GTWidget::findWidget(os, "toggleAutoAnnotationsButton");
    CHECK_SET_ERR(toggleAutoAnnotationsButton != NULL, "toggleAutoAnnotationsButton is NULL");
    CHECK_SET_ERR(toggleAutoAnnotationsButton->isEnabled() == true, "toggleAutoAnnotationsButton is disabled, expected enabled");

    GTUtilsDialog::preWaitForDialog(os, &allActionsPopupChooser, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleAutoAnnotationsButton);
    GTGlobals::sleep(1000);
}

} // namespace GUITest_common_scenarios_sequence_view

} // namespace U2
