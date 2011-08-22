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
#include "mainhost.h"
#include "mainwindow.h"
#include "models/programsmodel.h"
#include "connectables/objects/container.h"

#ifdef VSTSDK
    #include "connectables/objects/vstplugin.h"
    int MainHost::vstUsersCounter=0;
#endif

#include "projectfile/fileversion.h"
#include "projectfile/projectfile.h"
#include "views/configdialog.h"
#include "events.h"

EngineThread::EngineThread(QObject *parent) :
    QThread(parent)
{
    setObjectName("EngineThread");
//    start(QThread::LowPriority);
    start(QThread::TimeCriticalPriority);
}

EngineThread::~EngineThread()
{
    LOG("stop thread"<<objectName()<<(int)currentThreadId());
    quit();
    wait(1000);
}

void EngineThread::run()
{
    LOG("start thread"<<objectName()<<(int)currentThreadId());
    exec();
}

quint32 MainHost::currentFileVersion=PROJECT_AND_SETUP_FILE_VERSION;

MainHost::MainHost(QObject *parent, QString settingsGroup) :
    QObject(parent),
    MetaTransporter(),
    solver(new PathSolver(this)),
    objFactory(0),
    mainWindow(0),
    solverNeedAnUpdate(false),
    solverUpdateEnabled(true),
    mutexListCables(new DMutex(QMutex::Recursive)),
    settingsGroup(settingsGroup),
    undoProgramChangesEnabled(false),
    undoStack(new QUndoStack(this)),
    renderer(0),
    programsModel(0),
    updateViewTimer(0),
    doublePrecision(false)

{
    setObjectName("MainHost");

    SET_MUTEX_NAME((*mutexListCables),"mutexListCables mainhost");
    SET_MUTEX_NAME(mutexRender,"mutexRender mainhost");
    SET_MUTEX_NAME(mutexSolver,"mutexSolver mainhost");

    sampleRate = 44100.0;
    bufferSize = 100;

    currentTempo=120;
    currentTimeSig1=4;
    currentTimeSig2=4;

    doublePrecision=GetSetting("doublePrecision",false).toBool();
    MetaTransporter::autoUpdate=false;

#ifdef VSTSDK
    vstHost=0;
#endif

#ifdef SCRIPTENGINE
    scriptEngine = new QScriptEngine(this);
    QScriptValue scriptObj = scriptEngine->newQObject(this);
    scriptEngine->globalObject().setProperty("MainHost", scriptObj);
#endif
}

MainHost::~MainHost()
{
    RemoveAllListeners();

    EnableSolverUpdate(false);

    updateViewTimer->stop();
    delete updateViewTimer;
    updateViewTimer=0;

    mutexListCables->lock();
    workingListOfCables.clear();
    mutexListCables->unlock();

    solver->Resolve(workingListOfCables, renderer);
    delete renderer;
    renderer=0;

    mainContainer.clear();
    hostContainer.clear();
    projectContainer.clear();
    groupContainer.clear();
    programContainer.clear();

    delete objFactory;

#ifdef VSTSDK
    vstUsersCounter--;
    if(vstUsersCounter==0 && vstHost)
        delete vstHost;
#endif

    delete mutexListCables;
}

void MainHost::InitThread()
{
#ifdef VSTSDK
    if(!vst::CVSTHost::Get())
        vstHost = new vst::CVSTHost();
    else
        vstHost = vst::CVSTHost::Get();

    vstUsersCounter++;
#endif

    renderer = new Renderer(this);

    //timer
    timeFromStart.start();

    updateViewTimer = new QTimer(this);
    updateViewTimer->start(40);

    connect(this,SIGNAL(SolverToUpdate()),
            this,SLOT(UpdateSolver()),
            Qt::QueuedConnection);

    programsModel = new ProgramsModel(this);
}

void MainHost::SetMainWindow(MainWindow *win)
{
    mainWindow=win;

    EnableSolverUpdate(false);
    SetupMainContainer();
    SetupHostContainer();
//    SetupProjectContainer();
//    SetupProgramContainer();
//    SetupGroupContainer();

    EnableSolverUpdate(true);
    programsModel->BuildDefaultModel();
}

