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

#ifndef CABLE_H
#define CABLE_H

#include "objectinfo.h"

class MainHost;
namespace Connectables {

    class Cable : public ObjectInfo
    {
    public:
        Cable();
        Cable(const MetaInfo &pinOut, const MetaInfo &pinIn);

        /*!
          Get the output pin info
          \return a ConnectionInfo
          */
        inline const MetaInfo & GetInfoOut() const {return pinOut;}

        /*!
          Get the input pin info
          \return a ConnectionInfo
          */
        inline const MetaInfo & GetInfoIn() const {return pinIn;}

        QDataStream & toStream (QDataStream &) const;
        QDataStream & fromStream (QDataStream &);

        void UpdatePinsParentIds(MainHost *host);
    protected:
        /// the output pin (from the sender object)
        MetaInfo pinOut;

        /// the input pin (the receiver object)
        MetaInfo pinIn;
    };
}

QDataStream & operator<< (QDataStream & out, const Connectables::Cable& value);
QDataStream & operator>> (QDataStream & in, Connectables::Cable& value);

#endif // CABLE_H
