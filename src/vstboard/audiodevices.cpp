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

#include "audiodevices.h"
#include "globals.h"
#include "objectinfo.h"
#include "mainhosthost.h"
#include "pa_asio.h"
#include "views/mmeconfigdialog.h"
#include "views/wasapiconfigdialog.h"
#include "connectables/audiodevicein.h"
#include "connectables/audiodeviceout.h"
#include "mainwindow.h"

FakeTimer::FakeTimer(MainHostHost *myHost) :
    QThread(myHost),
    myHost(myHost),
    stop(false)
{
    setObjectName("FakeTimer");
    start(QThread::TimeCriticalPriority);
}

FakeTimer::~FakeTimer()
{
    LOG("stop thread"<<objectName()<<(int)currentThreadId());
    stop=true;
    wait(1000);
}

void FakeTimer::run()
{

    LOG("start thread"<<objectName()<<(int)currentThreadId());
    while(!stop) {
        msleep(FAKE_RENDER_TIMER_MS);
        myHost->Render();
    }
    stop=false;
}

/*!
  \class AudioDevices
  \brief manage the list of audio devices provided by PortAudio
  */

/*!
  \param myHost pointer to the MainHost
  */
AudioDevices::AudioDevices(MainHostHost *myHost) :
    QObject(myHost),
    closing(false),
    model(0),
    countActiveDevices(0),
    myHost(myHost),
    fakeRenderTimer(0)
{
    SET_MUTEX_NAME(mutexDevices,"mutexDevices audiodevices");
    SET_MUTEX_NAME(mutexClosing,"mutexClosing audiodevices");

    fakeRenderTimer = new FakeTimer(myHost);
    Init();
}

/*!
  Destructor, close all the opened devices
  */
AudioDevices::~AudioDevices()
{
    mutexClosing.lock();
    closing=true;
    mutexClosing.unlock();

    mutexDevices.lock();
    foreach(Connectables::AudioDevice *ad, listAudioDevices) {
        ad->SetSleep(true);
    }
    mutexDevices.unlock();

    if(model) {
        PaError err=Pa_Terminate();
        if(err!=paNoError) {
            LOG("Pa_Terminate"<<Pa_GetErrorText( err ));
        }
//        model->deleteLater();
    }

    mutexDevices.lock();
    foreach(Connectables::AudioDevice *dev, listAudioDevices)
        delete dev; //dev->DeleteIfUnused();
    listAudioDevices.clear();
    mutexDevices.unlock();

    if(fakeRenderTimer)
        delete fakeRenderTimer;
}

/*!
  Get the view model of the list
  \return pointer to the model
  */
bool AudioDevices::Init()
{
    mutexClosing.lock();
    closing=true;
    mutexClosing.unlock();

    mutexDevices.lock();
    foreach(Connectables::AudioDevice *ad, listAudioDevices) {
        ad->SetSleep(true);
    }
    mutexDevices.unlock();

    mutexClosing.lock();
    closing=false;
    mutexClosing.unlock();

    if(model) {
        PaError err=Pa_Terminate();
        if(err!=paNoError) {
            LOG("Pa_Terminate"<<Pa_GetErrorText( err ));
        }
        model->invisibleRootItem()->removeRows(0, model->invisibleRootItem()->rowCount());
    }


    PaError paRet =Pa_Initialize();
    if(paRet!=paNoError) {
        myHost->mainWindow->DisplayMessage(QMessageBox::Critical, tr("Unable to initialize audio engine : %1").arg( Pa_GetErrorText(paRet) ));
        return false;
    }
    BuildModel();

    mutexClosing.lock();
    closing=true;
    mutexClosing.unlock();

    mutexDevices.lock();
    foreach(Connectables::AudioDevice *ad, listAudioDevices) {
        ad->SetSleep(false);
    }
    mutexDevices.unlock();

    mutexClosing.lock();
    closing=false;
    mutexClosing.unlock();


    //rebuild all audio in&out objects
    foreach(QSharedPointer<Connectables::Object>obj, myHost->objFactory->GetListObjects()) {
        if(obj.isNull())
            continue;

        if(obj->Meta(MetaInfos::ObjType).toInt() == ObjTypes::AudioInterface) {
            QString errMsg;
            Connectables::AudioDevice *newDevice = AddDevice( obj.data() );
            switch(obj->Meta(MetaInfos::Direction).toInt()) {
                case Directions::Input :
                    obj.staticCast<Connectables::AudioDeviceIn>()->SetParentDevice(newDevice);
                    break;
                case Directions::Output :
                    obj.staticCast<Connectables::AudioDeviceOut>()->SetParentDevice(newDevice);
                    break;
            }

            if(obj->Open()) {
//                obj->UpdateModelNode();
                obj->AddToView();
            } else {
                static_cast<Connectables::Container*>(myHost->objFactory->GetObjectFromId( obj->ContainerId() ).data())->UserParkObject( obj );
            }
            obj->SetMeta(MetaInfos::errorMessage,errMsg);
        }
    }

    return true;
}