bool MainHost::event(QEvent *event)
{
    switch(event->type()) {
        case Events::typeCommand : {
            Events::command *e = static_cast<Events::command*>(event);
            undoStack->push(e->cmd);
            return true;
        }
        case Events::typeValChanged : {
            Events::valChanged *e = static_cast<Events::valChanged*>(event);
            switch(e->objInfo.GetMetaData<int>(metaT::ObjType)) {
                case MetaType::object : {
                    QSharedPointer<Connectables::Object>obj = objFactory->GetObjectFromId(e->objInfo.GetMetaData<quint32>(metaT::ObjId));
                    if(!obj)
                        return true;
                    obj->SetMeta(e->type,e->value);
                    return true;
                }
                case MetaType::pin: {
                    Connectables::Pin *pin = objFactory->GetPin(e->objInfo);
                    if(!pin)
                        return true;

                    Connectables::ParameterPin* p = static_cast<Connectables::ParameterPin*>(pin);
                    p->SetMeta(e->type,e->value);
                    switch(e->type) {
                        case metaT::Value :
                        case metaT::LimitInMin :
                        case metaT::LimitInMax :
                        case metaT::LimitOutMin :
                        case metaT::LimitOutMax :
                            p->ChangeValue(p->GetMetaData<float>(metaT::Value));
                    }
                    return true;
                }
            }
        }
    }

    return QObject::event(event);
}

void MainHost::SetupMainContainer()
{
    MetaData info(MetaType::container, FixedObjIds::mainContainer);
    info.SetMeta<QString>(metaT::ObjName, "mainContainer");

    mainContainer = objFactory->NewObject(info).staticCast< Connectables::Container >();
    if(mainContainer.isNull())
        return;

    mainContainer->SetLoadingMode(true);

    mainContainer->LoadProgram(0);
    mainContainer->Resume();
    Events::sendObj *event = new Events::sendObj( *static_cast<MetaData*>(mainContainer.data()), Events::typeNewObj);
    PostEvent(event);

    mainContainer->parked=false;
    mainContainer->listenProgramChanges=false;

    mainContainer->SetLoadingMode(false);
    mainContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();
}

void MainHost::SetupHostContainer()
{
    if(!hostContainer.isNull()) {
        mainContainer->ParkObject( hostContainer );
        hostContainer.clear();
        UpdateSolver(true);
        if(mainWindow)
            mainWindow->mySceneView->viewHost->ClearViewPrograms();
    }

    MetaData info(MetaType::container, FixedObjIds::hostContainer);
    info.SetMeta<QString>(metaT::ObjName, "hostContainer");

    hostContainer = objFactory->NewObject(info).staticCast<Connectables::Container>();
    if(hostContainer.isNull())
        return;

    hostContainer->SetLoadingMode(true);

    hostContainer->LoadProgram(0);
    mainContainer->AddObject(hostContainer);

    {
        //bridge in
        MetaData info(MetaType::bridge, FixedObjIds::hostContainerIn);
        info.SetMeta<QString>(metaT::ObjName, "bridgeIn");
        info.SetMeta(metaT::Direction, Directions::Input);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        hostContainer->AddObject( bridge );

        hostContainer->bridgeIn = bridge;
    }

    {
        //bridge out
        MetaData info(MetaType::bridge, FixedObjIds::hostContainerOut);
        info.SetMeta<QString>(metaT::ObjName, "bridgeOut");
        info.SetMeta(metaT::Direction, Directions::Output);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        hostContainer->AddObject( bridge );

        hostContainer->bridgeOut = bridge;
    }

    //connect with groupContainer
    if(!groupContainer.isNull()) {
        mainContainer->ConnectObjects(groupContainer->bridgeSend, hostContainer->bridgeIn,true);
        mainContainer->ConnectObjects(hostContainer->bridgeOut, groupContainer->bridgeReturn,true);
    }

    {
        //bridge send
        MetaData info(MetaType::bridge, FixedObjIds::hostContainerSend);
        info.SetMeta<QString>(metaT::ObjName, "bridgeSend");
        info.SetMeta(metaT::Direction, Directions::Send);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        hostContainer->AddObject( bridge );

        hostContainer->bridgeSend = bridge;
    }

    {
        //bridge return
        MetaData info(MetaType::bridge, FixedObjIds::hostContainerReturn);
        info.SetMeta<QString>(metaT::ObjName, "bridgeReturn");
        info.SetMeta(metaT::Direction, Directions::Return);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        hostContainer->AddObject( bridge );

        hostContainer->bridgeReturn = bridge;
    }

    //connect with projectContainer
    if(!projectContainer.isNull()) {
        mainContainer->ConnectObjects(hostContainer->bridgeSend, projectContainer->bridgeIn,true);
        mainContainer->ConnectObjects(projectContainer->bridgeOut, hostContainer->bridgeReturn,true);
    }
    hostContainer->listenProgramChanges=false;

    if(projectContainer)
        hostContainer->childContainer=projectContainer;

    hostContainer->SetLoadingMode(false);
    hostContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();
}

