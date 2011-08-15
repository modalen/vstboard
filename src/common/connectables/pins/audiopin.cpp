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

#include "audiopin.h"

#include "pin.h"
#include "connectables/objects/object.h"
#include "globals.h"


using namespace Connectables;

/*!
  \class Connectables::AudioPin
  \brief base class for audio pins
  */

/*!
  Constructor, used by PinsList with the help of Object::CreatePin
  \param parent pointer to the parent Object
  \param direction the PinDirection
  \param number pin number in the list
  \param bufferSize size of the buffer
  \param doublePrecision true if the buffer should be a double precision buffer
  \param externalAllocation true if the audio buffer is not owned by the pin and shouldn't be deleted
  */
AudioPin::AudioPin(Object *parent, MetaInfo &info, unsigned long bufferSize, bool doublePrecision, bool externalAllocation) :
    Pin(parent,info),
    doublePrecision(doublePrecision),
    buffer(0)
{
    buffer = new AudioBuffer(doublePrecision,externalAllocation);
    SetBufferSize(bufferSize);
}

/*!
  Destructor
  */
AudioPin::~AudioPin()
{
    if(buffer)
        delete buffer;
}


void AudioPin::NewRenderLoop()
{
    buffer->ResetStackCounter();
}

/*!
  set the buffer precision
  \param dblp true for double, false for float
  \return true on success
  */
bool AudioPin::SetDoublePrecision(bool dblp)
{
    if(dblp==doublePrecision)
        return true;

    if(doublePrecision) {
        SetMeta(MetaInfos::displayedText, QString(Name()+"=D=") );
//        displayedText=objectName()+"=D=";
    } else {
        SetMeta(MetaInfos::displayedText, QString(Name()+"=S=") );
//        displayedText=objectName()+"=S=";
    }
    valueChanged=true;

    doublePrecision=dblp;
    buffer->SetDoublePrecision(dblp);

//    if(doublePrecision) {
//        if(!bufferD) {
//            LOG("no double precision buffer : can't convert to single precision");
//            return false;
//        }

//        if(!buffer)
//            buffer = new AudioBuffer(bufferD->IsExternallyAllocated());

//        buffer->SetSize(bufferD->GetSize());
//        buffer->ResetStackCounter();
//        buffer->AddToStack(bufferD);
//        delete bufferD;
//        bufferD = 0;
//    } else {
//        if(!buffer) {
//            LOG("no single precision buffer : can't convert to double precision");
//            return false;
//        }

//        if(!bufferD)
//            bufferD = new AudioBufferD(buffer->IsExternallyAllocated());

//        bufferD->SetSize(buffer->GetSize());
//        bufferD->ResetStackCounter();
//        bufferD->AddToStack(buffer);
//        delete buffer;
//        buffer = 0;
//    }


    return true;
}

/*!
  Resize the current buffer
  \param size the new size
  */
void AudioPin::SetBufferSize(unsigned long size)
{
    buffer->SetSize(size);
}

float AudioPin::GetValue()
{
    float newVu=.0f;
    newVu=buffer->GetVu();

    if(newVu != internValue) {
        valueChanged=true;
    }
    return newVu;
}

void AudioPin::ReceiveMsg(const PinMessage::Enum msgType,void *data)
{
    if(msgType==PinMessage::AudioBuffer) {
        AudioBuffer *buf = static_cast<AudioBuffer*>(data);
        buffer->AddToStack(buf);
    }
}

/*!
  Send the current buffer to all connected pins
  */
void AudioPin::SendAudioBuffer()
{
    SendMsg(PinMessage::AudioBuffer,(void*)buffer);
}