/*!
  Create or update the list of devices
  */
void AudioDevices::BuildModel()
{
    if(!model)
        model = new ListAudioInterfacesModel(this);

    QStringList headerLabels;
    headerLabels << "Name";
//    headerLabels << "In";
//    headerLabels << "Out";
    headerLabels << "I/O";
    headerLabels << "InUse";
    model->setHorizontalHeaderLabels(  headerLabels );

    QStandardItem *parentItem = model->invisibleRootItem();

    //APIs
    for (int i = 0; i < Pa_GetHostApiCount(); ++i) {
        const PaHostApiInfo *apiInfo = Pa_GetHostApiInfo(i);
        QStandardItem *apiItem = new QStandardItem(apiInfo->name);
        apiItem->setData( (quint8)apiInfo->type, UserRoles::value );
        apiItem->setDragEnabled(false);
        apiItem->setSelectable(false);
        parentItem->appendRow(apiItem);
        if(apiInfo->type == paASIO)
            AsioIndex = apiItem->index();

        //Devices

        //an api can contain multiple devices with the same name
        QString lastName;
        int cptDuplicateNames=0;

        for (int j=0; j<apiInfo->deviceCount; j++) {
            PaDeviceIndex devIndex = Pa_HostApiDeviceIndexToDeviceIndex(i, j);
            const PaDeviceInfo *devInfo = Pa_GetDeviceInfo( devIndex );

            QString devName(devInfo->name);

            //remove " x64" from device name so we can share files with 32bit version
            devName.remove(QRegExp("( )?x64"));

            //count duplicate names
            if(lastName == devName) {
                cptDuplicateNames++;
            } else {
                cptDuplicateNames=0;
                lastName = devName;
            }

            MetaInfo metaInfo(MetaTypes::object);
            metaInfo.SetName(devName);
            metaInfo.SetMeta(MetaInfos::ObjType, ObjTypes::AudioInterface);
            metaInfo.SetMeta(MetaInfos::devId, devIndex);
            metaInfo.SetMeta(MetaInfos::devName, devName);
            metaInfo.SetMeta(MetaInfos::apiId, apiInfo->type);
            metaInfo.SetMeta(MetaInfos::duplicateNamesCounter, cptDuplicateNames);
            metaInfo.SetMeta(MetaInfos::nbInputs, devInfo->maxInputChannels);
            metaInfo.SetMeta(MetaInfos::nbOutputs, devInfo->maxOutputChannels);

            //separate input and output devices
            if(devInfo->maxInputChannels>0) {
                metaInfo.SetMeta(MetaInfos::Direction, Directions::Input);
                AddDeviceToApiItem(metaInfo,apiItem);
            }
            if(devInfo->maxOutputChannels>0) {
                metaInfo.SetMeta(MetaInfos::Direction, Directions::Output);
                AddDeviceToApiItem(metaInfo,apiItem);
            }
        }
    }
}

void AudioDevices::AddDeviceToApiItem(const MetaInfo &info, QStandardItem *apiItem)
{
    QList<QStandardItem *> listItems;

    QStandardItem *devItem = new QStandardItem( info.Meta(MetaInfos::devName).toString() );
    devItem->setEditable(false);
    devItem->setData(QVariant::fromValue(info), UserRoles::metaInfo);
    devItem->setDragEnabled(true);
    listItems << devItem;

    QStandardItem *IOitem = new QStandardItem();
    if(info.Meta(MetaInfos::Direction).toInt() == Directions::Input)
        IOitem->setText( info.Meta(MetaInfos::nbInputs).toString()+"in" );
    if(info.Meta(MetaInfos::Direction).toInt() == Directions::Output)
        IOitem->setText( info.Meta(MetaInfos::nbOutputs).toString()+"out" );

    IOitem->setEditable(false);
    listItems << IOitem;

//    QStandardItem *outputItem = new QStandardItem();
//    if(info.Meta(MetaInfos::Direction).toInt() == Directions::Output)
//        outputItem->setText( info.Meta(MetaInfos::nbOutputs).toString() );
//    outputItem->setEditable(false);
//    listItems << outputItem;

    QStandardItem *inUseItem = new QStandardItem();
    inUseItem->setCheckable(true);
    inUseItem->setCheckable(false);
    inUseItem->setEditable(false);
    listItems << inUseItem;

    apiItem->appendRow( listItems );
}

