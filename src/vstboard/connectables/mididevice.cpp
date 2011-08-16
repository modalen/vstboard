/**************************************************************************
#    Copyright 2010-2011 Rapha�l Fran�ois
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

#include "mainhosthost.h"
#include "connectables/mididevice.h"
#include "globals.h"

using namespace Connectables;

MidiDevice::MidiDevice(MainHost *myHost, MetaInfo &info) :
        Object(myHost, info),
        stream(0),
        queue(0),
        devInfo(0),
        deviceOpened(false)
{
}

MidiDevice::~MidiDevice()
{
    Close();
}

void MidiDevice::Render()
{
    if(!deviceOpened)
        return;

    if(devInfo->input) {
        PmEvent buffer;
        objMutex.lock();

        while (Pm_Dequeue(queue, &buffer) == 1) {
            foreach(Pin *pin,listMidiPinOut->listPins) {
                pin->SendMsg(PinMessage::MidiMsg,(void*)&buffer.message);
            }
        }
        objMutex.unlock();
    }
}

void MidiDevice::MidiMsgFromInput(long msg) {
    if(devInfo->output) {
        objMutex.lock();
        Pm_Enqueue(queue,(void*)&msg);
        objMutex.unlock();
    }
}

bool MidiDevice::OpenStream()
{
    if(deviceOpened)
        return true;

    QMutexLocker l(&objMutex);

    if(!FindDeviceByName()){
        errorMessage = tr("Device not found");
        return false;
    }

    queue = Pm_QueueCreate(QUEUE_SIZE, sizeof(PmEvent));
    if(!queue) {
        LOG("can't create queue");
        return false;
    }

    if(objInfo.Meta(MetaInfos::nbInputs).toInt()>0) {
        PmError err = Pm_OpenInput(&stream, (PmDeviceID)objInfo.Meta(MetaInfos::devId).toInt(), 0, 512, 0, 0);
        if (err!=pmNoError) {
            QString msgTxt;
            if(err==pmHostError) {
                char msg[255];
                Pm_GetHostErrorText(msg,255);
                LOG("openInput host error"<<msg);
                msgTxt=msg;
            } else {
                LOG("openInput error"<<Pm_GetErrorText(err));
                msgTxt=Pm_GetErrorText(err);
            }
            errorMessage=tr("Error while opening midi device %1 %2").arg(objInfo.Name()).arg(msgTxt);
            return false;
        }

        err = Pm_SetFilter(stream, PM_FILT_ACTIVE | PM_FILT_SYSEX | PM_FILT_CLOCK);
        if (err!=pmNoError) {
            QString msgTxt;
            if(err==pmHostError) {
                char msg[20];
                unsigned int len=20;
                Pm_GetHostErrorText(msg,len);
                LOG("setFilter host error"<<msg);
                msgTxt=msg;
            } else {
                msgTxt=Pm_GetErrorText(err);
                LOG("setFilter error"<<Pm_GetErrorText(err));
            }
            errorMessage=tr("Error while opening midi device %1 %2").arg(objInfo.Name()).arg(msgTxt);
            return false;
        }
    }

    if(objInfo.Meta(MetaInfos::nbOutputs).toInt()>0) {
        PmError err = Pm_OpenOutput(&stream, (PmDeviceID)objInfo.Meta(MetaInfos::devId).toInt(), 0, 512, 0, 0, 0);
        if (err!=pmNoError) {
            QString msgTxt;
            if(err==pmHostError) {
                char msg[20];
                unsigned int len=20;
                Pm_GetHostErrorText(msg,len);
                LOG("openInput host error"<<msg);
                msgTxt=msg;
            } else {
                LOG("openInput error"<<Pm_GetErrorText(err));
                msgTxt=Pm_GetErrorText(err);
            }
            errorMessage=tr("Error while opening midi device %1 %2").arg(objInfo.Name()).arg(msgTxt);
            return false;
        }
    }

    deviceOpened=true;
//    SetSleep(false);
    return true;
}

bool MidiDevice::CloseStream()
{
//    if(!deviceOpened)
//        return true;
//    SetSleep(true);

    QMutexLocker l(&objMutex);

    PmError err = pmNoError;

    if(stream) {
        err = Pm_Close(stream);
        if(err!=pmNoError) {
            LOG("error closing midi port");
        }
        stream=0;
    }

    if(queue) {
        err = Pm_QueueDestroy(queue);
        if(err!=pmNoError) {
            LOG("error closing midi queue");
        }
        queue=0;
    }

    deviceOpened=false;

    return true;
}

bool MidiDevice::Close()
{
    MainHostHost *host = static_cast<MainHostHost*>(myHost);
    MidiDevices *devCtrl = host->midiDevices;
    if(devCtrl)
        devCtrl->CloseDevice(this);

    CloseStream();
    return true;
}


bool MidiDevice::FindDeviceByName()
{
    int cptDuplicateNames=0;
    int canBe=-1;
    int deviceNumber=-1;

    for(int i=0;i<Pm_CountDevices();i++) {
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if( !strcmp(info->interf, objInfo.Meta(MetaInfos::apiName).toByteArray())
            && !strcmp(info->name, objInfo.Meta(MetaInfos::devName).toByteArray())
            && info->input == objInfo.Meta(MetaInfos::nbInputs).toInt()
            && info->output == objInfo.Meta(MetaInfos::nbOutputs).toInt()
        ) {
            //can be this one, but the interface number can change form a comp to another
            if(cptDuplicateNames==0)
                canBe=i;

            //we found the same number and the same name
            if(objInfo.Meta(MetaInfos::duplicateNamesCounter).toInt() == cptDuplicateNames) {
                devInfo = info;
                deviceNumber = i;
                break;
            }
            cptDuplicateNames++;
        }
    }

    //didn't found an exact match
    if(deviceNumber==-1) {
        //but we found a device with the same name
        if(canBe!=-1) {
            deviceNumber=canBe;
            devInfo = Pm_GetDeviceInfo(deviceNumber);
        } else {
            errorMessage=tr("Error : device was deleted");
            LOG("device not found");
            return false;
        }
    }

    objInfo.SetMeta(MetaInfos::devId, deviceNumber);
    return true;
}

bool MidiDevice::Open()
{
    errorMessage="";
    closed=false;

    //device not found, create a dummy object
    if(!FindDeviceByName()){
        errorMessage = tr("Device not found");
        return true;
    }

    //error while opening device, delete it now
    if(!OpenStream()) {
        return true;
    }
    listMidiPinOut->ChangeNumberOfPins(devInfo->input);
    listMidiPinIn->ChangeNumberOfPins(devInfo->output);

    Object::Open();

    MidiDevices *devCtrl = static_cast<MainHostHost*>(myHost)->midiDevices;
    if(devCtrl)
        devCtrl->OpenDevice(this);
    return true;
}
