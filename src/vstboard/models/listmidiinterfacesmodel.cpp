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
#include "listmidiinterfacesmodel.h"
#include "objectinfo.h"

ListMidiInterfacesModel::ListMidiInterfacesModel(QObject *parent):
        QStandardItemModel(parent)
{
}

QMimeData  * ListMidiInterfacesModel::mimeData ( const QModelIndexList  & indexes ) const
{
    QMimeData  *data = new QMimeData();
    QByteArray b;
    QDataStream stream(&b,QIODevice::WriteOnly);

    foreach(QModelIndex idx, indexes) {
        if(idx.column()!=0)
            continue;
        stream << itemFromIndex(idx)->data(UserRoles::objInfo).value<ObjectInfo>();
    }

    data->setData("application/x-midiinterface",b);
    return data;
}
