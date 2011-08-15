/**************************************************************************
#    Copyright 2010-2011 Raphaël François
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include "projectfile/projectfile.h"
#include "views/configdialog.h"
#include "views/aboutdialog.h"
#include "objectinfo.h"
#include "views/viewconfigdialog.h"
#include "models/programsmodel.h"
#include "views/vstpluginwindow.h"
#include "views/scripteditor.h"
#include "events.h"

MainWindow::MainWindow(MainHost * myHost,QWidget *parent) :
    QMainWindow(parent),
    mySceneView(0),
    listToolsModel(0),
    listVstPluginsModel(0),
    listVstBanksModel(0),
    ui(new Ui::MainWindow),
    myHost(myHost),
    viewConfig( new View::ViewConfig(myHost,this)),
    viewConfigDlg(0),
    lastMessageResult(0)
{
    myHost->mainWindow=this;
    myHost->AddEventsListener(this);
    AddEventsListener(myHost);

    hostModel = new HostModel(myHost,this);

//    hostModel->setColumnCount(1);

    connect(this, SIGNAL(askLoadProject(QString)),
            myHost, SLOT(LoadProjectFile(QString)));
    connect(this, SIGNAL(askSaveProject(bool)),
            myHost, SLOT(SaveProjectFile(bool)));
    connect(this, SIGNAL(askClearProject()),
            myHost, SLOT(ClearProject()));

    connect(this, SIGNAL(askLoadSetup(QString)),
            myHost, SLOT(LoadSetupFile(QString)));
    connect(this, SIGNAL(askSaveSetup(bool)),
            myHost, SLOT(SaveSetupFile(bool)));
    connect(this, SIGNAL(askClearSetup()),
            myHost, SLOT(ClearSetup()));

    connect(myHost,SIGNAL(programParkingModelChanged(QStandardItemModel*)),
            this,SLOT(programParkingModelChanges(QStandardItemModel*)));
    connect(myHost,SIGNAL(groupParkingModelChanged(QStandardItemModel*)),
            this,SLOT(groupParkingModelChanges(QStandardItemModel*)));
    connect(myHost,SIGNAL(currentFileChanged()),
            this,SLOT(currentFileChanged()));

    ui->setupUi(this);
    ui->statusBar->hide();

    connect(ui->mainToolBar, SIGNAL(visibilityChanged(bool)),
            ui->actionTool_bar, SLOT(setChecked(bool)));

    SetupBrowsersModels( ConfigDialog::defaultVstPath(myHost), ConfigDialog::defaultBankPath(myHost));


//    ui->treeHostModel->setModel(hostModel);

    QAction *undo = myHost->GetUndoStack()->createUndoAction(ui->mainToolBar);
    undo->setIcon(QIcon(":/img16x16/undo.png"));
    undo->setShortcut( QKeySequence::Undo );
    undo->setShortcutContext(Qt::ApplicationShortcut);
    ui->mainToolBar->addAction( undo );

    QAction *redo = myHost->GetUndoStack()->createRedoAction(ui->mainToolBar);
    redo->setIcon(QIcon(":/img16x16/redo.png"));
    redo->setShortcut( QKeySequence::Redo );
    redo->setShortcutContext(Qt::ApplicationShortcut);
    ui->mainToolBar->addAction( redo );

    ui->listUndo->setStack( myHost->GetUndoStack() );
}

void MainWindow::Init()
{

    //programs
//    myHost->programsModel = new ProgramsModel(myHost);
    ui->Programs->SetModel( myHost->programsModel );
    myHost->programsModel->UserChangeProg(0);

    mySceneView = new View::SceneView(myHost, ui->hostView, ui->projectView, ui->programView, ui->groupView, this);
    mySceneView->SetParkings(ui->programParkList, ui->groupParkList);
//    mySceneView->setModel(hostModel);

    ui->solverView->setModel(myHost->GetRendererModel());
    connect(myHost->GetRendererModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            ui->solverView, SLOT(resizeColumnsToContents()));
    connect(myHost->GetRendererModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            ui->solverView, SLOT(resizeRowsToContents()));

    setPalette( viewConfig->GetPaletteFromColorGroup( ColorGroups::Window, palette() ));
    connect( viewConfig, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
             myHost->programsModel, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)) );
    connect( viewConfig, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
             this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)));

}

bool MainWindow::event(QEvent *event)
{
    switch(event->type()) {
        case Events::typeNewObj : {
            Events::sendObj *e = static_cast<Events::sendObj*>(event);
            e->objInfo.SetModel(hostModel);
//            LOG("add" << e->objInfo.toStringFull());
            mySceneView->AddObj(e->objInfo);
            return true;
        }
        case Events::typeDelObj : {
            Events::delObj *e = static_cast<Events::delObj*>(event);
//            LOG("del" << e->objId);
            mySceneView->DelObj(e->objId);
            return true;
        }
        case Events::typeUpdateObj : {
            Events::sendObj *e = static_cast<Events::sendObj*>(event);
            e->objInfo.SetModel(hostModel);
//            LOG("update" << e->objInfo.toStringFull());
            mySceneView->UpdateObj(e->objInfo);
            return true;
        }
    }

    return QMainWindow::event(event);
}

void MainWindow::DisplayMessage(QMessageBox::Icon icon,const QString &text, const QString &info, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox msgBox;
    msgBox.setIcon(icon);
    msgBox.setText(text);
    msgBox.setInformativeText(info);
    msgBox.setStandardButtons(buttons);
    msgBox.setDefaultButton(defaultButton);
    lastMessageResult=msgBox.exec();
}

void MainWindow::CreateNewPluginWindow(QObject* obj)
{
    View::VstPluginWindow *editorWnd = new View::VstPluginWindow(this);
    if(!editorWnd->SetPlugin(obj))
        editorWnd->close();
}
void MainWindow::CreateNewScriptEditor(QObject* obj)
{
    View::ScriptEditor *editorWnd = new View::ScriptEditor(this);
    static_cast<Connectables::Object*>(obj)->SetEditorWnd(editorWnd);
}

void MainWindow::SetupBrowsersModels(const QString &vstPath, const QString &browserPath)
{
#if !defined(__GNUC__)
    //vst plugins browser
    //sse2 crash with gcc ?

    listVstPluginsModel = new QFileSystemModel(this);
    listVstPluginsModel->setReadOnly(true);
    listVstPluginsModel->setResolveSymlinks(true);
    QStringList fileFilter;
    fileFilter << "*.dll";
    listVstPluginsModel->setNameFilters(fileFilter);
    listVstPluginsModel->setNameFilterDisables(false);
    listVstPluginsModel->setRootPath(vstPath);
    ui->VstBrowser->setModel(listVstPluginsModel);

    //bank file browser
    listVstBanksModel = new QFileSystemModel(this);
    listVstBanksModel->setReadOnly(false);
    listVstBanksModel->setResolveSymlinks(true);
    //    QStringList bankFilter;
    //    bankFilter << "*.fxb";
    //    bankFilter << "*.fxp";
    //    listVstBanksModel->setNameFilters(bankFilter);
    //    listVstBanksModel->setNameFilterDisables(false);
    listVstBanksModel->setRootPath(browserPath);
    ui->BankBrowser->setModel(listVstBanksModel);
#endif
}

MainWindow::~MainWindow()
{
    if(ui)
        delete ui;
}

void MainWindow::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId!=ColorGroups::Window)
        return;

    QPalette::ColorRole role = viewConfig->GetPaletteRoleFromColor(colorId);

    QPalette pal=palette();
    pal.setColor(role, color);
    setPalette(pal);
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::BuildListTools()
{
    QStringList headerLabels;
    headerLabels << tr("Name");

    listToolsModel = new ListToolsModel(this);
    listToolsModel->setHorizontalHeaderLabels(headerLabels);
    QStandardItem *parentItem = listToolsModel->invisibleRootItem();

#ifdef SCRIPTENGINE
    {
        //script
        MetaInfo info(MetaTypes::object);
        info.SetMeta(MetaInfos::ObjType, ObjTypes::Script);

        QStandardItem *item = new QStandardItem(tr("Script"));
        item->setData(QVariant::fromValue(info), UserRoles::objInfo);
        parentItem->appendRow(item);
    }
#endif

    {
        //midi parameters
        MetaInfo info(MetaTypes::object);
        info.SetMeta(MetaInfos::ObjType, ObjTypes::MidiToAutomation);

        QStandardItem *item = new QStandardItem(tr("Midi to parameter"));
        item->setData(QVariant::fromValue(info), UserRoles::objInfo);
        parentItem->appendRow(item);
    }

    {
        //midi sender
        MetaInfo info(MetaTypes::object);
        info.SetMeta(MetaInfos::ObjType, ObjTypes::MidiSender);

        QStandardItem *item = new QStandardItem(tr("Midi sender"));
        item->setData(QVariant::fromValue(info),UserRoles::objInfo);
        parentItem->appendRow(item);
    }

    {
        //host controller
        MetaInfo info(MetaTypes::object);
        info.SetMeta(MetaInfos::ObjType, ObjTypes::HostController);

        QStandardItem *item = new QStandardItem(tr("Host Controller"));
        item->setData(QVariant::fromValue(info),UserRoles::objInfo);
        parentItem->appendRow(item);
    }

    ui->treeTools->setModel(listToolsModel);
    ui->treeTools->header()->setResizeMode(0,QHeaderView::Stretch);
}

void MainWindow::on_actionLoad_triggered()
{
    emit askLoadProject("");
}

void MainWindow::on_actionNew_triggered()
{
    emit askClearProject();
}

void MainWindow::on_actionSave_triggered()
{
    emit askSaveProject(false);
}

void MainWindow::on_actionSave_Project_As_triggered()
{
    emit askSaveProject(true);
}

void MainWindow::on_actionLoad_Setup_triggered()
{
    emit askLoadSetup("");
}

void MainWindow::on_actionNew_Setup_triggered()
{
    emit askClearSetup();
}

void MainWindow::on_actionSave_Setup_triggered()
{
    if(myHost->currentSetupFile.isEmpty()) {
        on_actionSave_Setup_As_triggered();
        return;
    }
    emit askSaveSetup(false);
}

void MainWindow::on_actionSave_Setup_As_triggered()
{
    emit askSaveSetup(true);
}

void MainWindow::on_actionConfig_triggered()
{
    ConfigDialog conf(myHost,this);
    conf.exec();
}

void MainWindow::writeSettings()
{
    myHost->SetSetting("MainWindow/geometry", saveGeometry());
    myHost->SetSetting("MainWindow/state", saveState());
//    myHost->SetSetting("MainWindow/statusBar", ui->statusBar->isVisible());
    myHost->SetSetting("MainWindow/splitPan", ui->splitterPanels->saveState());
    myHost->SetSetting("MainWindow/splitProg", ui->splitterProg->saveState());
    myHost->SetSetting("MainWindow/splitGroup", ui->splitterGroup->saveState());

    myHost->SetSetting("MainWindow/planelHost", ui->actionHost_panel->isChecked());
    myHost->SetSetting("MainWindow/planelProject", ui->actionProject_panel->isChecked());
    myHost->SetSetting("MainWindow/planelProgram", ui->actionProgram_panel->isChecked());
    myHost->SetSetting("MainWindow/planelGroup", ui->actionGroup_panel->isChecked());

    myHost->SetSetting("lastVstPath", ui->VstBrowser->path());
    myHost->SetSetting("lastBankPath", ui->BankBrowser->path());
    ui->Programs->writeSettings(myHost);
    //settings.sync();
}

void MainWindow::readSettings()
{
    QList<QDockWidget*>listDocks;
    listDocks << ui->dockTools;
    listDocks << ui->dockVstBrowser;
    listDocks << ui->dockBankBrowser;
    listDocks << ui->dockPrograms;
    listDocks << ui->dockUndo;
    foreach(QDockWidget *dock, listDocks) {
        ui->menuView->addAction(dock->toggleViewAction());
        ui->mainToolBar->addAction(dock->toggleViewAction());
    }

    QList<QDockWidget*>listDocksNoToolbar;
    listDocksNoToolbar << ui->dockSolver;
    listDocksNoToolbar << ui->dockHostModel;
    foreach(QDockWidget *dock, listDocksNoToolbar) {
        ui->menuView->addAction(dock->toggleViewAction());
    }

    //recent setups
    for(int i=0; i<NB_RECENT_FILES; i++) {
        QAction *act = new QAction(this);
        act->setVisible(false);
        connect(act, SIGNAL(triggered()),
                this, SLOT(openRecentSetup()));

        ui->menuRecent_Setups->addAction(act);

        listRecentSetups << act;
    }

    //recent projects
    for(int i=0; i<NB_RECENT_FILES; i++) {
        QAction *act = new QAction(this);
        act->setVisible(false);
        connect(act, SIGNAL(triggered()),
                this, SLOT(openRecentProject()));

        ui->menuRecent_Projects->addAction(act);

        listRecentProjects << act;
    }

    //window state

    if(myHost->SettingDefined("MainWindow/geometry")) {
        restoreGeometry(myHost->GetSetting("MainWindow/geometry").toByteArray());
        restoreState(myHost->GetSetting("MainWindow/state").toByteArray());
//        bool statusb = myHost->GetSetting("MainWindow/statusBar",false).toBool();
//        ui->actionStatus_bar->setChecked( statusb );
//        ui->statusBar->setVisible(statusb);
    } else {
        resetSettings();
    }

    ui->splitterProg->setStretchFactor(0,100);
    ui->splitterGroup->setStretchFactor(0,100);

    if(myHost->SettingDefined("MainWindow/splitPan"))
        ui->splitterPanels->restoreState(myHost->GetSetting("MainWindow/splitPan").toByteArray());
    if(myHost->SettingDefined("MainWindow/splitProg"))
        ui->splitterProg->restoreState(myHost->GetSetting("MainWindow/splitProg").toByteArray());
    if(myHost->SettingDefined("MainWindow/splitGroup"))
        ui->splitterGroup->restoreState(myHost->GetSetting("MainWindow/splitGroup").toByteArray());

    ui->actionHost_panel->setChecked( myHost->GetSetting("MainWindow/planelHost",true).toBool() );
    ui->actionProject_panel->setChecked( myHost->GetSetting("MainWindow/planelProject",false).toBool() );
    ui->actionProgram_panel->setChecked( myHost->GetSetting("MainWindow/planelProgram",true).toBool() );
    ui->actionGroup_panel->setChecked( myHost->GetSetting("MainWindow/planelGroup",true).toBool() );

    ui->Programs->readSettings(myHost);

    viewConfig->LoadFromRegistry();

}

void MainWindow::LoadDefaultFiles()
{
    //load default files
    QString file = ConfigDialog::defaultSetupFile(myHost);
    if(!file.isEmpty())
        emit askLoadSetup( file );

    file = ConfigDialog::defaultProjectFile(myHost);
    if(!file.isEmpty())
        emit askLoadProject( file );

    updateRecentFileActions();
}

void MainWindow::currentFileChanged()
{
    QFileInfo set(myHost->currentSetupFile);
    QFileInfo proj(myHost->currentProjectFile);
    setWindowTitle(QString("VstBoard %1:%2").arg( set.baseName() ).arg( proj.baseName() ));

    ui->actionSave_Setup_As->setEnabled(!myHost->currentSetupFile.isEmpty());
    ui->actionSave_Project_As->setEnabled(!myHost->currentProjectFile.isEmpty());

    updateRecentFileActions();
}

void MainWindow::resetSettings()
{
    QList<QDockWidget*>listDocksVisible;
    listDocksVisible << ui->dockTools;
    listDocksVisible << ui->dockVstBrowser;
    listDocksVisible << ui->dockBankBrowser;
    listDocksVisible << ui->dockPrograms;
    listDocksVisible << ui->dockUndo;
    foreach(QDockWidget *dock, listDocksVisible) {
        dock->setFloating(false);
        dock->setVisible(true);
    }

    QList<QDockWidget*>listDocksHidden;
    listDocksHidden << ui->dockSolver;
    listDocksHidden << ui->dockHostModel;
    foreach(QDockWidget *dock, listDocksHidden) {
        dock->setFloating(false);
        dock->setVisible(false);
    }

    ui->Programs->resetSettings();

    addDockWidget(Qt::LeftDockWidgetArea,  ui->dockTools);
    addDockWidget(Qt::LeftDockWidgetArea,  ui->dockVstBrowser);
    addDockWidget(Qt::LeftDockWidgetArea,  ui->dockBankBrowser);

    addDockWidget(Qt::RightDockWidgetArea,  ui->dockPrograms);
    addDockWidget(Qt::RightDockWidgetArea,  ui->dockUndo);
    addDockWidget(Qt::RightDockWidgetArea,  ui->dockSolver);
    addDockWidget(Qt::RightDockWidgetArea,  ui->dockHostModel);

    tabifyDockWidget(ui->dockHostModel,ui->dockSolver);

    ui->actionHost_panel->setChecked(true);
    ui->actionProject_panel->setChecked(false);
    ui->actionProgram_panel->setChecked(true);
    ui->actionGroup_panel->setChecked(true);

    ui->actionTool_bar->setChecked(true);
//    ui->actionStatus_bar->setChecked(false);
    ui->statusBar->setVisible(false);

    int h = ui->splitterPanels->height()/4;
    QList<int>heights;
    heights << h << h << h << h;
    ui->splitterPanels->setSizes(heights);

    QList<int> szProg;
    szProg << 1000 << 100;
    ui->splitterProg->setSizes(szProg);

    QList<int> szGrp;
    szGrp << 1000 << 100;
    ui->splitterGroup->setSizes(szGrp);
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog about;
    about.exec();
}

void MainWindow::updateRecentFileActions()
{
    {
        QStringList files = myHost->GetSetting("recentSetupFiles").toStringList();

        int numRecentFiles = qMin(files.size(), (int)NB_RECENT_FILES);

        for (int i = 0; i < numRecentFiles; ++i) {
            QString text = tr("&%1 %2").arg(i + 1).arg( QFileInfo(files[i]).fileName() );
            listRecentSetups[i]->setText(text);
            listRecentSetups[i]->setData(files[i]);
            listRecentSetups[i]->setVisible(true);
        }
        for (int j = numRecentFiles; j < NB_RECENT_FILES; ++j)
            listRecentSetups[j]->setVisible(false);
    }

    {
        QStringList files = myHost->GetSetting("recentProjectFiles").toStringList();

        int numRecentFiles = qMin(files.size(), (int)NB_RECENT_FILES);

        for (int i = 0; i < numRecentFiles; ++i) {
            QString text = tr("&%1 %2").arg(i + 1).arg( QFileInfo(files[i]).fileName() );
            listRecentProjects[i]->setText(text);
            listRecentProjects[i]->setData(files[i]);
            listRecentProjects[i]->setVisible(true);
        }
        for (int j = numRecentFiles; j < NB_RECENT_FILES; ++j)
            listRecentProjects[j]->setVisible(false);
    }
}

void MainWindow::openRecentSetup()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action)
        return;

    emit askLoadSetup( action->data().toString() );
}

void MainWindow::openRecentProject()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action)
        return;

    emit askLoadProject( action->data().toString() );
}

void MainWindow::programParkingModelChanges(QStandardItemModel *model)
{
    ui->programParkList->setModel(model);
}

void MainWindow::groupParkingModelChanges(QStandardItemModel *model)
{
    ui->groupParkList->setModel(model);
}

void MainWindow::on_actionRestore_default_layout_triggered()
{
    resetSettings();
}

void MainWindow::on_solverView_clicked(const QModelIndex &index)
{
    myHost->OptimizeRenderer();
}

void MainWindow::on_actionAppearance_toggled(bool arg1)
{
    if(arg1) {
        if(viewConfigDlg)
            return;
        viewConfigDlg = new View::ViewConfigDialog(myHost,this);
        connect(viewConfigDlg, SIGNAL(destroyed()),
                this, SLOT(OnViewConfigClosed()));
        viewConfigDlg->setAttribute( Qt::WA_DeleteOnClose, true );
        viewConfigDlg->show();
    } else {
        if(!viewConfigDlg)
            return;
        viewConfigDlg->close();
    }
}

void MainWindow::OnViewConfigClosed()
{
    viewConfigDlg=0;
    ui->actionAppearance->setChecked(false);
}