void MainHost::SetupProjectContainer()
{
    if(!projectContainer.isNull()) {
        mainContainer->ParkObject( projectContainer );
        projectContainer.clear();
        UpdateSolver(true);
        if(mainWindow)
            mainWindow->mySceneView->viewProject->ClearViewPrograms();
    }

    timeFromStart.restart();

    MetaData info(MetaType::container);
    info.SetMetaId(FixedObjIds::projectContainer);
    info.SetMeta<QString>(metaT::ObjName, "projectContainer");

    projectContainer = objFactory->NewObject(info).staticCast<Connectables::Container>();
    if(projectContainer.isNull())
        return;

    projectContainer->SetLoadingMode(true);

    projectContainer->LoadProgram(0);
    mainContainer->AddObject(projectContainer);

    {
        //bridge in
        MetaData info(MetaType::bridge,FixedObjIds::projectContainerIn);
        info.SetMeta<QString>(metaT::ObjName, "bridgeIn");
        info.SetMeta(metaT::Direction, Directions::Input);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        projectContainer->AddObject( bridge );

        projectContainer->bridgeIn = bridge;
    }

    {
        //bridge out
        MetaData info(MetaType::bridge,FixedObjIds::projectContainerOut);
        info.SetMeta<QString>(metaT::ObjName, "bridgeOut");
        info.SetMeta(metaT::Direction, Directions::Output);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        projectContainer->AddObject( bridge );

        projectContainer->bridgeOut = bridge;
    }

    //connect with hostContainer
    if(!hostContainer.isNull()) {
        mainContainer->ConnectObjects(hostContainer->bridgeSend, projectContainer->bridgeIn,true);
        mainContainer->ConnectObjects(projectContainer->bridgeOut, hostContainer->bridgeReturn,true);
    }

    {
        //bridge send
        MetaData info(MetaType::bridge,FixedObjIds::projectContainerSend);
        info.SetMeta<QString>(metaT::ObjName, "bridgeSend");
        info.SetMeta(metaT::Direction, Directions::Send);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        projectContainer->AddObject( bridge );

        projectContainer->bridgeSend = bridge;
    }

    {
        //bridge return
        MetaData info(MetaType::bridge,FixedObjIds::projectContainerReturn);
        info.SetMeta<QString>(metaT::ObjName, "bridgeReturn");
        info.SetMeta(metaT::Direction, Directions::Return);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        projectContainer->AddObject( bridge );

        projectContainer->bridgeReturn = bridge;
    }

    //connect with programContainer
    if(!programContainer.isNull()) {
        mainContainer->ConnectObjects(projectContainer->bridgeSend, programContainer->bridgeIn,true);
        mainContainer->ConnectObjects(programContainer->bridgeOut, projectContainer->bridgeReturn,true);
    }

    //connect with itself (pass-though cables)
    projectContainer->ConnectObjects(projectContainer->bridgeIn, projectContainer->bridgeSend,false);
    projectContainer->ConnectObjects(projectContainer->bridgeReturn, projectContainer->bridgeOut,false);

    projectContainer->listenProgramChanges=false;

    if(hostContainer)
        hostContainer->childContainer=projectContainer;
    if(groupContainer)
        projectContainer->childContainer=groupContainer;

    projectContainer->SetLoadingMode(false);
    projectContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();
}

