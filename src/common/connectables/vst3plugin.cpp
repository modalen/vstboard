#include "vst3plugin.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "mainhost.h"
#include "mainwindow.h"
#include "views/vstpluginwindow.h"
#include "public.sdk/source/common/memorystream.h"
//#include "public.sdk/source/common/pluginview.h"

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

//        char8 cidString[50];
//        FUID(classInfo.cid).toRegistryString (cidString);
//        QString cidStr(cidString);
//        LOG("  Class Info " << i << ":\n\tname = " << classInfo.name << "\n\tcategory = " << classInfo.category << "\n\tcid = " << cidStr);

        if (strcmp(kVstAudioEffectClass, classInfo.category)==0)
        {
            tresult result;
            result = factory->createInstance (classInfo.cid, Vst::IComponent::iid, (void**)&processorComponent);

            if (!processorComponent || (result != kResultOk)) {
                errorMessage = tr("Processor not created");
                return true;
            }
            result = (processorComponent->initialize(myHost->vst3Host) == kResultOk);

            if (processorComponent->queryInterface (Vst::IEditController::iid, (void**)&editController) != kResultTrue) {
                FUID controllerCID;
                if (processorComponent->getControllerClassId (controllerCID) == kResultTrue && controllerCID.isValid ()) {
                    result = factory->createInstance (controllerCID, Vst::IEditController::iid, (void**)&editController);
                    if (editController && (result == kResultOk)) {
                        result = (editController->initialize (myHost->vst3Host) == kResultOk);
                    }
                }
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

                if(audioEffect->setupProcessing(processSetup) == kResultOk) {
                    if (processorComponent->setActive(true) != kResultTrue) {
                        LOG("not activated")
                    }
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
        qint32 numParameters = editController->getParameterCount ();
        for (qint32 i = 0; i < numParameters; i++) {
            Vst::ParameterInfo paramInfo = {0};
            tresult result = editController->getParameterInfo (i, paramInfo);
            if (result == kResultOk) {
                 if(paramInfo.flags & Vst::ParameterInfo::kIsReadOnly) {
                    listParameterPinOut->AddPin(cpt++);
                } else {
                    listParameterPinIn->AddPin(cpt++);
                }
            }
        }

        // set the host handler
        // the host set its handler to the controller
//        editController->setComponentHandler (myHost->vst3Host);

        // connect the 2 components
        Vst::IConnectionPoint* iConnectionPointComponent = 0;
        Vst::IConnectionPoint* iConnectionPointController = 0;

        processorComponent->queryInterface (Vst::IConnectionPoint::iid, (void**)&iConnectionPointComponent);
        editController->queryInterface (Vst::IConnectionPoint::iid, (void**)&iConnectionPointController);

        if (iConnectionPointComponent && iConnectionPointController) {
            iConnectionPointComponent->connect (iConnectionPointController);
            iConnectionPointController->connect (iConnectionPointComponent);
        }

        // synchronize controller to component by using setComponentState
        MemoryStream stream;
//        stream.setByteOrder (kLittleEndian);
        if (processorComponent->getState (&stream)) {
//            stream.rewind ();
            editController->setComponentState (&stream);
        }
    }

    if(myHost->settings->GetSetting("fastEditorsOpenClose",true).toBool()) {
        CreateEditorWindow();
    }

    hasEditor=(editController?true:false);

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
    editorWnd = new View::VstPluginWindow(myHost->mainWindow);
    pView = (editController->createView("editor"));
    if(!pView)
        return;

    pView->attached(editorWnd->GetWinId(),"");
    ViewRect size;
    pView->getSize(&size);
    editorWnd->SetWindowSize(size.getWidth(),size.getHeight());

    connect(this,SIGNAL(HideEditorWindow()),
            editorWnd,SLOT(hide()),
            Qt::QueuedConnection);
    connect(editorWnd, SIGNAL(Hide()),
            this, SLOT(OnEditorClosed()));
    connect(editorWnd,SIGNAL(destroyed()),
            this,SLOT(EditorDestroyed()));
    connect(this,SIGNAL(WindowSizeChange(int,int)),
            editorWnd,SLOT(SetWindowSize(int,int)));
//    editorWnd->show();

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
        editorWnd->disconnect();
        editorWnd->SetPlugin(0);
        disconnect(editorWnd);
        QTimer::singleShot(0,editorWnd,SLOT(close()));
        editorWnd=0;
//        objMutex.lock();
        pView->removed();
        pView=0;
//        objMutex.unlock();
    }
}

