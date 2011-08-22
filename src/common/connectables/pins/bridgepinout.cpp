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


#include "bridgepinout.h"
#include "audiobuffer.h"
#include "mainhost.h"
using namespace Connectables;

BridgePinOut::BridgePinOut(Object *parent, MetaPin &info)
    :Pin(parent,info),
    valueType(MediaTypes::ND)
{
}

void BridgePinOut::SendMsg(const PinMessage::Enum msgType,void *data)
{
    Pin::SendMsg(msgType,data);

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

float BridgePinOut::GetValue()
{
    if(valueChanged) {
        if(internValue==1.0f) internValue=0.99f;
        else internValue=1.0f;
        SetMeta(metaT::BridgeMedia, valueType);
    }
    return internValue;
}