void MainHost::SetupProgramContainer()
{
    if(!programContainer.isNull()) {
        mainContainer->ParkObject( programContainer );
        programContainer.clear();
        UpdateSolver(true);
        if(mainWindow)
            mainWindow->mySceneView->viewProgram->ClearViewPrograms();
    }

    MetaData info(MetaType::container, FixedObjIds::programContainer);
    info.SetMeta<QString>(metaT::ObjName, "programContainer");

    programContainer = objFactory->NewObject(info).staticCast<Connectables::Container>();
    if(programContainer.isNull())
        return;

    programContainer->SetLoadingMode(true);

    programContainer->SetOptimizerFlag(true);
    programContainer->LoadProgram(0);
    mainContainer->AddObject(programContainer);

    {
        //bridge in
        MetaData info(MetaType::bridge, FixedObjIds::programContainerIn);
        info.SetMeta<QString>(metaT::ObjName, "bridgeIn");
        info.SetMeta(metaT::Direction, Directions::Input);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        programContainer->AddObject( bridge );

        programContainer->bridgeIn = bridge;
    }

    {
        //bridge out
        MetaData info(MetaType::bridge, FixedObjIds::programContainerOut);
        info.SetMeta<QString>(metaT::ObjName, "bridgeOut");
        info.SetMeta(metaT::Direction, Directions::Output);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        programContainer->AddObject( bridge );

        programContainer->bridgeOut = bridge;
    }

    //connect with projectContainer
    if(!projectContainer.isNull()) {
        mainContainer->ConnectObjects(projectContainer->bridgeSend, programContainer->bridgeIn,true);
        mainContainer->ConnectObjects(programContainer->bridgeOut, projectContainer->bridgeReturn,true);
    }


    {
        //bridge send
        MetaData info(MetaType::bridge, FixedObjIds::programContainerSend);
        info.SetMeta<QString>(metaT::ObjName, "bridgeSend");
        info.SetMeta(metaT::Direction, Directions::Send);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        programContainer->AddObject( bridge );

        programContainer->bridgeSend = bridge;
    }

    {
        //bridge return
        MetaData info(MetaType::bridge, FixedObjIds::programContainerReturn);
        info.SetMeta<QString>(metaT::ObjName, "bridgeReturn");
        info.SetMeta(metaT::Direction, Directions::Return);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        programContainer->AddObject( bridge );

        programContainer->bridgeReturn = bridge;
    }

    //connect with groupContainer
    if(!groupContainer.isNull()) {
        mainContainer->ConnectObjects(programContainer->bridgeSend, groupContainer->bridgeIn,true);
        mainContainer->ConnectObjects(groupContainer->bridgeOut, programContainer->bridgeReturn,true);
    }

    connect(programsModel, SIGNAL(ProgChanged(QModelIndex)),
            programContainer.data(), SLOT(SetProgram(QModelIndex)));
    connect(programsModel, SIGNAL(ProgDelete(int)),
            programContainer.data(), SLOT(RemoveProgram(int)));

    emit programParkingModelChanged(&programContainer->parkModel);

    if(groupContainer) {
        groupContainer->childContainer=programContainer;
        programContainer->parentContainer=groupContainer;
    }

    programContainer->SetLoadingMode(false);
    programContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();
}

