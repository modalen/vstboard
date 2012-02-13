#include "vst3plugin.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "mainhost.h"
#include "mainwindow.h"
#include "views/vstpluginwindow.h"

//#include "public.sdk/source/common/pluginview.h"
#include "commands/comaddpin.h"
#include "commands/comremovepin.h"

namespace Steinberg {
    FUnknown* gStandardPluginContext = 0;
}

using namespace Connectables;
using namespace Steinberg;

extern "C"
{
    typedef bool (PLUGIN_API *InitModuleProc) ();
    typedef bool (PLUGIN_API *ExitModuleProc) ();
}

Vst3Plugin::Vst3Plugin(MainHost *host, int index, const ObjectInfo &info) :
    Object(host,index,info),
    pluginLib(0),
    factory(0),
    processorComponent(0),
    editController(0),
    audioEffect(0),
    hasEditor(false),
    editorWnd(0),
    pView(0)
{
    for(int i=0;i<128;i++) {
        listValues << i;
    }
    listBypass << "On" << "Bypass" << "Mute";

    listParameterPinIn->AddPin(FixedPinNumber::vstProgNumber);
    listParameterPinIn->AddPin(FixedPinNumber::bypass);

}

Vst3Plugin::~Vst3Plugin()
{
    Close();
}

bool Vst3Plugin::Open()
{
    pluginLib = new QLibrary(objInfo.filename, this);
    if(!pluginLib->load()) {
        Unload();
        return false;
    }

    InitModuleProc initProc = (InitModuleProc)pluginLib->resolve("InitDll");
    if (initProc)
    {
        if (initProc () == false)
        {
            Unload();
            return false;
        }
    }
    GetFactoryProc entryProc = (GetFactoryProc)pluginLib->resolve("GetPluginFactory");

    if(!entryProc) {
        Unload();
        return false;
    }

    factory = entryProc();

//    PFactoryInfo factoryInfo ;
//    factory->getFactoryInfo (&factoryInfo);
//    LOG("Factory Info:\n\tvendor = " << factoryInfo.vendor << "\n\turl = " << factoryInfo.url << "\n\temail = " << factoryInfo.email)
    for (qint32 i = 0; i < factory->countClasses(); i++) {
        PClassInfo classInfo;
        factory->getClassInfo(i, &classInfo);

        char8 cidString[50];
        FUID(classInfo.cid).toRegistryString (cidString);
        QString cidStr(cidString);
        LOG("  Class Info " << i << ":\n\tname = " << classInfo.name << "\n\tcategory = " << classInfo.category << "\n\tcid = " << cidStr);

        bool res=false;

        if (strcmp(kVstAudioEffectClass, classInfo.category)==0)
        {
            tresult result = factory->createInstance (classInfo.cid, Vst::IComponent::iid, (void**)&processorComponent);
            if (!processorComponent || (result != kResultOk)) {
                errorMessage = tr("Processor not created");
                return true;
            }

            res = (processorComponent->initialize(myHost->vst3Host) == kResultOk);

            if (processorComponent->queryInterface (Vst::IEditController::iid, (void**)&editController) != kResultTrue) {
                FUID controllerCID;
                if (processorComponent->getControllerClassId (controllerCID) == kResultTrue && controllerCID.isValid ()) {
                    result = factory->createInstance (controllerCID, Vst::IEditController::iid, (void**)&editController);
                    if (editController && (result == kResultOk)) {
                        res = (editController->initialize (myHost->vst3Host) == kResultOk);
                    }
                }
            }

            if(!res) {
                errorMessage = tr("not initialized");
                return true;
            }

            tresult check = processorComponent->queryInterface (Vst::IAudioProcessor::iid, (void**)&audioEffect);
            if (check == kResultTrue) {

                processData.numSamples = myHost->GetBufferSize();
                if(doublePrecision)
                    processData.symbolicSampleSize = Vst::kSample64;
                else
                    processData.symbolicSampleSize = Vst::kSample32;


                Vst::ProcessSetup processSetup;
                memset (&processSetup, 0, sizeof (Vst::ProcessSetup));
                processSetup.processMode = Vst::kRealtime;
                if(doublePrecision)
                    processSetup.symbolicSampleSize = Vst::kSample64;
                else
                    processSetup.symbolicSampleSize = Vst::kSample32;
                processSetup.maxSamplesPerBlock = myHost->GetBufferSize();
                processSetup.sampleRate = myHost->GetSampleRate();

                processData.prepare (*processorComponent, myHost->GetBufferSize(), processSetup.symbolicSampleSize);

                if(audioEffect->setupProcessing(processSetup)!= kResultOk) {
                    errorMessage = tr("process setup not accepted");
                    return true;
                }
            }

        }
    }

    closed=false;

    qint32 cpt=0;
    qint32 numBusIn = processorComponent->getBusCount(Vst::kAudio, Vst::kInput);
    for (qint32 i = 0; i < numBusIn; i++) {
        Vst::BusInfo busInfo = {0};
        if(processorComponent->getBusInfo(Vst::kAudio, Vst::kInput, i, busInfo) == kResultTrue) {
            for(qint32 j=0; j<busInfo.channelCount; j++) {
                Pin *p = listAudioPinIn->AddPin(cpt++);
                p->setObjectName( QString::fromUtf16(busInfo.name) );
                static_cast<AudioPin*>(p)->GetBuffer()->SetBufferPointer(processData.inputs[i].channelBuffers32[j]);
            }
        }
    }

    cpt=0;
    qint32 numBusOut = processorComponent->getBusCount(Vst::kAudio, Vst::kOutput);
    for (qint32 i = 0; i < numBusOut; i++) {
        Vst::BusInfo busInfo = {0};
        if(processorComponent->getBusInfo(Vst::kAudio, Vst::kOutput, i, busInfo) == kResultTrue) {
            for(qint32 j=0; j<busInfo.channelCount; j++) {
                Pin *p = listAudioPinOut->AddPin(cpt++);
                p->setObjectName( QString::fromUtf16(busInfo.name) );
                static_cast<AudioPin*>(p)->GetBuffer()->SetBufferPointer(processData.outputs[i].channelBuffers32[j]);
            }
        }
    }

    cpt=0;
    qint32 numBusEIn = processorComponent->getBusCount(Vst::kEvent, Vst::kInput);
    for (qint32 i = 0; i < numBusEIn; i++) {
        Vst::BusInfo busInfo = {0};
        if(processorComponent->getBusInfo(Vst::kEvent, Vst::kInput, i, busInfo) == kResultTrue) {
            for(qint32 j=0; j<busInfo.channelCount; j++) {
                Pin *p = listMidiPinIn->AddPin(cpt++);
                p->setObjectName( QString::fromUtf16(busInfo.name) );
            }
        }
    }

    cpt=0;
    qint32 numBusEOut = processorComponent->getBusCount(Vst::kEvent, Vst::kOutput);
    for (qint32 i = 0; i < numBusEOut; i++) {
        Vst::BusInfo busInfo = {0};
        if(processorComponent->getBusInfo(Vst::kEvent, Vst::kOutput, i, busInfo) == kResultTrue) {
            for(qint32 j=0; j<busInfo.channelCount; j++) {
                Pin *p = listMidiPinOut->AddPin(cpt++);
                p->setObjectName( QString::fromUtf16(busInfo.name) );
            }
        }
    }

    if(editController) {
        hasEditor=true;

        qint32 numParameters = editController->getParameterCount ();
        for (qint32 i = 0; i < numParameters; i++) {
            Vst::ParameterInfo paramInfo = {0};
            tresult result = editController->getParameterInfo (i, paramInfo);
            if (result == kResultOk) {
                 if(paramInfo.flags & Vst::ParameterInfo::kIsReadOnly) {
                    listParameterPinOut->AddPin(i);
                } else {
                    listParameterPinIn->AddPin(i);
                }
            }
        }

        // set the host handler
        // the host set its handler to the controller
        if(editController->setComponentHandler(this) != kResultOk) {
            errorMessage = tr("can't set comp. handler");
            return true;
        }

//         connect the 2 components
        Vst::IConnectionPoint* iConnectionPointComponent = 0;
        Vst::IConnectionPoint* iConnectionPointController = 0;
        processorComponent->queryInterface (Vst::IConnectionPoint::iid, (void**)&iConnectionPointComponent);
        editController->queryInterface (Vst::IConnectionPoint::iid, (void**)&iConnectionPointController);
        if (iConnectionPointComponent && iConnectionPointController) {
            iConnectionPointComponent->connect (iConnectionPointController);
            iConnectionPointController->connect (iConnectionPointComponent);
        }

//         synchronize controller to component by using setComponentState
        MemoryStream stream;
        if (processorComponent->getState (&stream)) {
            stream.seek(0,IBStream::kIBSeekSet,0);
            editController->setComponentState (&stream);
        }
    }

    if(myHost->settings->GetSetting("fastEditorsOpenClose",true).toBool()) {
        CreateEditorWindow();
    }



    if(hasEditor) {
        //editor pin
        listEditorVisible << "hide";
        listEditorVisible << "show";
        listParameterPinIn->AddPin(FixedPinNumber::editorVisible);

        //learning pin
        listIsLearning << "off";
        listIsLearning << "learn";
        listIsLearning << "unlearn";
        listParameterPinIn->AddPin(FixedPinNumber::learningMode);
    }


    return true;
}