/*!
  Called when a device is added or removed from a panel
  \param objInfo object description
  \param opened true if opened, false if closed
  */
void AudioDevices::OnToggleDeviceInUse(PaHostApiIndex apiId, PaDeviceIndex devId, bool inUse, PaTime inLatency, PaTime outLatency, double sampleRate)
{

    //find API item
    QStandardItem *apiItem = 0;
    int nbApi = model->invisibleRootItem()->rowCount();
    int apiCount = 0;
    while(!apiItem && apiCount<nbApi) {
        if(model->item(apiCount,0)->data(UserRoles::value).toInt() == apiId )
            apiItem = model->item(apiCount,0);
        apiCount++;
    }

    if(!apiItem) {
        LOG("API not found"<<apiId);
        return;
    }

    //find device item
    QStandardItem *devItem = 0;
    int nbDev = apiItem->rowCount();
    int devCount = 0;
    while(devCount<nbDev) {
        MetaInfo info = apiItem->child(devCount,0)->data(UserRoles::metaInfo).value<MetaInfo>();
        if(info.Meta(MetaInfos::devId).toInt() == devId) {
            devItem = apiItem->child(devCount,0);

            //change status
            QStandardItem *chk = apiItem->child( devItem->row(), 2);
            if(!chk)
                return;

            if(inUse) {
                if(chk->checkState()!=Qt::Checked) {
                    chk->setCheckState(Qt::Checked);
                    countActiveDevices++;
                }

                int inL = ceil(inLatency*1000);
                int outL = ceil(outLatency*1000);
                devItem->setToolTip( QString("Input latency %1ms\nOutput latency %2ms\nSample rate %3Hz")
                                     .arg(inL).arg(outL).arg(sampleRate) );
            } else {
                if(chk->checkState()==Qt::Checked) {
                    chk->setCheckState(Qt::Unchecked);
                    countActiveDevices--;
                }
                devItem->setToolTip("");
            }
        }
        devCount++;
    }



//    qDebug()<<"countActiveDevices"<<countActiveDevices<<fakeRenderTimer;

    //the renderer is normally launched when all the audio devices are ready,
    //if there is no audio device we have to run a timer
    if(countActiveDevices>0) {
        myHost->SetBufferSize(1);
        if(fakeRenderTimer) {
            delete fakeRenderTimer;
            fakeRenderTimer=0;
        }
    }
    if(countActiveDevices==0) {
        myHost->SetBufferSizeMs(FAKE_RENDER_TIMER_MS);
        if(!fakeRenderTimer && !closing)
            fakeRenderTimer = new FakeTimer(myHost);
    }


}

Connectables::AudioDevice * AudioDevices::AddDevice(Connectables::Object *obj)
{
    PaDeviceInfo PAinfo;

    if(!FindPortAudioDevice(obj, &PAinfo)) {
        obj->SetMeta(MetaInfos::errorMessage,tr("Device not found"));
        return 0;
    }

    mutexDevices.lock();
    Connectables::AudioDevice *dev = listAudioDevices.value(obj->Meta(MetaInfos::devId).toInt(),0);
    mutexDevices.unlock();

    if(!dev) {
        dev = new Connectables::AudioDevice(PAinfo, myHost, obj->info());
        if(!dev->Open()) {
            if(!dev->errorMessage.isEmpty())
                obj->SetMeta(MetaInfos::errorMessage,dev->errorMessage);
            delete dev;
            return 0;
        }
        mutexDevices.lock();
        listAudioDevices.insert(obj->Meta(MetaInfos::devId).toInt(), dev);
        mutexDevices.unlock();
    }
    return dev;
}

void AudioDevices::RemoveDevice(PaDeviceIndex devId)
{
    mutexDevices.lock();
    Connectables::AudioDevice *dev = listAudioDevices.take(devId);
    mutexDevices.unlock();
    if(dev) {
        delete dev;
    }
}