void MainHost::SetupGroupContainer()
{
    if(!groupContainer.isNull()) {
        mainContainer->ParkObject( groupContainer );
        groupContainer.clear();
        UpdateSolver(true);
        if(mainWindow)
            mainWindow->mySceneView->viewGroup->ClearViewPrograms();
    }

    MetaData info(MetaType::container,FixedObjIds::groupContainer);
    info.SetMeta<QString>(metaT::ObjName, "groupContainer");

    groupContainer = objFactory->NewObject(info).staticCast<Connectables::Container>();
    if(groupContainer.isNull())
        return;

    groupContainer->SetLoadingMode(true);

    groupContainer->LoadProgram(0);
    mainContainer->AddObject(groupContainer);

    {
        //bridge in
        MetaData info(MetaType::bridge, FixedObjIds::groupContainerIn);
        info.SetMeta<QString>(metaT::ObjName, "bridgeIn");
        info.SetMeta(metaT::Direction, Directions::Input);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        groupContainer->AddObject( bridge );

        groupContainer->bridgeIn = bridge;
    }

    {
        //bridge out
        MetaData info(MetaType::bridge, FixedObjIds::groupContainerOut);
        info.SetMeta<QString>(metaT::ObjName, "bridgeOut");
        info.SetMeta(metaT::Direction, Directions::Output);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        groupContainer->AddObject( bridge );

        groupContainer->bridgeOut = bridge;
    }

    //connect with programContainer
    if(!programContainer.isNull()) {
        mainContainer->ConnectObjects(programContainer->bridgeSend, groupContainer->bridgeIn,true);
        mainContainer->ConnectObjects(groupContainer->bridgeOut, programContainer->bridgeReturn,true);
    }

    {
        //bridge send
        MetaData info(MetaType::bridge, FixedObjIds::groupContainerSend);
        info.SetMeta<QString>(metaT::ObjName, "bridgeSend");
        info.SetMeta(metaT::Direction, Directions::Send);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        groupContainer->AddObject( bridge );

        groupContainer->bridgeSend = bridge;
    }

    {
        //bridge return
        MetaData info(MetaType::bridge, FixedObjIds::groupContainerReturn);
        info.SetMeta<QString>(metaT::ObjName, "bridgeReturn");
        info.SetMeta(metaT::Direction, Directions::Return);

        QSharedPointer<Connectables::Object> bridge = objFactory->NewObject(info);
        groupContainer->AddObject( bridge );

        groupContainer->bridgeReturn = bridge;
    }

    //connect with hostContainer
    if(!hostContainer.isNull()) {
        mainContainer->ConnectObjects(groupContainer->bridgeSend, hostContainer->bridgeIn,true);
        mainContainer->ConnectObjects(hostContainer->bridgeOut, groupContainer->bridgeReturn,true);
    }

    connect(programsModel, SIGNAL(GroupChanged(QModelIndex)),
            groupContainer.data(), SLOT(SetProgram(QModelIndex)));
    connect(programsModel, SIGNAL(GroupDelete(int)),
            groupContainer.data(), SLOT(RemoveProgram(int)));

    emit groupParkingModelChanged(&groupContainer->parkModel);

    if(projectContainer)
        projectContainer->childContainer=groupContainer;
    if(programContainer) {
        groupContainer->childContainer=programContainer;
        programContainer->parentContainer=groupContainer;
    }

    groupContainer->SetLoadingMode(false);
    groupContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();
}

bool MainHost::EnableSolverUpdate(bool enable)
{
    mutexSolver.lock();
    bool ret = solverUpdateEnabled;
    solverUpdateEnabled = enable;
    mutexSolver.unlock();
    return ret;
}

void MainHost::UpdateSolver(bool forceUpdate)
{
    if(!renderer)
        return;

    mutexSolver.lock();

        //update not forced, not needed or disabled : return
        if( (!solverUpdateEnabled || !solverNeedAnUpdate) && !forceUpdate) {
            mutexSolver.unlock();
            return;
        }

        //disable other solver updates
        bool solverWasEnabled=solverUpdateEnabled;
        solverUpdateEnabled = false;

        //allow others to ask for a new update while we're updating
        solverNeedAnUpdate = false;

    mutexSolver.unlock();

    //if forced : lock rendering
    if(forceUpdate) {
        mutexRender.lock();
    } else {
        //not forced : do it later if we can't do it now
        if(!mutexRender.tryLock()) {
            //can't lock, ask for a ne update
            SetSolverUpdateNeeded();
            EnableSolverUpdate(solverWasEnabled);
            return;
        }
    }

    //update the solver
    mutexListCables->lock();
        solver->Resolve(workingListOfCables, renderer);
    mutexListCables->unlock();

    mutexRender.unlock();
    EnableSolverUpdate(solverWasEnabled);
}

void MainHost::ChangeNbThreads(int nbThreads)
{
    if(!renderer)
        return;

    if(nbThreads<=0) {
#ifdef _WIN32
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        nbThreads = info.dwNumberOfProcessors;
#else
        nbThreads = 1;
#endif
    }

    mutexRender.lock();
    renderer->SetNbThreads(nbThreads);
    mutexRender.unlock();
    SetSolverUpdateNeeded();

}