void Vst3Plugin::CreateEditorWindow()
{
    QMutexLocker l(&objMutex);

    //already done
    if(editorWnd)
        return;

    pView = editController->createView("editor");
    if(!pView) {
        LOG("view not created")
        return;
    }
//    uint32 r1 = pView->addRef();

#if WINDOWS
    if(pView->isPlatformTypeSupported(kPlatformTypeHWND)!=kResultTrue) {
        LOG("platform not supported (HWND)")
        return;
    }

#elif MAC
        #if MAC_CARBON
    if(pView->isPlatformTypeSupported(kPlatformTypeHIView)!=kResultTrue) {
        LOG("platform not supported (HIView)")
        return;
    }
        #endif

        #if MAC_COCOA
    if(pView->isPlatformTypeSupported(kPlatformTypeNSView)!=kResultTrue) {
        LOG("platform not supported (NSView)")
        return;
    }
        #endif
#endif

    editorWnd = new View::VstPluginWindow(myHost->mainWindow);
    editorWnd->SetPlugin(this);

    tresult result;

    result = pView->setFrame(editorWnd);
    if(result!=kResultOk) {
        LOG("frame not set")
    }
    result = pView->attached(editorWnd->GetWinId(),kPlatformString);
    if(result!=kResultOk) {
        LOG("editor not attached")
    }

    ViewRect size;
    if(pView->getSize(&size)!=kResultOk) {
        LOG("can't get size")
//        return;
    }

    editorWnd->SetWindowSize(size.getWidth(),size.getHeight());

    if(pView->onSize(&size)!=kResultOk) {
        LOG("can't set size")
//        return;
    }

//    result = pView->canResize();
//    if(result == kResultTrue) {
//        LOG("can resize")
//        ViewRect rec(0,0,600,600);
//        result = pView->checkSizeConstraint(&rec);
//        if(result == kResultTrue) {
////            editorWnd->SetWindowSize(rec.getWidth(),rec.getHeight());
//            result = pView->onSize(&rec);
//        }
//    }

    connect(this,SIGNAL(HideEditorWindow()),
            editorWnd,SLOT(hide()),
            Qt::QueuedConnection);
    connect(editorWnd, SIGNAL(Hide()),
            this, SLOT(OnEditorClosed()));
    connect(editorWnd,SIGNAL(destroyed()),
            this,SLOT(EditorDestroyed()));
    connect(this,SIGNAL(WindowSizeChange(int,int)),
            editorWnd,SLOT(SetWindowSize(int,int)));

    hasEditor=true;
}