#ifdef CIRCULAR_BUFFER
void AudioDevices::PutPinsBuffersInRingBuffers()
{
    mutexClosing.lock();
    if(closing) {
        mutexClosing.unlock();
        return;
    }

    mutexDevices.lock();
    foreach(Connectables::AudioDevice *dev, listAudioDevices) {
        dev->PinsToRingBuffers();
    }
    mutexDevices.unlock();

    mutexClosing.unlock();
}
#endif
/*!
  Try to find a device in the list returned by PortAudio
  \param[in] objInfo the ObjectInfo we're looking for
  \param[out] devInfo the PaDeviceInfo of the object found
  \return true if found
  */
bool AudioDevices::FindPortAudioDevice(Connectables::Object *obj, PaDeviceInfo *dInfo)
{
    int cptDuplicateNames=0;

    PaDeviceIndex foundSameName=-1;
    PaDeviceIndex foundSameNameId=-1;
    PaDeviceIndex foundSameNamePins=-1;
    PaDeviceIndex foundSameNamePinsId=-1;

    PaHostApiTypeId apiType = (PaHostApiTypeId)obj->Meta(MetaInfos::apiId).toInt();
    PaHostApiIndex apiIndex = Pa_HostApiTypeIdToHostApiIndex( apiType );
    const PaHostApiInfo *apiInfo = Pa_GetHostApiInfo( apiIndex );

    for (int i=0; i<apiInfo->deviceCount; i++) {
        PaDeviceIndex devIndex = Pa_HostApiDeviceIndexToDeviceIndex( apiIndex, i);
        const PaDeviceInfo *info = Pa_GetDeviceInfo( devIndex );

        QString devName(info->name);
        //remove " x64" from device name so we can share files with 32bit version
        devName.remove(QRegExp("( )?x64"));

        if(devName == obj->Meta(MetaInfos::devName).toString()) {

            if(info->maxInputChannels == obj->Meta(MetaInfos::nbInputs).toInt()
            && info->maxOutputChannels == obj->Meta(MetaInfos::nbOutputs).toInt()) {
                if(obj->Meta(MetaInfos::duplicateNamesCounter).toInt() == cptDuplicateNames) {
                    foundSameNamePinsId = devIndex;
                } else {
                    foundSameNamePins = devIndex;
                }
            } else {
                if(obj->Meta(MetaInfos::duplicateNamesCounter).toInt() == cptDuplicateNames) {
                    foundSameNameId = devIndex;
                } else {
                    foundSameName = devIndex;
                }
            }

            cptDuplicateNames++;
        }
    }


    PaDeviceIndex deviceNumber=-1;

    if(foundSameNamePinsId!=-1)
        deviceNumber = foundSameNamePinsId;
    else if(foundSameNameId!=-1)
        deviceNumber = foundSameNameId;
    else if(foundSameNamePins!=-1)
        deviceNumber = foundSameNamePins;
    else if(foundSameName!=-1)
        deviceNumber = foundSameName;
    else {
        LOG("device not found");
        return false;
    }

    if(dInfo) {
        const PaDeviceInfo *i = Pa_GetDeviceInfo(deviceNumber);
        if(!i) {
            LOG("error in GetDeviceInfo");
            return false;
        }
        *dInfo = *i;
    }
    obj->SetMeta(MetaInfos::devId, deviceNumber);
    return true;
}

void AudioDevices::ConfigDevice(const QModelIndex &index)
{
    PaHostApiTypeId apiIndex;
    PaDeviceIndex devId=-1;

    if(index.data(UserRoles::metaInfo).isValid()) {
        MetaInfo info = index.data(UserRoles::metaInfo).value<MetaInfo>();
        devId = (PaDeviceIndex)info.Meta(MetaInfos::devId).toInt();
        apiIndex = (PaHostApiTypeId)info.Meta(MetaInfos::apiId).toInt();
    }

    switch(apiIndex) {
        case paASIO: {
            PaError err;
#if WIN32
            err = PaAsio_ShowControlPanel( devId, (void*)myHost->mainWindow );
#endif
#ifdef __APPLE__
            err = PaAsio_ShowControlPanel( devId, (void*)0 );
#endif

            if( err != paNoError )
                myHost->mainWindow->DisplayMessage(QMessageBox::Warning, Pa_GetErrorText( err ));
            return;
        }

        case paMME: {
            MmeConfigDialog dlg( myHost );
            dlg.exec();
            return;
        }

        case paWASAPI: {
            WasapiConfigDialog dlg( myHost );
            dlg.exec();
            return;
        }

        default:
            break;
    }

    myHost->mainWindow->DisplayMessage(QMessageBox::Information, tr("There's no config dialog for this device"));
}
