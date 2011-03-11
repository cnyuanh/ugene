#ifndef _U2_USER_APP_SETTINGS_GUI_CONTROLLER_H_
#define _U2_USER_APP_SETTINGS_GUI_CONTROLLER_H_

#include <ui/ui_UserApplicationsSettingsWidget.h>

#include <U2Core/NetworkConfiguration.h>
#include <U2Gui/AppSettingsGUI.h>

#include <QtCore/QUrl>

namespace U2 {


class UserApplicationsSettingsPageController : public AppSettingsGUIPageController {
    Q_OBJECT
public:
    UserApplicationsSettingsPageController(QObject* p = NULL);

    virtual AppSettingsGUIPageState* getSavedState();

    virtual void saveState(AppSettingsGUIPageState* s);

    virtual AppSettingsGUIPageWidget* createWidget(AppSettingsGUIPageState* data);
    
    QMap<QString, QString> translations;
};


class UserApplicationsSettingsPageState : public AppSettingsGUIPageState {
    Q_OBJECT
public:
    UserApplicationsSettingsPageState() : useDefaultWebBrowser (true), 
        openLastProjectFlag(false), enableStatistics(false), tabbedWindowLayout(false) {}

    QString webBrowserUrl;
    QString translFile;
    QString style;
    QString downloadsDirPath;
    QString temporaryDirPath;
    bool useDefaultWebBrowser;
    bool openLastProjectFlag;
    bool enableStatistics;
    bool tabbedWindowLayout;
};


class UserApplicationsSettingsPageWidget: public AppSettingsGUIPageWidget, public Ui_UserApplicationsSettingsWidget {
    Q_OBJECT
public:
    UserApplicationsSettingsPageWidget(UserApplicationsSettingsPageController* ctrl);

    virtual void setState(AppSettingsGUIPageState* state);

    virtual AppSettingsGUIPageState* getState(QString& err) const;

private slots:
    void sl_wbURLClicked();
    void sl_transFileClicked();
    void sl_browseButtonClicked();
    void sl_browseTmpDirButtonClicked();
};

}//namespace

#endif
