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

#ifndef METAPIN_H
#define METAPIN_H

#include "metaobjengine.h"

class MetaPin : public MetaObjEngine
{
public:
    MetaPin(MetaTransporter *transport=0, quint32 objid=0) :
        MetaObjEngine(MetaType::pin,transport,objid)
    {}

    void SetMedia(MediaTypes::Enum type) {SetMeta(metaT::Media, type);}
    MediaTypes::Enum Media() const {return GetMetaData<MediaTypes::Enum>(metaT::Media);}

    void SetDirection(Directions::Enum dir) {SetMeta(metaT::Direction, dir);}
    Directions::Enum Direction() const {return GetMetaData<Directions::Enum>(metaT::Direction);}

    void SetPinNumber(quint16 nb) {SetMeta(metaT::PinNumber, nb);}
    quint16 PinNumber() const {return GetMetaData<quint16>(metaT::PinNumber);}

    bool CanConnectPins(const MetaPin &p) const
    {
        //don't connect object to itself
    //    if(out.objId == c.objId)
    //        return false;

        if(Type()!=MetaType::pin)
            return false;
        if(p.Type()!=MetaType::pin)
            return false;

        //must be in the same container
        if(ContainerId() != p.ContainerId())
            return false;

        //must be opposite directions
        if(Direction() == p.Direction())
            return false;

        //must be the same type (audio/midi/automation) or a bridge pin
        if(Media() != MediaTypes::Bridge
            && p.Media()!=MediaTypes::Bridge
            && Media() != p.Media())
            return false;

        return true;
    }
};

#endif // METAPIN_H
