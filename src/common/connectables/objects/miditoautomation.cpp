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

#include "miditoautomation.h"
#include "connectables/pins/midipinin.h"
#include "mainhost.h"
#include "commands/comaddpin.h"
#include "commands/comremovepin.h"
#include "connectables/mididefines.h"

using namespace Connectables;

MidiToAutomation::MidiToAutomation(MainHost *myHost, MetaData &info) :
        Object(myHost,info)
{
    SetName("MidiToAutomation");

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
        ParameterPin *pin = static_cast<ParameterPin*>(listParameterPinOut->GetPin(i.key()));
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
                    MetaPin info(listParameterPinOut->getMetaForPin(ctrl));
                    emit UndoStackPush( new ComRemovePin(myHost, info) );
                }
                break;
            case LearningMode::learn :
                if(!listParameterPinOut->listPins.contains(ctrl)) {
                    MetaPin info(listParameterPinOut->getMetaForPin(ctrl));
                    info.SetMeta(metaT::Removable, true);
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

Pin* MidiToAutomation::CreatePin(MetaPin &info)
{
    Pin *newPin = Object::CreatePin(info);
    if(newPin)
        return newPin;

    int pinnumber = info.GetMetaData<int>(metaT::PinNumber);

    switch(info.GetMetaData<int>(metaT::Direction)) {
        case Directions::Input : {
            if(pinnumber == FixedPinNumber::learningMode) {
                info.SetMeta(metaT::ObjName,tr("Learn"));
                ParameterPin *newPin = new ParameterPin(this,info,"off",&listIsLearning);
                newPin->SetLimitsEnabled(false);
                return newPin;
            }
            break;
        }

        case Directions::Output : {

            if(pinnumber<128) {
                info.SetMeta(metaT::ObjName, tr("CC%1").arg(info.GetMetaData<int>(metaT::PinNumber)) );
                info.SetMeta(metaT::Removable,true);

            } else if(pinnumber>=para_notes) {
                info.SetMeta(metaT::ObjName, tr("note%1").arg(info.GetMetaData<int>(metaT::PinNumber)) );
                info.SetMeta(metaT::Removable,true);

            } else switch(pinnumber) {
                case para_prog:
                    info.SetMeta(metaT::ObjName, tr("prog") );
                    break;
                case para_velocity:
                    info.SetMeta(metaT::ObjName, tr("vel") );
                    break;
                case para_notepitch:
                    info.SetMeta(metaT::ObjName, tr("note") );
                    break;
                case para_pitchbend:
                    info.SetMeta(metaT::ObjName, tr("p.bend") );
                    break;
                case para_chanpress:
                    info.SetMeta(metaT::ObjName, tr("pressr") );
                    break;
                case para_aftertouch:
                    info.SetMeta(metaT::ObjName, tr("aftr.t") );
                    break;
                default :
                    LOG("pin not created");
                    return 0;
            }

            return new ParameterPin(this,info,0,&listValues);
        }
    }
    return 0;
}
