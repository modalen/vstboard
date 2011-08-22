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
    MetaObjEngine(MetaType::cable)
{
    SetName("Cable");
}

/*!
  \param myHost pointer to the MainHost
  \param pinOut the output pin (the one sending the messages)
  \param pinIn the input pin (the receiver)
  */
Cable::Cable( const MetaPin &pinOut, const MetaPin &pinIn) :
    MetaObjEngine(MetaType::cable),
    pinOut(pinOut),
    pinIn(pinIn)
{
    SetType(MetaType::cable);
    SetName("Cable");

    SetMeta(metaT::ObjIdOut, pinOut.MetaId());
    SetMeta(metaT::ObjIdIn, pinIn.MetaId());
}

QDataStream & Cable::toStream (QDataStream& out) const
{
    out << *(MetaData*)this;
    out << pinOut;
    out << pinIn;
    return out;
}

QDataStream & Cable::fromStream (QDataStream& in)
{
    in >> *(MetaData*)this;
    in >> pinOut;
    in >> pinIn;
    return in;
}

QDataStream & operator<< (QDataStream & out, const Connectables::Cable& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream & in, Connectables::Cable& value)
{
    return value.fromStream(in);
}
