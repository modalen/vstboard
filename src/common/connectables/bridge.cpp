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

#include "bridge.h"
#include "objectfactory.h"
#include "../audiobuffer.h"
#include "../mainhost.h"
#include "bridgepinin.h"
#include "bridgepinout.h"

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
Bridge::Bridge(MainHost *myHost, ObjectInfo & info) :
        Object(myHost, info)
{
    ObjectInfo::metaType = MetaTypes::bridge;
}

/*!
  hide the output or input pins depending of the bridge type
  */
bool Bridge::Open()
{
    closed=false;

    listBridgePinIn->ChangeNumberOfPins(8);
    listBridgePinOut->ChangeNumberOfPins(8);

    switch(Meta(MetaInfos::Direction).toInt()) {
        case Directions::Input :
        case Directions::Return :
            listBridgePinIn->SetBridge(true);
            break;
        case Directions::Output :
        case Directions::Send :
            listBridgePinOut->SetBridge(true);
    }

    Object::Open();
    return true;
}
