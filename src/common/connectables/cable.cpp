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


#include "cable.h"
#include "mainhost.h"

using namespace Connectables;

/*!
  \class Connectables::Cable
  \brief cable are created in Container to connect Objects
  */

Cable::Cable() :
    ObjectInfo()
{
    SetType(MetaTypes::cable);
    SetName("Cable");
}

/*!
  \param myHost pointer to the MainHost
  \param pinOut the output pin (the one sending the messages)
  \param pinIn the input pin (the receiver)
  */
Cable::Cable( const MetaInfo &pinOut, const MetaInfo &pinIn) :
    ObjectInfo(),
    pinOut(pinOut),
    pinIn(pinIn)
{
    SetType(MetaTypes::cable);
    SetName("Cable");
    SetMeta(MetaInfos::nbOutputs,QVariant::fromValue(pinOut.info()));
    SetMeta(MetaInfos::nbInputs,QVariant::fromValue(pinIn.info()));
}

QDataStream & Cable::toStream (QDataStream& out) const
{
    out << *(MetaInfo*)this;
    out << pinOut;
    out << pinIn;
    return out;
}

QDataStream & Cable::fromStream (QDataStream& in)
{
    in >> *(MetaInfo*)this;
    in >> pinOut;
    in >> pinIn;

    pinOut.SetParentObjectId( MetaInfo::GetIdFromSavedId( pinOut.ParentObjectId() ) );
    pinIn.SetParentObjectId( MetaInfo::GetIdFromSavedId( pinIn.ParentObjectId() ) );
    return in;
}

void Cable::UpdatePinsParentIds(MainHost *host)
{

    host->objFactory->UpdatePinInfo(pinIn);
    host->objFactory->UpdatePinInfo(pinOut);

    SetMeta(MetaInfos::nbOutputs,QVariant::fromValue(pinOut.info()));
    SetMeta(MetaInfos::nbInputs,QVariant::fromValue(pinIn.info()));
}

QDataStream & operator<< (QDataStream & out, const Connectables::Cable& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream & in, Connectables::Cable& value)
{
    return value.fromStream(in);
}