void Vst3Plugin::OnShowEditor()
{
    if(!editorWnd)
        CreateEditorWindow();

    if(editorWnd->isVisible())
        return;

    editorWnd->show();
    editorWnd->LoadAttribs();
}

void Vst3Plugin::OnHideEditor()
{
    if(!editorWnd)
        return;

    editorWnd->SaveAttribs();

    if(myHost->settings->GetSetting("fastEditorsOpenClose",true).toBool()) {
        disconnect(myHost->updateViewTimer,SIGNAL(timeout()),
            this,SLOT(EditIdle()));
        emit HideEditorWindow();
    } else {
        RemoveGui();
        editorWnd->disconnect();
        editorWnd->UnsetPlugin();
        disconnect(editorWnd);
        QTimer::singleShot(0,editorWnd,SLOT(close()));
        editorWnd=0;

    }
}

void Vst3Plugin::OnEditorClosed()
{
    ToggleEditor(false);
}

void Vst3Plugin::EditorDestroyed()
{
    editorWnd=0;
    RemoveGui();
    listParameterPinIn->listPins.value(FixedPinNumber::editorVisible)->SetVisible(false);
}

void Vst3Plugin::RemoveGui()
{
//    objMutex.lock();
    if(!pView)
        return;

    tresult result;
    result = pView->removed();
    if(result!=kResultOk) {
        LOG("view not removed")
    }
    result = pView->setFrame(0);
    if(result!=kResultOk) {
        LOG("frame not removed")
    }
    pView=0;

//    objMutex.unlock();
}

