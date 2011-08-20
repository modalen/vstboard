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

#include "metatransporter.h"



void MetaTransporter::ValueChanged( const MetaData & senderInfo, int type, const QVariant &value)
{
    if(!autoUpdate)
        return;
    Events::valChanged *e = new Events::valChanged(MetaData(senderInfo), (metaT::Enum)type, value);
    PostEvent(e);
}

void MetaTransporter::PostEvent( QEvent * event) {
//#ifndef QT_NO_DEBUG
//    if(listeners.isEmpty()) {
//        LOG("no listener")
//    }
//#endif
    foreach(QObject *obj, listeners) {
        qApp->postEvent(obj,event);
    }
}