void MainHost::SendMsg(const MetaData &senderPin,const PinMessage::Enum msgType,void *data)
{
    QMutexLocker lock(mutexListCables);

    mapCables::const_iterator i = workingListOfCables.constFind(senderPin);
    while (i != workingListOfCables.constEnd()  && i.key() == senderPin) {
        Connectables::Pin *pin = objFactory->GetPin( i.value() );
        if(!pin) {
            LOG("unknown pin");
            return;
        }

        pin->ReceiveMsg(msgType,data);
        ++i;
    }
}

void MainHost::Render(unsigned long samples)
{
    if(samples==0)
        samples=bufferSize;

#ifdef VSTSDK
    CheckTempo();
#endif

    if(!renderer)
        return;

    mutexRender.lock();

    if(mainContainer)
        mainContainer->NewRenderLoop();
    if(hostContainer)
        hostContainer->NewRenderLoop();
    if(projectContainer)
        projectContainer->NewRenderLoop();
    if(groupContainer)
        groupContainer->NewRenderLoop();
    if(programContainer)
        programContainer->NewRenderLoop();

    renderer->StartRender();
    mutexRender.unlock();

    if(solverNeedAnUpdate && solverUpdateEnabled)
        emit SolverToUpdate();
}

void MainHost::OnCableAdded(Connectables::Cable *cab)
{
    mutexListCables->lock();
    workingListOfCables.insert(cab->GetInfoOut(),cab->GetInfoIn());
    mutexListCables->unlock();
}

void MainHost::OnCableRemoved(Connectables::Cable *cab)
{
    mutexListCables->lock();
    workingListOfCables.remove(cab->GetInfoOut(),cab->GetInfoIn());
    mutexListCables->unlock();
}

#ifdef VSTSDK
void MainHost::SetTimeInfo(const VstTimeInfo *info)
{
    vstHost->SetTimeInfo(info);
//    CheckTempo();
}

void MainHost::SetTempo(int tempo, int sign1, int sign2)
{
    vstHost->SetTempo(tempo,sign1,sign2);
//    CheckTempo();

}

void MainHost::CheckTempo()
{
    int tempo=0;
    int sign1=0;
    int sign2=0;

    vstHost->GetTempo(tempo,sign1,sign2);
    if(tempo!=currentTempo
       || sign1!=currentTimeSig1
       || sign2!=currentTimeSig2) {

        currentTempo=tempo;
        currentTimeSig1=sign1;
        currentTimeSig2=sign2;
        emit TempoChanged(currentTempo,currentTimeSig1,currentTimeSig2);
    }
}

void MainHost::GetTempo(int &tempo, int &sign1, int &sign2)
{
    vstHost->GetTempo(tempo,sign1,sign2);
}
#endif

void MainHost::SetSetting(QString name, QVariant value)
{
    settings.setValue(settingsGroup + name,value);
}

QVariant MainHost::GetSetting(QString name, QVariant defaultVal)
{
    return settings.value(settingsGroup + name,defaultVal);
}

bool MainHost::SettingDefined(QString name)
{
    return settings.contains(settingsGroup + name);
}

void MainHost::LoadFile(const QString &filename)
{
    QFileInfo info(filename);
    if ( info.suffix()==SETUP_FILE_EXTENSION ) {
        LoadSetupFile(filename);
    }
    if ( info.suffix()==PROJECT_FILE_EXTENSION ) {
        LoadProjectFile(filename);
    }
}

void MainHost::LoadSetupFile(const QString &filename)
{
    if(!programsModel->userWantsToUnloadSetup())
        return;

    QString name = filename;

    if(name.isEmpty()) {
        QString lastDir = GetSetting("lastSetupDir").toString();
        name = QFileDialog::getOpenFileName(mainWindow, tr("Open a Setup file"), lastDir, tr("Setup Files (*.%1)").arg(SETUP_FILE_EXTENSION));
    }

    if(name.isEmpty())
        return;

    undoStack->clear();

    if(ProjectFile::LoadFromFile(this,name)) {
        ConfigDialog::AddRecentSetupFile(name,this);
        currentSetupFile = name;
    } else {
        ConfigDialog::RemoveRecentSetupFile(name,this);
        ClearSetup();
    }
    emit currentFileChanged();
}

