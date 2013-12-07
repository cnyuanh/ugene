/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "GTFileDialog.h"
#include "GTMenu.h"
#include "GTKeyboardDriver.h"
#include "GTMouseDriver.h"
#include "GTComboBox.h"
#include "api/GTGlobals.h"
#include "api/GTLineEdit.h"
#include "GTWidget.h"

#include <U2Gui/MainWindow.h>
#include <QtGui/QApplication>
#include <QtGui/QLineEdit>
#include <QtGui/QTreeView>
#include <QtGui/QFileSystemModel>
#include <QtGui/QHeaderView>
#include <QtGui/QFileDialog>
#include <QtGui/QPushButton>

#define FILE_NAME_LINE_EDIT "fileNameEdit"

namespace U2 {

#define GT_CLASS_NAME "GTFileDialogUtils"

GTFileDialogUtils::GTFileDialogUtils(U2OpStatus &_os, const QString &_path, const QString &_fileName,
                                     Button _button, GTGlobals::UseMethod _method) :
    Filler(_os, "QFileDialog"),
    fileName(_fileName),
    button(_button),
    method(_method)

{   path = QDir::cleanPath(QDir::currentPath() + "/" + _path);
    if (path.at(path.count() - 1) != '/') {
        path += '/';
    }
}

GTFileDialogUtils::GTFileDialogUtils(U2OpStatus &os, const QString &filePath, GTGlobals::UseMethod method) :
    Filler(os, "QFileDialog"),
    button(Open),
    method(method)

{
    QFileInfo fileInfo(filePath);
    path = fileInfo.absoluteDir().absolutePath();
    fileName = fileInfo.fileName();
    if (path.at(path.count() - 1) != '/') {
        path += '/';
    }
}

#define GT_METHOD_NAME "run"
void GTFileDialogUtils::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL && QString(dialog->metaObject()->className()) == "QFileDialog",
                   "file dialog not found");

    fileDialog = dialog;
    GTGlobals::sleep(300);
    setPath();
    GTGlobals::sleep(300);
    if(button == Choose){
        clickButton(button);
        return;
    }

    clickButton(Open);
    GTGlobals::sleep(300);

    if(button == Save){//saving file
        setName();
        clickButton(button);
        return;
    }

    //opening file or getting size
    GTGlobals::sleep(300);
    setViewMode(Detail);
    GTGlobals::sleep(300);
    selectFile();
    GTGlobals::sleep(300);
    if(method == GTGlobals::UseKey){
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    }else{
        clickButton(button);
    }

}
#undef GT_METHOD_NAME

GTFileDialogUtils_list::GTFileDialogUtils_list(U2OpStatus &_os, const QString &_path, const QStringList &_fileNameList) :
    GTFileDialogUtils(_os,_path, "", Open, GTGlobals::UseMouse),
    fileNameList(_fileNameList)
{
    path = QDir::cleanPath(QDir::currentPath() + "/" + _path);
    if (path.at(path.count() - 1) != '/') {
        path += '/';
    }
}

#define GT_METHOD_NAME "GTFileDialogUtils_list run"
void GTFileDialogUtils_list::run(){
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL && QString(dialog->metaObject()->className()) == "QFileDialog",
                   "file dialog not found");

    fileDialog = dialog;
    GTGlobals::sleep(200);
    setPath();
    GTGlobals::sleep(200);
    clickButton(Open);
    GTGlobals::sleep(200);
    setViewMode(Detail);
    GTGlobals::sleep(200);
    setNameList(os,fileNameList);
    GTGlobals::sleep(200);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setNameList"
void GTFileDialogUtils_list::setNameList(U2OpStatus &os, const QStringList & nameList){
    QString str;
    foreach (QString name, nameList){
        str.append('\"' + name + "\" ");
    }
    QLineEdit* fileEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os,FILE_NAME_LINE_EDIT));
    GTLineEdit::setText(os,fileEdit,str);
}
#undef GT_METHOD_NAME

void GTFileDialogUtils_list::selectFile(){
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["ctrl"]);
    foreach(QString name, fileNameList){
        GTFileDialogUtils::fileName = name;
        GTFileDialogUtils::selectFile();
    }
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["ctrl"]);
}
void GTFileDialogUtils::openFileDialog()
{
    QMenu *menu;
    QStringList itemPath;
    itemPath << ACTION_PROJECTSUPPORT__OPEN_PROJECT;

    switch(method) {
    case GTGlobals::UseMouse:
        menu = GTMenu::showMainMenu(os, MWMENU_FILE, method);
        GTMenu::clickMenuItem(os, menu, itemPath);
        break;

    case GTGlobals::UseKey:
        GTKeyboardDriver::keyClick(os, 'O', GTKeyboardDriver::key["ctrl"]);
        break;
    default:
        break;
    }
    GTGlobals::sleep(500);
}

