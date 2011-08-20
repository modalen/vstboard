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

#include "vstautomation.h"
#include "mainhostvst.h"
#include "vst.h"
#include "commands/comaddpin.h"
#include "commands/comremovepin.h"

using namespace Connectables;

VstAutomation::VstAutomation(MainHost *myHost, MetaData &info) :
        Object(myHost,info )
{
    for(int i=0;i<128;i++) {
        listValues << i;
    }

    //learning pin
    listIsLearning << "off";
    listIsLearning << "learn";
    listIsLearning << "unlearn";
    listParameterPinIn->AddPin(FixedPinNumber::learningMode);

    listParameterPinIn->AddPin(FixedPinNumber::numberOfPins);
    listParameterPinOut->AddPin(FixedPinNumber::vstProgNumber);

    listParameterPinIn->SetNbPins(VST_AUTOMATION_DEFAULT_NB_PINS);
    listParameterPinOut->SetNbPins(VST_AUTOMATION_DEFAULT_NB_PINS);

    connect(static_cast<MainHostVst*>(myHost)->myVstPlugin,SIGNAL(HostChangedProg(int)),
            this,SLOT(OnHostChangedProg(int)));
}

VstAutomation::~VstAutomation()
{
    Close();
}


void VstAutomation::Render()
{
    if(!listChanged.isEmpty()) {
        Vst *vst=static_cast<MainHostVst*>(myHost)->myVstPlugin;
        QHash<int,float>::const_iterator i = listChanged.constBegin();
        while(i!=listChanged.constEnd()) {
            if(listParameterPinOut->listPins.contains(i.key())) {
                static_cast<ParameterPin*>( listParameterPinOut->listPins.value(i.key()) )->ChangeValue(i.value());
            }
            vst->updateParameter(i.key(),i.value());
            ++i;
        }
        listChanged.clear();
    }
}

void VstAutomation::OnHostChangedProg(int prog)
{
    static_cast<ParameterPin*>(listParameterPinOut->listPins.value(FixedPinNumber::vstProgNumber))->ChangeValue( prog, true );
}

void VstAutomation::ValueFromHost(int pinNum, float value)
{
    switch(GetLearningMode()) {
        case LearningMode::unlearn :
            {
                QUndoCommand *com = new QUndoCommand(tr("Remove pin"));
                if(listParameterPinOut->listPins.contains(pinNum)) {
                    new ComRemovePin(myHost, listParameterPinOut->listPins.value(pinNum)->info(),com);
                }
                if(listParameterPinIn->listPins.contains(pinNum)) {
                    new ComRemovePin(myHost, listParameterPinIn->listPins.value(pinNum)->info(),com);
                }
                if(com->childCount())
                    emit UndoStackPush(com);
                else
                    delete com;
            }
            break;
        case LearningMode::learn :
            {
                QUndoCommand *com = new QUndoCommand(tr("Add pin"));
                if(!listParameterPinOut->listPins.contains(pinNum)) {
                    MetaData info(listParameterPinOut->getMetaForPin(pinNum));
                    info.SetMeta(metaT::Removable,true);
                    new ComAddPin(myHost,info,com);
                }
                if(!listParameterPinIn->listPins.contains(pinNum)) {
                    MetaData info(listParameterPinIn->getMetaForPin(pinNum));
                    info.SetMeta(metaT::Removable,true);
                    new ComAddPin(myHost,info,com);
                }
                if(com->childCount())
                    emit UndoStackPush(com);
                else
                    delete com;
            }
        case LearningMode::off :
            if(listParameterPinOut->listPins.contains(pinNum))
                static_cast<ParameterPin*>(listParameterPinOut->listPins.value(pinNum))->ChangeValue( value, true );
            break;
    }
}

void VstAutomation::OnParameterChanged(const MetaData &pinInfo, float value)
{
    Object::OnParameterChanged(pinInfo,value);
    if(pininfo.GetMetaData<FixedPinNumber::Enum>(metaT::PinNumber)==FixedPinNumber::numberOfPins) {
        int nbPins=static_cast<ParameterPin*>( listParameterPinIn->listPins.value(FixedPinNumber::numberOfPins) )->GetIndex();

        QUndoCommand *com = new QUndoCommand(tr("Change number of pins"));
        QList<quint16>listAdded;
        QList<quint16>listRemoved;

        listParameterPinOut->SetNbPins(nbPins,&listAdded,&listRemoved);
        qSort(listRemoved.begin(),listRemoved.end(),qGreater<quint16>());
        foreach(quint16 i, listRemoved)
            new ComRemovePin(myHost,listParameterPinOut->listPins.value(i)->info(),com);
        foreach(quint16 i, listAdded) {
            MetaData info(listParameterPinOut->info());
            info.SetMeta(metaT::PinNumber, i);
            info.SetMeta(metaT::Removable,true);
            new ComAddPin(myHost,info,com);
        }

        listAdded.clear();
        listRemoved.clear();

        listParameterPinIn->SetNbPins(nbPins,&listAdded,&listRemoved);
        qSort(listRemoved.begin(),listRemoved.end(),qGreater<quint16>());
        foreach(quint16 i, listRemoved)
            new ComRemovePin(myHost,listParameterPinIn->listPins.value(i)->info(),com);
        foreach(quint16 i, listAdded) {
            MetaData info(listParameterPinIn->info());
            info.SetMeta(metaT::PinNumber, i);
            info.SetMeta(metaT::Removable,true);
            new ComAddPin(myHost,info,com);
        }

        if(com->childCount())
            emit UndoStackPush(com);
        else
            delete com;
        return;
    }

    if(pininfo.GetMetaData<Directions::Enum>(metaT::Direction)==Directions::Input && pininfo.GetMetaData<int>(metaT::PinNumber)<200)
        listChanged.insert(pininfo.GetMetaData<int>(metaT::PinNumber),value);
}

bool VstAutomation::Close()
{
    static_cast<MainHostVst*>(myHost)->myVstPlugin->removeVstAutomation(this);
    return Object::Close();
}

bool VstAutomation::Open()
{
    static_cast<MainHostVst*>(myHost)->myVstPlugin->addVstAutomation(this);
    return Object::Open();
}

Pin* VstAutomation::CreatePin(MetaData &info)
{
    Pin *newPin = Object::CreatePin(info);
    if(newPin)
        return newPin;

    if(info.GetMetaData<MediaTypes::Enum>(metaT::Media)!=MediaTypes::Parameter) {
        LOG("wrong PinType");
        return 0;
    }

    int pinnumber = info.GetMetaData<int>(metaT::PinNumber);

    switch(pinnumber) {

       case FixedPinNumber::vstProgNumber : {
            int prog=(int)static_cast<MainHostVst*>(myHost)->myVstPlugin->getProgram();
            info.SetName(tr("Prog"));
            ParameterPin *newPin = new ParameterPin(this,info,prog,&listValues);
            newPin->SetLimitsEnabled(false);
            return newPin;
        }

        case FixedPinNumber::numberOfPins : {
            info.SetName(tr("NbPins"));
            ParameterPin *newPin = new ParameterPin(this,info,VST_AUTOMATION_DEFAULT_NB_PINS,&listValues);
            newPin->SetLimitsEnabled(false);
            return newPin;
        }

        case FixedPinNumber::learningMode : {
            info.SetName(tr("Learn"));
            ParameterPin *newPin = new ParameterPin(this,info,"off",&listIsLearning);
            newPin->SetLimitsEnabled(false);
            return newPin;
        }

    }

    info.SetName(tr("autom%1").arg(pinnumber));
    return new ParameterPin(this,info,0);

    return 0;
}
