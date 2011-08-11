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


#include "bridgepinin.h"
#include "object.h"
#include "audiobuffer.h"
#include "mainhost.h"

using namespace Connectables;

BridgePinIn::BridgePinIn(Object *parent, int number, bool bridge) :
    Pin(parent,MediaTypes::Bridge,Directions::Input,number,bridge),
    valueType(MediaTypes::ND),
    loopCounter(0)
{
    setObjectName(QString("BIn%1").arg(number));
    visible=true;
}

//send message to the corresponding output pin
void BridgePinIn::ReceiveMsg(const PinMessage::Enum msgType,void *data)
{
    if(loopCounter>20)
        return;
    ++loopCounter;

    ConnectionInfo info = connectInfo;
    info.direction=Directions::Output;
    parent->GetPin(info)->SendMsg(msgType,data);

    switch(msgType) {
        case PinMessage::AudioBuffer :
            if(static_cast<AudioBuffer*>(data)->GetCurrentVu() < 0.01)
                return;
            valueType=MediaTypes::Audio;
            break;
        case PinMessage::ParameterValue :
            valueType=MediaTypes::Parameter;
            break;
        case PinMessage::MidiMsg:
            valueType=MediaTypes::Midi;
            break;
        default :
            valueType=MediaTypes::ND;
    }
    valueChanged=true;
}

void BridgePinIn::NewRenderLoop()
{
    loopCounter=0;
}

float BridgePinIn::GetValue()
{
    if(valueChanged) {
        if(value==1.0f) value=0.99f;
        else value=1.0f;
        parent->getHost()->GetModel()->setData(modelIndex, valueType, UserRoles::type);
    }
    return value;
}