#define GT_METHOD_NAME "setPath"
void GTFileDialogUtils::setPath()
{
    QLineEdit* lineEdit = fileDialog->findChild<QLineEdit*>(FILE_NAME_LINE_EDIT);
    GT_CHECK(lineEdit != 0, QString("line edit \"1\" not found").arg(FILE_NAME_LINE_EDIT));
    lineEdit->setCompleter(NULL);
    GTLineEdit::setText(os,lineEdit,path);

    GT_CHECK(lineEdit->text() == path, "Can't open file \"" + lineEdit->text() + "\"");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setName"
void GTFileDialogUtils::setName()
{
    QLineEdit* lineEdit = fileDialog->findChild<QLineEdit*>(FILE_NAME_LINE_EDIT);
    GT_CHECK(lineEdit != 0, QString("line edit \"1\" not found").arg(FILE_NAME_LINE_EDIT));
    lineEdit->setCompleter(NULL);

    GTLineEdit::setText(os, lineEdit,fileName);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "selectFile"
void GTFileDialogUtils::selectFile()
{
    QTreeView *w = fileDialog->findChild<QTreeView*>("treeView");
    GT_CHECK(w != NULL, "widget, which contains list of file, not found");

    QFileSystemModel *model = qobject_cast<QFileSystemModel*>(w->model());
    QModelIndex index = model->index(path + fileName);
    GT_CHECK(index.isValid(), "File <" + path + fileName + "> not found");

    QPoint indexCenter;

    switch(method) {
    case GTGlobals::UseKey:{
        QLineEdit* lineEdit = fileDialog->findChild<QLineEdit*>(FILE_NAME_LINE_EDIT);
        GT_CHECK(lineEdit != 0, QString("line edit \"1\" not found").arg(FILE_NAME_LINE_EDIT));
        GTLineEdit::setText(os,lineEdit,fileName);

        GTWidget::click(os,lineEdit);
        break;
    }

    case GTGlobals::UseMouse:
        w->scrollTo(index);
        indexCenter = w->visualRect(index).center();
        indexCenter.setY(indexCenter.y() + w->header()->rect().height());
        GTMouseDriver::moveTo(os, w->mapToGlobal(indexCenter));
        GTMouseDriver::click(os);
        break;
    default:
        break;
    }

    GTGlobals::sleep(100);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickButton"
void GTFileDialogUtils::clickButton(Button btn)
{
    QMap<Button, QString> button;
    button[Open] = "Open";
    button[Cancel] = "Cancel";
    button[Save] = "Save";
    button[Choose] = "Choose";

    QAbstractButton *button_to_click = GTWidget::findButtonByText(os, button[btn],fileDialog);
    GT_CHECK(button_to_click != NULL, "button not found");

    while (! button_to_click->isEnabled()) {
        GTGlobals::sleep(100);
    }

    GTGlobals::sleep(500);

    switch(method) {
    case GTGlobals::UseKey:
        while (! button_to_click->hasFocus()) {
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["tab"]);
            GTGlobals::sleep(100);
        }
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
        break;

    case GTGlobals::UseMouse:
        GTWidget::click(os, button_to_click);
        GTGlobals::sleep(100);
        break;
    default:
        break;
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setViewMode"
void GTFileDialogUtils::setViewMode(ViewMode v)
{
    QMap<ViewMode, QString> button;
    button[List] = "listModeButton";
    button[Detail] = "detailModeButton";
    QWidget *w = fileDialog->findChild<QWidget*>(button[v]);

    GT_CHECK(w != NULL, "view mode button not found");

    switch(method) {
    case GTGlobals::UseMouse:
        GTWidget::click(os, w);
        break;

    case GTGlobals::UseKey:
        while (! w->hasFocus()) {
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["tab"]);
            GTGlobals::sleep(100);
        }
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
        break;

    default:
        break;
    }

    GTGlobals::sleep(100);
}
#undef GT_METHOD_NAME

void GTFileDialog::openFile(U2OpStatus &os, const QString &path, const QString &fileName,
                            Button button, GTGlobals::UseMethod m)
{
    GTFileDialogUtils *ob = new GTFileDialogUtils(os, path, fileName, (GTFileDialogUtils::Button)button, m);
    GTUtilsDialog::waitForDialog(os, ob);

    ob->openFileDialog();

    GTGlobals::sleep();
}

void GTFileDialog::openFileList(U2OpStatus &os, const QString &path, const QStringList &fileNameList)
{
    GTFileDialogUtils_list *ob = new GTFileDialogUtils_list(os, path, fileNameList);
    GTUtilsDialog::waitForDialog(os, ob);

    ob->openFileDialog();

    GTGlobals::sleep();
}

#undef GT_CLASS_NAME

} // namespace