void Vst3Plugin::OnEditorClosed()
{
    ToggleEditor(false);
}

void Vst3Plugin::EditorDestroyed()
{
    editorWnd = 0;
//    MainHost::GetModel()->setData(modelIndex, false, UserRoles::editorVisible);
    listParameterPinIn->listPins.value(FixedPinNumber::editorVisible)->SetVisible(false);
}

bool Vst3Plugin::Close()
{
    Unload();
    return true;
}

void Vst3Plugin::Unload()
{
    if(editorWnd) {
        editorWnd->disconnect();
        editorWnd->SetPlugin(0);
        disconnect(editorWnd);
        QTimer::singleShot(0,editorWnd,SLOT(close()));
        editorWnd=0;
//        objMutex.lock();
        pView->removed();
        pView=0;
//        objMutex.unlock();
    }

    processData.unprepare();

    if(processorComponent) {
        processorComponent->setActive(false);
        processorComponent->terminate();
        processorComponent->release();
        processorComponent=0;
    }

    if(factory) {
        factory->release();
        factory=0;
    }

    ExitModuleProc exitProc = (ExitModuleProc)pluginLib->resolve("ExitDll");
    if (exitProc)
          exitProc();


    if(pluginLib) {
        if(pluginLib->isLoaded())
            if(!pluginLib->unload()) {
                LOG("can't unload plugin"<< objInfo.filename);
            }
        delete pluginLib;
        pluginLib=0;
    }
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

    processData.inputParameterChanges = &vstParamChanges;

    audioEffect->setProcessing (true);
    tresult result = audioEffect->process (processData);
    if (result != kResultOk) {
        LOG("error while processing")
    }
    audioEffect->setProcessing (false);

//    for(int32 i=0; i<vstParamChanges.getParameterCount(); i++) {
//        delete vstParamChanges.getParameterData(i);
//    }
    vstParamChanges.clearQueue();
    listParamQueue.clear();

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

        QMutexLocker l(&objMutex);
        Vst::IParamValueQueue* queue=0;
        if(listParamQueue.contains(pinInfo.pinNumber)) {
            queue = vstParamChanges.getParameterData( listParamQueue[pinInfo.pinNumber] );
        } else {
            int32 idx=0;

            Vst::ParameterInfo paramInfo = {0};
            tresult result = editController->getParameterInfo (pinInfo.pinNumber, paramInfo);
            if(result==kResultOk) {
                queue = vstParamChanges.addParameterData(paramInfo.id, idx);
                listParamQueue.insert(pinInfo.pinNumber,idx);
            }
        }

        if(!queue) {
            LOG("no param queue ?")
            return;
        }

        int32 pIdx=0;
        queue->addPoint(0, value, pIdx);
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
                return new ParameterPinIn(this,info.pinNumber,"show",&listEditorVisible,tr("Editor"));
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
                if(result) {
                    if(info.direction==PinDirection::Output) {
                        pin = new ParameterPinOut(this,info.pinNumber,paramInfo.defaultNormalizedValue,QString::fromUtf16(paramInfo.title),hasEditor,hasEditor);
                    } else {
                        pin = new ParameterPinIn(this,info.pinNumber,paramInfo.defaultNormalizedValue,QString::fromUtf16(paramInfo.title),hasEditor,hasEditor);
                    }
                    pin->SetDefaultVisible(!hasEditor);
                }
                return pin;
            }
        }
    }

    return 0;
}
