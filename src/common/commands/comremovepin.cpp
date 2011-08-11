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

#include "comremovepin.h"
#include "mainhost.h"
#include "models/programsmodel.h"

ComRemovePin::ComRemovePin(MainHost *myHost,
                           const ObjectInfo &pinInfo,
                           QUndoCommand  *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    pinInfo(pinInfo),
    currentGroup(0),
    currentProg(0)
{
    setText(QObject::tr("Remove pin"));

    currentGroup = myHost->programsModel->GetCurrentMidiGroup();
    currentProg =  myHost->programsModel->GetCurrentMidiProg();
}

void ComRemovePin::undo ()
{
    myHost->programsModel->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( pinInfo.ContainerId() ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;

    QSharedPointer<Connectables::Object>objPtr = myHost->objFactory->GetObjectFromId( pinInfo.ObjId() );
    if(!objPtr)
        return;

    objPtr->UserAddPin(pinInfo);

    foreach( ObjectInfo info, listConnectedPins) {
        if(pinInfo.Meta(MetaInfos::Direction).toInt()==Directions::Output)
            cntPtr->UserAddCable(pinInfo,info);
        else
            cntPtr->UserAddCable(info,pinInfo);
    }

    Connectables::ParameterPin *pin = static_cast<Connectables::ParameterPin*>(myHost->objFactory->GetPin(pinInfo));
    if(pin) {
        pin->SetVariantValue(value);
    }

    listConnectedPins.clear();
}

void ComRemovePin::redo ()
{
    myHost->programsModel->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( pinInfo.ContainerId() ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;

    QSharedPointer<Connectables::Object>objPtr = myHost->objFactory->GetObjectFromId( pinInfo.ObjId() );
    if(!objPtr)
        return;

    Connectables::ParameterPin *pin = static_cast<Connectables::ParameterPin*>(myHost->objFactory->GetPin(pinInfo));
    if(pin) {
        value=pin->GetVariantValue();
    }

    cntPtr->GetListOfConnectedPinsTo(pinInfo,listConnectedPins);
    objPtr->UserRemovePin(pinInfo);
}
