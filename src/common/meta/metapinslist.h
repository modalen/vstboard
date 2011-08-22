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

#ifndef METAPINSLIST_H
#define METAPINSLIST_H

#include "metaobjengine.h"

class MetaPinsList : public MetaObjEngine
{
public:
    MetaPinsList(quint32 objId=0, MetaTransporter *transport=0) :
        MetaObjEngine(MetaType::listPin,transport,objId)
    {}

    void SetMedia(MediaTypes::Enum type) {SetMeta(metaT::Media, type);}
    MediaTypes::Enum Media() const {return GetMetaData<MediaTypes::Enum>(metaT::Media);}

    void SetDirection(Directions::Enum dir) {SetMeta(metaT::Direction, dir);}
    Directions::Enum Direction() const {return GetMetaData<Directions::Enum>(metaT::Direction);}

};

#endif // METAPINSLIST_H
