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

#include "hostcontroller.h"
#include "globals.h"
#include "mainhost.h"
#include "models/programsmodel.h"

using namespace Connectables;

HostController::HostController(MainHost *myHost, MetaInfo &info):
    Object(myHost, info ),
    tempoChanged(false),
    progChanged(false),
    grpChanged(false)
{

    SetName("HostController");

    for(int i=1;i<300;i++) {
        listTempo << i;
    }

    for(int i=2;i<30;i++) {
        listSign1 << i;
    }

    for(int i=0;i<9;i++) {
        listSign2 << (1<<i);
    }
    for(int i=0;i<128;i++) {
        list128 << i;
    }

    listParameterPinIn->AddPin(Param_Tempo);
    listParameterPinIn->AddPin(Param_Sign1);
    listParameterPinIn->AddPin(Param_Sign2);
    listParameterPinIn->AddPin(Param_Group);
    listParameterPinIn->AddPin(Param_Prog);

    listParameterPinOut->AddPin(Param_Tempo);
    listParameterPinOut->AddPin(Param_Sign1);
    listParameterPinOut->AddPin(Param_Sign2);
    listParameterPinOut->AddPin(Param_Group);
    listParameterPinOut->AddPin(Param_Prog);
    listParameterPinOut->AddPin(Param_Bar);

    connect(this, SIGNAL(progChange(int)),
            myHost->programsModel,SLOT(UserChangeProg(int)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(grpChange(int)),
            myHost->programsModel,SLOT(UserChangeGroup(int)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(tempoChange(int,int,int)),
            myHost,SLOT(SetTempo(int,int,int)),
            Qt::QueuedConnection);

    connect(myHost->programsModel,SIGNAL(ProgChanged(QModelIndex)),
           this,SLOT(OnHostProgChanged(QModelIndex)));
    connect(myHost->programsModel,SIGNAL(GroupChanged(QModelIndex)),
           this,SLOT(OnHostGroupChanged(QModelIndex)));
    connect(myHost,SIGNAL(TempoChanged(int,int,int)),
            this,SLOT(OnHostTempoChange(int,int,int)));
}

void HostController::Render()
{
    ParameterPin *pin=0;

    if(tempoChanged) {
        tempoChanged=false;
        int tempo=120;
        int sign1=4;
        int sign2=4;

        pin=static_cast<ParameterPin*>(listParameterPinIn->listPins.value(Param_Tempo));
        if(pin)
            tempo = pin->GetVariantValue().toInt();
        pin=static_cast<ParameterPin*>(listParameterPinIn->listPins.value(Param_Sign1));
        if(pin)
            sign1 = pin->GetVariantValue().toInt();
        pin=static_cast<ParameterPin*>(listParameterPinIn->listPins.value(Param_Sign2));
        if(pin)
            sign2 = pin->GetVariantValue().toInt();

        emit tempoChange(tempo,sign1,sign2);
    }

    if(progChanged) {
        progChanged=false;
        pin=static_cast<ParameterPin*>(listParameterPinIn->listPins.value(Param_Prog));
        if(pin)
            emit progChange( pin->GetVariantValue().toInt() );
    }
    if(grpChanged) {
        grpChanged=false;
        pin=static_cast<ParameterPin*>(listParameterPinIn->listPins.value(Param_Group));
        if(pin)
            emit grpChange( pin->GetVariantValue().toInt() );
    }

#ifdef VSTSDK
    pin=static_cast<ParameterPin*>(listParameterPinOut->listPins.value(Param_Bar));
    if(pin) {
        int step;
        float percent =  myHost->vstHost->GetCurrentBarTic(step);
        pin->ChangeValue( percent );
        pin->SetMeta(MetaInfos::displayedText, QString("%1:%2").arg(pin->Name()).arg(step) );
    }
#endif
}

void HostController::OnParameterChanged(const MetaInfo &pinInfo, float value)
{
    Object::OnParameterChanged(pinInfo,value);

    if(pinInfo.Meta(MetaInfos::Direction).toInt()!=Directions::Input)
        return;

    switch(pinInfo.Meta(MetaInfos::PinNumber).toInt()) {
        case Param_Tempo :
        case Param_Sign1 :
        case Param_Sign2 :
            tempoChanged=true;
            break;
        case Param_Group :
            grpChanged=true;
            break;
        case Param_Prog :
            progChanged=true;
            break;
    }
}

void HostController::OnHostProgChanged(const QModelIndex &idx)
{
    if(listParameterPinOut->listPins.contains(Param_Prog))
        static_cast<ParameterPin*>(listParameterPinOut->listPins.value(Param_Prog))->ChangeValue( idx.row(), true );
}

void HostController::OnHostGroupChanged(const QModelIndex &idx)
{
    if(listParameterPinOut->listPins.contains(Param_Group))
        static_cast<ParameterPin*>(listParameterPinOut->listPins.value(Param_Group))->ChangeValue( idx.row(), true );
}

void HostController::OnHostTempoChange(int tempo, int sign1, int sign2)
{
    static_cast<ParameterPin*>(listParameterPinOut->listPins.value(Param_Tempo))->SetVariantValue( tempo);
    static_cast<ParameterPin*>(listParameterPinOut->listPins.value(Param_Sign1))->SetVariantValue( sign1 );
    static_cast<ParameterPin*>(listParameterPinOut->listPins.value(Param_Sign2))->SetVariantValue( sign2 );
}

void HostController::SetContainer(ObjectInfo *container)
{
    switch(container->ObjId()) {
        case FixedObjIds::programContainer :
            listParameterPinIn->RemovePin(Param_Prog);
            disconnect(this, SIGNAL(progChange(int)),
                    myHost->programsModel,SLOT(UserChangeProg(int)));
            disconnect(myHost->programsModel,SIGNAL(ProgChanged(QModelIndex)),
                   this,SLOT(OnHostProgChanged(QModelIndex)));

        case FixedObjIds::groupContainer :
            listParameterPinIn->RemovePin(Param_Group);
            disconnect(this, SIGNAL(grpChange(int)),
                    myHost->programsModel,SLOT(UserChangeGroup(int)));
            disconnect(myHost->programsModel,SIGNAL(GroupChanged(QModelIndex)),
                   this,SLOT(OnHostGroupChanged(QModelIndex)));
    }

    ObjectInfo::SetContainer(container);
}

Pin* HostController::CreatePin(MetaInfo &info)
{
    Pin *newPin = Object::CreatePin(info);
    if(newPin)
        return newPin;

    int pinnumber = info.Meta(MetaInfos::PinNumber).toInt();
    int tempo=120;
    int sign1=4;
    int sign2=4;
    myHost->GetTempo(tempo,sign1,sign2);

    switch(pinnumber) {
        case Param_Tempo:
            info.SetName(tr("bpm"));
            return new ParameterPin(this,info,tempo,&listTempo);
        case Param_Sign1:
            info.SetName(tr("sign1"));
            return new ParameterPin(this,info,sign1,&listSign1);
        case Param_Sign2:
            info.SetName(tr("sign2"));
            return new ParameterPin(this,info,sign2,&listSign2);
        case Param_Group:
            info.SetName(tr("Group"));
            return new ParameterPin(this,info, myHost->programsModel->GetCurrentMidiGroup(),&list128);
        case Param_Prog:
            info.SetName(tr("Prog"));
            return new ParameterPin(this,info, myHost->programsModel->GetCurrentMidiProg(),&list128);
        case Param_Bar:
            info.SetName(tr("Bar"));
            return new ParameterPin(this,info, 0, &list128);
    }

    return 0;
}
