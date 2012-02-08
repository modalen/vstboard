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
#include "listmidiinterfacesmodel.h"
#include "connectables/objectinfo.h"

ListMidiInterfacesModel::ListMidiInterfacesModel(MsgController *msgCtrl, int objId, QObject *parent):
    QStandardItemModel(parent),
    MsgHandler(msgCtrl, objId)
{
    QStringList headerLabels;
    headerLabels << "Name";
    headerLabels << "In";
    headerLabels << "Out";
    headerLabels << "InUse";
    setHorizontalHeaderLabels(  headerLabels );
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

void ListMidiInterfacesModel::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains("state")) {
        for(int i=0; i<rowCount(); i++) {
            if(index(i,0).data(UserRoles::value).toInt() == msg.prop["dev"].toInt()) {
                if(msg.prop["state"].toBool())
                    item(i,3)->setCheckState(Qt::Checked);
                else
                    item(i,3)->setCheckState(Qt::Unchecked);

                return;
            }
        }
        return;
    }

    if(msg.prop.contains("fullUpdate")) {
        invisibleRootItem()->removeRows(0, rowCount());

        foreach(const MsgObject &msgDevice, msg.children) {
            QList<QStandardItem *> listItems;
            QStandardItem *devItem = new QStandardItem(msgDevice.prop["name"].toString());
            devItem->setEditable(false);
            ObjectInfo obj = msgDevice.prop["objInfo"].value<ObjectInfo>();
            devItem->setData(QVariant::fromValue(obj), UserRoles::objInfo);
            devItem->setData(obj.id, UserRoles::value);
            devItem->setDragEnabled(true);
            listItems << devItem;

            QStandardItem *inputItem = new QStandardItem( QString::number(obj.inputs));
            inputItem->setEditable(false);
            listItems << inputItem;

            QStandardItem *outputItem = new QStandardItem( QString::number(obj.outputs));
            outputItem->setEditable(false);
            listItems << outputItem;

            QStandardItem *inUseItem = new QStandardItem(false);
            inUseItem->setEditable(false);
            if(msgDevice.prop["state"].toBool())
                inUseItem->setCheckState(Qt::Checked);
            else
                inUseItem->setCheckState(Qt::Unchecked);

            listItems << inUseItem;

            invisibleRootItem()->appendRow(listItems);
        }
    }
}


void ListMidiInterfacesModel::Update()
{
    MsgObject msg(-1,GetIndex());
    msg.prop["fullUpdate"]=1;
    msgCtrl->SendMsg(msg);
}

void ListMidiInterfacesModel::Rescan()
{
    MsgObject msg(-1,GetIndex());
    msg.prop["rescan"]=1;
    msgCtrl->SendMsg(msg);
}