void Vst3Plugin::SetSleep(bool sleeping)
{
    if(closed)
        return;

    Lock();

    if(sleeping) {
        audioEffect->setProcessing(false);
        processorComponent->setActive(false);

    } else {
        foreach(Pin *in, listAudioPinIn->listPins ) {
            AudioPin *audioIn = static_cast<AudioPin*>(in);
            audioIn->GetBuffer()->ResetStackCounter();
        }
        processorComponent->setActive(true);
        audioEffect->setProcessing(true);
    }

    Unlock();

    Object::SetSleep(sleeping);
}

bool Vst3Plugin::Close()
{
    SetSleep(true);
    closed=true;
    Unload();
    return true;
}

void Vst3Plugin::Unload()
{
    QMutexLocker l(&objMutex);

    RemoveGui();


    if(editorWnd) {
        editorWnd->disconnect();
        editorWnd->UnsetPlugin();
        disconnect(editorWnd);
//        QTimer::singleShot(0,editorWnd,SLOT(close()));
        editorWnd->close();
        editorWnd=0;
    }


//    Vst::IConnectionPoint* iConnectionPointComponent = 0;
//    Vst::IConnectionPoint* iConnectionPointController = 0;
//    processorComponent->queryInterface (Vst::IConnectionPoint::iid, (void**)&iConnectionPointComponent);
//    editController->queryInterface (Vst::IConnectionPoint::iid, (void**)&iConnectionPointController);
//    if (iConnectionPointComponent && iConnectionPointController) {
//        iConnectionPointComponent->disconnect(iConnectionPointController);
//        iConnectionPointComponent->release();
//        iConnectionPointController->disconnect (iConnectionPointComponent);
//        iConnectionPointController->release();
//    }

    if(audioEffect) {
        audioEffect->release();
    }

    if(processorComponent) {
        processorComponent->setActive(false);
        processorComponent->terminate();
        processorComponent->release();
        processorComponent=0;
    }

    if(editController) {
        editController->terminate();
        editController->release();
        editController=0;
    }

    if(factory) {
        factory->release();
        factory=0;
    }



    ExitModuleProc exitProc = (ExitModuleProc)pluginLib->resolve("ExitDll");
    if (exitProc) {
        if(!exitProc()) {
            LOG("exitProc error")
        }
    }


    if(pluginLib) {
        if(pluginLib->isLoaded())
            if(!pluginLib->unload()) {
                LOG("can't unload plugin"<< objInfo.filename);
            }
        delete pluginLib;
        pluginLib=0;
    }

    processData.unprepare();

}

