#ifndef _U2_PROJECT_SUPPORT_H_
#define _U2_PROJECT_SUPPORT_H_

#include "ui/ui_CreateNewProjectWidget.h"

#include <U2Core/ProjectModel.h>
#include <U2Core/ProjectService.h>
#include <assert.h>

#include <QtGui/QtGui>

namespace U2 {

#define SETTINGS_DIR QString("project_loader/")
#define RECENT_ITEMS_SETTINGS_NAME "recentItems"
#define RECENT_PROJECTS_SETTINGS_NAME "recentProjects"

class ProjectLoaderImpl : public ProjectLoader {
    Q_OBJECT
public:
    ProjectLoaderImpl();
	~ProjectLoaderImpl();

    virtual Task* openProjectTask(const QString& file, bool closeActiveProject);
    virtual Task* openProjectTask(const QList<GUrl>& urls, bool closeActiveProject);
    virtual Project* createProject(const QString& name, const QString& url, QList<Document*>& documents, QList<GObjectViewState*>& states);

    static QString getLastProjectURL();

private:
    void updateState();
	void updateRecentProjectsMenu();
    void prependToRecentProjects(const QString& pFile);
    void updateRecentItemsMenu();
    void prependToRecentItems(const QString& url);
    void rememberProjectURL();

private slots:
    void sl_newProject();
    void sl_newDocumentFromText();
    void sl_openProject();
	void sl_openRecentFile();
    void sl_openRecentProject();
    void sl_serviceStateChanged(Service* s, ServiceState prevState);
    void sl_documentAdded(Document* doc);
    void sl_documentStateChanged();
    void sl_projectURLChanged(const QString& oldURL);
    void sl_projectOpened();

	void sl_downloadRemoteFile();


// QT 4.5.0 bug workaround
    void sl_updateRecentItemsMenu();
    
private:
    
	QAction* newProjectAction;
	QAction* openProjectAction;
    QAction* downloadRemoteFileAction;
    QAction* newDocumentFromtext;
    QAction* separatorAction1;
	QAction* separatorAction2;

	QMenu* recentProjectsMenu;
    QMenu* recentItemsMenu;
};


//////////////////////////////////////////////////////////////////////////
/// Dialogs

//TODO: merge project dir & project name fields

class ProjectDialogController : public QDialog, public Ui::CreateNewProjectDialog {
    Q_OBJECT
public:
    enum Mode {New_Project, Save_Project};
    ProjectDialogController(Mode m, QWidget *p);

    void updateState();

protected:
    void keyPressEvent ( QKeyEvent * event );

private slots:
    void sl_folderSelectClicked();
    void sl_fileNameEdited(const QString&);
    void sl_projectNameEdited(const QString&);
private:
    bool fileEditIsEmpty;
};


}//namespace
#endif
