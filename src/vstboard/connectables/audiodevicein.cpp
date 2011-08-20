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

#include "connectables/audiodevicein.h"
#include "connectables/audiodevice.h"
#include "globals.h"
#include "audiobuffer.h"
#include "mainhosthost.h"
#include "audiodevices.h"

using namespace Connectables;

/*!
  \class Connectables::AudioDeviceIn
  \brief an audio device input. user by AudioDevice
  */

/*!
  \param myHost pointer to the MainHost
  \param index object number
  \param info object description
  */
AudioDeviceIn::AudioDeviceIn(MainHost *myHost, MetaData &info) :
    Object(myHost, info),
    parentDevice(0)
{
    listParameterPinOut->AddPin(0);
}

AudioDeviceIn::~AudioDeviceIn()
{
    Close();
}

bool AudioDeviceIn::Close()
{
    objMutex.lock();
    if(parentDevice) {
        parentDevice->SetObjectInput(0);
        parentDevice=0;
    }
    objMutex.unlock();
    return true;
}

void AudioDeviceIn::Render()
{

    foreach(Pin* pin,listAudioPinOut->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
        static_cast<AudioPin*>(pin)->SendAudioBuffer();
    }

    objMutex.lock();
    if(parentDevice) {
        ParameterPin* pin=static_cast<ParameterPin*>(listParameterPinOut->listPins.value(0));
        if(pin)
            pin->ChangeValue(parentDevice->GetCpuUsage());
    }
    objMutex.unlock();
}

void AudioDeviceIn::SetParentDevice( AudioDevice *device )
{
    objMutex.lock();
    parentDevice=device;
    objMutex.unlock();
}

bool AudioDeviceIn::Open()
{
    QMutexLocker l(&objMutex);

    closed=false;

    //create the audiodevice if needed
    if(!parentDevice) {
        MainHostHost *host=static_cast<MainHostHost*>(myHost);
        parentDevice=host->audioDevices->AddDevice( this );
        if(!parentDevice)
            return true;
    }

    //if no input channels
    if(parentDevice->GetNbInputs()==0) {
        parentDevice=0;
        //should be deleted : return false
        return false;
    }

    listAudioPinOut->ChangeNumberOfPins( parentDevice->GetNbInputs() );

    //device already has a child
    if(!parentDevice->SetObjectInput(this)) {
        parentDevice=0;
        data.SetMeta(metaT::errorMessage, tr("Already in use"));
        return true;
    }

    Object::Open();
    return true;
}

Pin* AudioDeviceIn::CreatePin(MetaData &info)
{
    Pin *newPin = Object::CreatePin(info);

    int pinnumber = data.GetMetaData<int>(metaT::PinNumber);

    if(newPin) {
        if(info.GetMetaData<MediaTypes::Enum>(metaT::Media)==MediaTypes::Audio)
            newPin->SetName(QString("Input %1").arg(info.GetMetaData<int>(metaT::PinNumber)));
        return newPin;
    }



    switch(info.GetMetaData<Directions::Enum>(metaT::Direction)) {
        case Directions::Output :
            if(pinnumber==0) {
                info.SetName(tr("cpu%"));
                ParameterPin *pin = new ParameterPin(this,info,0);
                pin->SetLimitsEnabled(false);
                return pin;
            }
            break;
    }

    return 0;
}

#ifdef CIRCULAR_BUFFER
void AudioDeviceIn::SetBufferFromRingBuffer(QList<CircularBuffer*>listCircularBuffers)
{
    unsigned long hostBuffSize = myHost->GetBufferSize();

    int cpt=0;
    foreach(CircularBuffer *buf, listCircularBuffers) {
        AudioBuffer *pinBuf = listAudioPinOut->GetBuffer(cpt);
        cpt++;
        if(!pinBuf)
            continue;

        if(pinBuf->GetSize() < hostBuffSize) {
//            LOG("resize buffer for" << objectName() );
            pinBuf->SetSize(hostBuffSize);
        }

        if(buf->filledSize >= hostBuffSize) {
            if(pinBuf->GetDoublePrecision())
                buf->Get( (double*)pinBuf->GetPointer(true), hostBuffSize );
            else
                buf->Get( (float*)pinBuf->GetPointer(true), hostBuffSize );
        }
    }
}
#endif
//QStandardItem *AudioDeviceIn::GetFullItem()
//{
//    QStandardItem *modelNode = Object::GetFullItem();
//    modelNode->setData(doublePrecision, UserRoles::isDoublePrecision);
//    return modelNode;
//}