void Vst3Plugin::Render()
{
    if(closed || sleep)
        return;

    if(!audioEffect)
        return;

    QMutexLocker lock(&objMutex);

    foreach(Pin *pin, listAudioPinIn->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
    }
    foreach(Pin *pin, listAudioPinOut->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->GetPointerWillBeFilled();
    }

//    qint32 cpt=0;
//    qint32 numBusIn = processorComponent->getBusCount(Vst::kAudio, Vst::kInput);
//    for (qint32 busIndex = 0; busIndex < numBusIn; busIndex++) {
//        Vst::BusInfo busInfo = {0};
//        if(processorComponent->getBusInfo(Vst::kAudio, Vst::kInput, busIndex, busInfo) == kResultTrue) {
//            for(qint32 channelIndex=0; channelIndex<busInfo.channelCount; channelIndex++) {
//                Pin *pin = listAudioPinIn->GetPin(cpt++);
//                float *buf = (float*)static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
//                processData.inputs[busIndex].channelBuffers32[channelIndex] = buf;
//            }
//        }
//    }

    //output buffers
//    cpt=0;
//    qint32 numBusOut = processorComponent->getBusCount(Vst::kAudio, Vst::kOutput);
//    for (qint32 busIndex = 0; busIndex < numBusOut; busIndex++) {
//        Vst::BusInfo busInfo = {0};
//        if(processorComponent->getBusInfo(Vst::kAudio, Vst::kOutput, busIndex, busInfo) == kResultTrue) {
//            for(qint32 channelIndex=0; channelIndex<busInfo.channelCount; channelIndex++) {
//                Pin *pin = listAudioPinOut->GetPin(cpt++);
//                float *buf = (float*)static_cast<AudioPin*>(pin)->GetBuffer()->GetPointerWillBeFilled();
//                processData.outputs[busIndex].channelBuffers32[channelIndex] = buf;
//            }
//        }
//    }

    paramLock.lock();
    Vst::ParameterChanges vstParamChanges;
    QMap<qint32,float>::const_iterator i = listParamChanged.constBegin();
    while(i!=listParamChanged.constEnd()) {
        int32 idx=0;
        Vst::IParamValueQueue* queue = vstParamChanges.addParameterData(i.key(), idx);
        int32 pIdx=0;
        queue->addPoint(0, i.value(), pIdx);
        ++i;
    }
    listParamChanged.clear();
    paramLock.unlock();
    processData.inputParameterChanges = &vstParamChanges;

//    audioEffect->setProcessing (true);
    tresult result = audioEffect->process (processData);
    if (result != kResultOk) {
        LOG("error while processing")
    }
//    audioEffect->setProcessing (false);

//    for(int32 i=0; i<vstParamChanges.getParameterCount(); i++) {
//        delete vstParamChanges.getParameterData(i);
//    }
//    vstParamChanges.clearQueue();
//    listParamQueue.clear();

    //send result
    //=========================
    foreach(Pin* pin,listAudioPinOut->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
        static_cast<AudioPin*>(pin)->SendAudioBuffer();
    }
}

void Vst3Plugin::OnParameterChanged(ConnectionInfo pinInfo, float value)
{
    Object::OnParameterChanged(pinInfo,value);

    if(closed)
        return;

    if(pinInfo.direction == PinDirection::Input) {
        if(pinInfo.pinNumber==FixedPinNumber::vstProgNumber) {
            return;
        }
        if(pinInfo.pinNumber==FixedPinNumber::bypass) {
            return;
        }

        //send value to the gui
        editController->setParamNormalized(pinInfo.pinNumber, value);
        //send value to the processor
        paramLock.lock();
        if(pinInfo.pinNumber<FIXED_PIN_STARTINDEX)
            listParamChanged.insert(pinInfo.pinNumber,value);
        paramLock.unlock();


    }
}

Pin* Vst3Plugin::CreatePin(const ConnectionInfo &info)
{
    if(info.type == PinType::Audio) {
        return new AudioPin(this,info.direction,info.pinNumber,myHost->GetBufferSize(),doublePrecision,true);
    }

    Pin *newPin = Object::CreatePin(info);
    if(newPin)
        return newPin;

    if(info.type == PinType::Parameter) {
        switch(info.pinNumber) {
            case FixedPinNumber::vstProgNumber :
                return new ParameterPinIn(this,info.pinNumber,0,&listValues,"prog");
            case FixedPinNumber::editorVisible :
                if(!hasEditor)
                    return 0;
                return new ParameterPinIn(this,info.pinNumber,"hide",&listEditorVisible,tr("Editor"));
            case FixedPinNumber::learningMode :
                if(!hasEditor)
                    return 0;
                return new ParameterPinIn(this,info.pinNumber,"off",&listIsLearning,tr("Learn"));
            case FixedPinNumber::bypass :
                return new ParameterPinIn(this,info.pinNumber,"On",&listBypass);
            default : {
                ParameterPin *pin=0;
                Vst::ParameterInfo paramInfo = {0};
                tresult result = editController->getParameterInfo (info.pinNumber, paramInfo);
                if(result != kResultOk)
                    return 0;

                if(info.direction==PinDirection::Output) {
                    pin = new ParameterPinOut(this,info.pinNumber,(float)paramInfo.defaultNormalizedValue,QString::fromUtf16(paramInfo.title),hasEditor,hasEditor);
                } else {
                    pin = new ParameterPinIn(this,info.pinNumber,(float)paramInfo.defaultNormalizedValue,QString::fromUtf16(paramInfo.title),hasEditor,hasEditor);
                }
                if(!hasEditor || (paramInfo.flags & Vst::ParameterInfo::kIsReadOnly))
                    pin->SetDefaultVisible(true);
                else
                    pin->SetDefaultVisible(false);

                return pin;
            }
        }
    }

    return 0;
}


