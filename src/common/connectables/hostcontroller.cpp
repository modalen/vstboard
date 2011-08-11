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

#include "hostcontroller.h"
#include "globals.h"
#include "mainhost.h"
#include "models/programsmodel.h"

using namespace Connectables;

HostController::HostController(MainHost *myHost,int index):
    Object(myHost,index, ObjectInfo(MetaTypes::object, ObjTypes::HostController, index, tr("HostController") ) ),
    tempoChanged(false),
    progChanged(false),
    grpChanged(false)
{

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
            listPrg << i;
        }
        for(int i=0;i<128;i++) {
            listGrp << i;
        }

    int tempo=120;
    int sign1=4;
    int sign2=4;
    myHost->GetTempo(tempo,sign1,sign2);

    listParameterPinIn->listPins.insert(Param_Tempo, new ParameterPinIn(this,Param_Tempo,tempo,&listTempo,"bpm"));
    listParameterPinIn->listPins.insert(Param_Sign1, new ParameterPinIn(this,Param_Sign1,sign1,&listSign1,"sign1"));
    listParameterPinIn->listPins.insert(Param_Sign2, new ParameterPinIn(this,Param_Sign2,sign2,&listSign2,"sign2"));
    listParameterPinIn->listPins.insert(Param_Group, new ParameterPinIn(this,Param_Group, myHost->programsModel->GetCurrentMidiGroup(),&listGrp,"Group"));
    listParameterPinIn->listPins.insert(Param_Prog, new ParameterPinIn(this,Param_Prog, myHost->programsModel->GetCurrentMidiProg(),&listPrg,"Prog"));

    listParameterPinOut->listPins.insert(Param_Tempo, new ParameterPinOut(this,Param_Tempo,tempo,&listTempo,"bpm"));
    listParameterPinOut->listPins.insert(Param_Sign1, new ParameterPinOut(this,Param_Sign1,sign1,&listSign1,"sign1"));
    listParameterPinOut->listPins.insert(Param_Sign2, new ParameterPinOut(this,Param_Sign2,sign2,&listSign2,"sign2"));
    listParameterPinOut->listPins.insert(Param_Group, new ParameterPinOut(this,Param_Group, myHost->programsModel->GetCurrentMidiGroup(),&listGrp,"Group"));
    listParameterPinOut->listPins.insert(Param_Prog, new ParameterPinOut(this,Param_Prog, myHost->programsModel->GetCurrentMidiProg(),&listPrg,"Prog"));
    listParameterPinOut->listPins.insert(Param_Bar, new ParameterPinOut(this,Param_Bar, 0,"Bar"));

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

    pin=static_cast<ParameterPin*>(listParameterPinOut->listPins.value(Param_Bar));
#ifdef VSTSDK
    if(pin)
        pin->ChangeValue( myHost->vstHost->GetCurrentBarTic() );
#endif
}

void HostController::OnParameterChanged(ConnectionInfo pinInfo, float value)
{
    Object::OnParameterChanged(pinInfo,value);

    if(pinInfo.direction!=Directions::Input)
        return;

    switch(pinInfo.pinNumber) {
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

void HostController::SetContainerId(quint16 id)
{
    switch(id) {
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

    Object::SetContainerId(id);
}
