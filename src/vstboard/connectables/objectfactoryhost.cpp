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

#include "objectfactoryhost.h"

#include "connectables/audiodevicein.h"
#include "connectables/audiodeviceout.h"
#include "connectables/mididevice.h"

using namespace Connectables;

ObjectFactoryHost::ObjectFactoryHost(MainHost *myHost) :
    ObjectFactory(myHost)
{
}

Object *ObjectFactoryHost::CreateOtherObjects(ObjectInfo &info)
{
    Object *obj=0;

    switch(info.Meta()) {
        case MetaTypes::object :

            switch(info.Meta(MetaInfos::ObjType).toInt()) {
                case ObjTypes::AudioInterface:
                    switch(info.Meta(MetaInfos::Direction).toInt()) {
                        case Directions::Input :
                            obj = new AudioDeviceIn(myHost, info);
                            break;
                        case Directions::Output :
                            obj = new AudioDeviceOut(myHost, info);
                            break;
                    }
                    break;

                case ObjTypes::MidiInterface:
                    obj = new MidiDevice(myHost, info);
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }

    return obj;
}
