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
Bridge::Bridge(MainHost *myHost, MetaInfo & info) :
        Object(myHost, info)
{
    SetType(MetaTypes::bridge);

    listBridgePinIn = new PinsList(myHost,this);
    listBridgePinIn->SetName("listBridgePinIn");
    listBridgePinIn->SetMeta(MetaInfos::Media,MediaTypes::Bridge);
    listBridgePinIn->SetMeta(MetaInfos::Direction,Directions::Input);
    listBridgePinIn->SetParent(this);
    listBridgePinIn->SetObjId( myHost->objFactory->GetNewId() );
    listBridgePinIn->SetNbPins(8);
    pinLists << listBridgePinIn;

    listBridgePinOut = new PinsList(myHost,this);
    listBridgePinOut->SetName("listBridgePinOut");
    listBridgePinOut->SetMeta(MetaInfos::Media,MediaTypes::Bridge);
    listBridgePinOut->SetMeta(MetaInfos::Direction,Directions::Output);
    listBridgePinOut->SetParent(this);
    listBridgePinOut->SetObjId( myHost->objFactory->GetNewId() );
    listBridgePinOut->SetNbPins(8);
    pinLists << listBridgePinOut;

    if(Meta(MetaInfos::Direction).toInt()==Directions::Input ||
            Meta(MetaInfos::Direction).toInt()==Directions::Return) {
        listBridgePinIn->SetVisible(false);
        listBridgePinIn->SetBridge(true);
    }
    if(Meta(MetaInfos::Direction).toInt()==Directions::Output ||
            Meta(MetaInfos::Direction).toInt()==Directions::Send) {
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

Pin* Bridge::CreatePin(MetaInfo &info)
{
    switch(info.Meta(MetaInfos::Direction).toInt()) {
        case Directions::Input :
            info.SetName(QString("BridgeIn%1").arg(info.Meta(MetaInfos::PinNumber).toInt()));
            return new BridgePinIn(this,info);
        case Directions::Output :
            info.SetName(QString("BridgeOut%1").arg(info.Meta(MetaInfos::PinNumber).toInt()));
            return new BridgePinOut(this,info);
    }

    return 0;
}
