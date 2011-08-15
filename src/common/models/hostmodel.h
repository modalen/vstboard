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

#ifndef HOSTMODEL_H
#define HOSTMODEL_H

#include "precomp.h"

namespace InsertionType {
    enum Enum {
        NoInsertion,
        InsertBefore,
        InsertAfter,
        Replace,
        AddBefore,
        AddAfter
    };
}

namespace RemoveType {
    enum Enum {
        RemoveWithCables,
        BridgeCables
    };
}

class MainHost;
class MetaInfo;
class HostModel : QObject
{
    Q_OBJECT
public:
    HostModel(MainHost *myHost=0, QObject *parent=0);
    bool dropMime ( const QMimeData * data, MetaInfo & senderInfo, InsertionType::Enum insertType=InsertionType::NoInsertion );
    void valueChanged( const MetaInfo & senderInfo, int info, const QVariant &value);
protected:
    MainHost *myHost;
    QTimer *delayedAction;
    QSignalMapper *LoadFileMapper;
};

#endif // HOSTMODEL_H
