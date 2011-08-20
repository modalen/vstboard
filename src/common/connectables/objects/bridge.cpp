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

#include "bridge.h"
//#include "connectables/objectfactory.h"
#include "audiobuffer.h"
#include "mainhost.h"
#include "connectables/pins/bridgepinin.h"
#include "connectables/pins/bridgepinout.h"

using namespace Connectables;

/*!
  \class Connectables::Bridge
  \brief used by containers to receive and send messages from and to other containers
  */

/*!
  Constructor
  \param myHost the MainHost
  \param index object number
  \param info ObjectInfo description of the object
  */
Bridge::Bridge(MainHost *myHost, MetaData & info) :
        Object(myHost, info)
{
    SetType(MetaType::bridge);

    listBridgePinIn = new PinsList(myHost,this);
    listBridgePinIn->SetName("listBridgePinIn");
    listBridgePinIn->data.SetMeta(metaT::Media,MediaTypes::Bridge);
    listBridgePinIn->data.SetMeta(metaT::Direction,Directions::Input);
    listBridgePinIn->SetParent(this);
    listBridgePinIn->SetObjId( myHost->objFactory->GetNewId() );
    listBridgePinIn->SetNbPins(8);
    pinLists << listBridgePinIn;

    listBridgePinOut = new PinsList(myHost,this);
    listBridgePinOut->SetName("listBridgePinOut");
    listBridgePinOut->data.SetMeta(metaT::Media,MediaTypes::Bridge);
    listBridgePinOut->data.SetMeta(metaT::Direction,Directions::Output);
    listBridgePinOut->SetParent(this);
    listBridgePinOut->SetObjId( myHost->objFactory->GetNewId() );
    listBridgePinOut->SetNbPins(8);
    pinLists << listBridgePinOut;

    if(data.GetMetaData<Directions::Enum>(metaT::Direction)==Directions::Input ||
            data.GetMetaData<Directions::Enum>(metaT::Direction)==Directions::Return) {
        listBridgePinIn->SetVisible(false);
        listBridgePinIn->SetBridge(true);
    }
    if(data.GetMetaData<Directions::Enum>(metaT::Direction)==Directions::Output ||
            data.GetMetaData<Directions::Enum>(metaT::Direction)==Directions::Send) {
        listBridgePinOut->SetVisible(false);
        listBridgePinOut->SetBridge(true);
    }
}

void Bridge::NewRenderLoop()
{
    foreach(Pin *p, listBridgePinIn->listPins) {
        p->NewRenderLoop();
    }
}

Pin* Bridge::CreatePin(MetaData &info)
{
    switch(info.GetMetaData<int>(metaT::Direction)) {
        case Directions::Input :
            info.SetName(QString("BridgeIn%1").arg(info.GetMetaData<int>(metaT::PinNumber)));
            return new BridgePinIn(this,info);
        case Directions::Output :
            info.SetName(QString("BridgeOut%1").arg(info.GetMetaData<int>(metaT::PinNumber)));
            return new BridgePinOut(this,info);
    }

    return 0;
}