void MainHost::LoadProjectFile(const QString &filename)
{
    if(!programsModel->userWantsToUnloadProject())
        return;

    QString name = filename;

    if(name.isEmpty()) {
        QString lastDir = GetSetting("lastProjectDir").toString();
        name = QFileDialog::getOpenFileName(mainWindow, tr("Open a Project file"), lastDir, tr("Project Files (*.%1)").arg(PROJECT_FILE_EXTENSION));
    }

    if(name.isEmpty())
        return;

    undoStack->clear();

    if(ProjectFile::LoadFromFile(this,name)) {
        ConfigDialog::AddRecentProjectFile(name,this);
        currentProjectFile = name;
    } else {
        ConfigDialog::RemoveRecentProjectFile(name,this);
        ClearProject();
    }
    emit currentFileChanged();
}

void MainHost::ReloadProject()
{
    if(currentProjectFile.isEmpty())
        return;

    undoStack->clear();

    ProjectFile::LoadFromFile(this,currentProjectFile);
}

void MainHost::ReloadSetup()
{
    if(currentSetupFile.isEmpty())
        return;

    undoStack->clear();

    ConfigDialog::AddRecentSetupFile(currentSetupFile,this);
}

void MainHost::ClearSetup()
{
    if(!programsModel->userWantsToUnloadSetup())
        return;

    undoStack->clear();

    EnableSolverUpdate(false);
    SetupHostContainer();
    EnableSolverUpdate(true);
    if(mainWindow)
        mainWindow->viewConfig->LoadFromRegistry();

    ConfigDialog::AddRecentSetupFile("",this);
    currentSetupFile = "";
    emit currentFileChanged();
}

void MainHost::ClearProject()
{
    if(!programsModel->userWantsToUnloadProject())
        return;

    undoStack->clear();

    EnableSolverUpdate(false);
    SetupProjectContainer();
    SetupProgramContainer();
    SetupGroupContainer();
    EnableSolverUpdate(true);

    programsModel->BuildDefaultModel();

    ConfigDialog::AddRecentProjectFile("",this);
    currentProjectFile = "";
    emit currentFileChanged();
}

void MainHost::SaveSetupFile(bool saveAs)
{
    QString filename;

    if(currentSetupFile.isEmpty() || saveAs) {
        QString lastDir = GetSetting("lastSetupDir").toString();
        filename = QFileDialog::getSaveFileName(mainWindow, tr("Save Setup"), lastDir, tr("Setup Files (*.%1)").arg(SETUP_FILE_EXTENSION));

        if(filename.isEmpty())
            return;

        if(!filename.endsWith(SETUP_FILE_EXTENSION, Qt::CaseInsensitive)) {
            filename += ".";
            filename += SETUP_FILE_EXTENSION;
        }
    } else {
        filename = currentSetupFile;
    }

    if(ProjectFile::SaveToSetupFile(this,filename)) {
        SetSetting("lastSetupDir",QFileInfo(filename).absolutePath());
        ConfigDialog::AddRecentSetupFile(filename,this);
        currentSetupFile = filename;
        emit currentFileChanged();
    }
}

void MainHost::SaveProjectFile(bool saveAs)
{
    QString filename;

    if(currentProjectFile.isEmpty() || saveAs) {
        QString lastDir = GetSetting("lastProjectDir").toString();
        filename = QFileDialog::getSaveFileName(mainWindow, tr("Save Project"), lastDir, tr("Project Files (*.%1)").arg(PROJECT_FILE_EXTENSION));

        if(filename.isEmpty())
            return;

        if(!filename.endsWith(PROJECT_FILE_EXTENSION, Qt::CaseInsensitive)) {
            filename += ".";
            filename += PROJECT_FILE_EXTENSION;
        }
    } else {
        filename = currentProjectFile;
    }

    if(ProjectFile::SaveToProjectFile(this,filename)) {
        SetSetting("lastProjectDir",QFileInfo(filename).absolutePath());
        ConfigDialog::AddRecentProjectFile(filename,this);
        currentProjectFile = filename;
        emit currentFileChanged();
    }
}
