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

#include "miditoautomation.h"
#include "midipinin.h"
#include "../globals.h"
#include "mainhost.h"
#include "commands/comaddpin.h"
#include "commands/comremovepin.h"

using namespace Connectables;

MidiToAutomation::MidiToAutomation(MainHost *myHost,int index) :
        Object(myHost,index, ObjectInfo(NodeType::object, ObjType::MidiToAutomation, index, tr("Midi to Parameter")) )
{
    for(int i=0;i<128;i++) {
        listValues << i;
    }

    listMidiPinIn->ChangeNumberOfPins(1);

    //learning pin
    listIsLearning << "off";
    listIsLearning << "learn";
    listIsLearning << "unlearn";
    listParameterPinIn->AddPin(FixedPinNumber::learningMode);
    listParameterPinOut->AddPin(para_prog);
    listParameterPinOut->AddPin(para_velocity);
    listParameterPinOut->AddPin(para_notepitch);
    listParameterPinOut->AddPin(para_pitchbend);
    listParameterPinOut->AddPin(para_chanpress);
    listParameterPinOut->AddPin(para_aftertouch);
}


void MidiToAutomation::Render()
{
    if(listChanged.isEmpty())
        return;

    QHash<quint16,quint8>::Iterator i = listChanged.begin();
    while(i!=listChanged.end()) {
        ParameterPinOut *pin = static_cast<ParameterPinOut*>(listParameterPinOut->listPins.value(i.key(),0));
        if(pin)
            pin->ChangeValue(i.value());
        ++i;
    }
    listChanged.clear();
}

void MidiToAutomation::MidiMsgFromInput(long msg)
{
    int command = MidiStatus(msg) & MidiConst::codeMask;

    switch(command) {
        case MidiConst::ctrl: {
            ChangeValue(MidiData1(msg),MidiData2(msg));
            break;
        }
        case MidiConst::prog : {
            ChangeValue(para_prog, MidiData1(msg) );
            break;
        }
        case MidiConst::noteOn : {
            ChangeValue(para_velocity, MidiData2(msg) );
            ChangeValue(para_notes+MidiData1(msg), MidiData2(msg) );
            ChangeValue(para_notepitch, MidiData1(msg) );
            break;
        }
        case MidiConst::noteOff : {
            ChangeValue(para_notepitch, MidiData1(msg) );
            ChangeValue(para_notes+MidiData1(msg), MidiData2(msg) );
            break;
        }
        case MidiConst::pitchbend : {
            ChangeValue(para_pitchbend, MidiData2(msg) );
            break;
        }
        case MidiConst::chanpressure : {
            ChangeValue(para_chanpress, MidiData1(msg) );
            break;
        }
        case MidiConst::aftertouch : {
            ChangeValue(para_velocity, MidiData1(msg) );
            ChangeValue(para_aftertouch, MidiData2(msg) );
        }
    }
}

void MidiToAutomation::ChangeValue(int ctrl, int value) {

    if(value>127 || value<0) {
        LOG("midi ctrl 0>127"<<ctrl<<value);
        return;
    }

    if(ctrl<128 || ctrl>=para_notes) {
        switch(GetLearningMode()) {
            case LearningMode::unlearn :
                if(listParameterPinOut->listPins.contains(ctrl)) {
                    emit UndoStackPush( new ComRemovePin(myHost, listParameterPinOut->listPins.value(ctrl)->GetConnectionInfo()) );
                }
                break;
            case LearningMode::learn :
                if(!listParameterPinOut->listPins.contains(ctrl)) {
                    ConnectionInfo info = listParameterPinOut->connInfo;
                    info.pinNumber = ctrl;
                    info.isRemoveable = true;
                    emit UndoStackPush( new ComAddPin(myHost,info) );
                }
            case LearningMode::off :
                listChanged.insert(ctrl,value);
                break;
        }
    } else {
        listChanged.insert(ctrl,value);
    }
}

Pin* MidiToAutomation::CreatePin(const ConnectionInfo &info)
{
    Pin *newPin = Object::CreatePin(info);
    if(newPin)
        return newPin;

    if(info.type!=PinType::Parameter) {
        LOG("wrong PinType"<<info.type);
        return 0;
    }

    switch(info.direction) {
        case PinDirection::Input : {
            if(info.pinNumber == FixedPinNumber::learningMode) {
                ParameterPin *newPin = new ParameterPinIn(this,FixedPinNumber::learningMode,"off",&listIsLearning,tr("Learn"));
                newPin->SetLimitsEnabled(false);
                return newPin;
            }
            break;
        }
        case PinDirection::Output : {
            ParameterPin *pin = 0;

            if(info.pinNumber<128)
                return new ParameterPinOut(this,info.pinNumber,0,&listValues,QString("CC%1").arg(info.pinNumber),false,true);
            if(info.pinNumber>=para_notes)
                return new ParameterPinOut(this,info.pinNumber,0,&listValues,QString("note%1").arg(info.pinNumber),false,true);
            if(info.pinNumber==para_prog)
                return new ParameterPinOut(this,info.pinNumber,0,&listValues,"prog");
            if(info.pinNumber==para_velocity)
                pin = new ParameterPinOut(this,info.pinNumber,0,&listValues,"vel");
            if(info.pinNumber==para_notepitch)
                pin = new ParameterPinOut(this,info.pinNumber,0,&listValues,"note");
            if(info.pinNumber==para_pitchbend)
                pin = new ParameterPinOut(this,info.pinNumber,0,&listValues,"p.bend");
            if(info.pinNumber==para_chanpress)
                pin = new ParameterPinOut(this,info.pinNumber,0,&listValues,"pressr");
            if(info.pinNumber==para_aftertouch)
                pin = new ParameterPinOut(this,info.pinNumber,0,&listValues,"aftr.t");
            if(pin) {
                return pin;
            }

            break;
        }
    }
    return newPin;
}