void Vst3Plugin::UserRemovePin(const ConnectionInfo &info)
{
    if(info.type!=PinType::Parameter)
        return;

    if(info.direction!=PinDirection::Input)
        return;

    if(!info.isRemoveable)
        return;

    if(listParameterPinIn->listPins.contains(info.pinNumber))
        static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(info.pinNumber))->SetVisible(false);
    OnProgramDirty();
}

void Vst3Plugin::UserAddPin(const ConnectionInfo &info)
{
    if(info.type!=PinType::Parameter)
        return;

    if(info.direction!=PinDirection::Input)
        return;

    if(listParameterPinIn->listPins.contains(info.pinNumber))
        static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(info.pinNumber))->SetVisible(true);
    OnProgramDirty();
}

tresult PLUGIN_API Vst3Plugin::queryInterface (const TUID iid, void** obj)
{
    char8 cidString[50];
    FUID(iid).toRegistryString (cidString);
    QString cidStr(cidString);
    LOG(cidStr)

    QUERY_INTERFACE (iid, obj, Vst::IComponentHandler::iid, Vst::IComponentHandler)
//    QUERY_INTERFACE (iid, obj, Vst::IComponentHandler2::iid, Vst::IComponentHandler2)
//    QUERY_INTERFACE (iid, obj, FUnknown::iid, Vst::IComponentHandler)
    *obj = 0;
    return kNoInterface;
}

uint32 PLUGIN_API Vst3Plugin::addRef ()
{
    return 1;
}
uint32 PLUGIN_API Vst3Plugin::release ()
{
    return 0;
}
tresult PLUGIN_API Vst3Plugin::beginEdit (Vst::ParamID id)
{
    return kResultOk;
}
tresult PLUGIN_API Vst3Plugin::performEdit (Vst::ParamID id, Vst::ParamValue valueNormalized)
{
    ParameterPin *pin = 0;
    if(listParameterPinIn->GetPin(id)) {
        pin = static_cast<ParameterPin*>(listParameterPinIn->GetPin(id));
    }
//    if(listParameterPinOut->GetPin(id)) {
//        pin = static_cast<ParameterPin*>(listParameterPinOut->GetPin(id));
//    }
    if(!pin) {
        LOG("parameter not found")
        return kInvalidArgument;
    }

    switch(GetLearningMode()) {
        case LearningMode::unlearn :
            if(pin->GetVisible())
                myHost->undoStack.push( new ComRemovePin(myHost, pin->GetConnectionInfo()) );
            break;

        case LearningMode::learn :
            if(!pin->GetVisible())
                myHost->undoStack.push( new ComAddPin(myHost, pin->GetConnectionInfo()) );

        case LearningMode::off :
            pin->ChangeOutputValue(static_cast<float>(valueNormalized));

    }

    return kResultOk;
}
tresult PLUGIN_API Vst3Plugin::endEdit (Vst::ParamID id)
{
    return kResultOk;
}
tresult PLUGIN_API Vst3Plugin::restartComponent (int32 flags)
{
    return kResultOk;
}

QDataStream & Vst3Plugin::toStream(QDataStream & out) const
{
    Object::toStream(out);

    if(!processorComponent) {
        out << savedState;
        return out;
    }

    MemoryStream state;
    if(processorComponent->getState(&state)!=kResultOk) {
        LOG("error saving state")
        out << savedState;
        return out;
    }

    state.seek(0,IBStream::kIBSeekSet,0);
    int32 len=-1;
    char *buf=new char[1024];
    QByteArray bArray;
    while(len!=0) {
        if(state.read(buf,1024,&len)!=kResultOk)
            len=0;
        bArray.append(buf,len);
    }
    out << bArray;
    return out;
}

bool Vst3Plugin::fromStream(QDataStream & in)
{
    if(!Object::fromStream(in))
        return false;

    in >> savedState;

    if(processorComponent && savedState.size()!=0) {
        MemoryStream state;
        state.write(savedState.data(),savedState.size(),0);
        state.seek(0,IBStream::kIBSeekSet,0);
        processorComponent->setState(&state);
    }
    return true;
}

//tresult PLUGIN_API Vst3Plugin::setDirty (TBool state)
//{

//}

//tresult PLUGIN_API Vst3Plugin::requestOpenEditor (FIDString name)
//{

//}

//tresult PLUGIN_API Vst3Plugin::startGroupEdit ()
//{

//}

//tresult PLUGIN_API Vst3Plugin::finishGroupEdit ()
//{

//}