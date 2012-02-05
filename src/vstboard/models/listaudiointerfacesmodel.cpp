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
#include "listaudiointerfacesmodel.h"
#include "connectables/objectinfo.h"

ListAudioInterfacesModel::ListAudioInterfacesModel(MsgController *msgCtrl, int objId, QObject *parent) :
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

Qt::ItemFlags ListAudioInterfacesModel::flags ( const QModelIndex & index ) const
{
    if(!index.parent().isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;

}

QMimeData  * ListAudioInterfacesModel::mimeData ( const QModelIndexList  & indexes ) const
{
    QMimeData  *data = new QMimeData();
    QByteArray b;
    QDataStream stream(&b,QIODevice::WriteOnly);

    QStandardItem *item = itemFromIndex(indexes.first());

    foreach(QModelIndex idx, indexes) {
        //don't drag api
        if(item->parent()==0)
            continue;
        if(idx.column()!=0)
            continue;
        stream << itemFromIndex(idx)->data(UserRoles::objInfo).value<ObjectInfo>();
    }

    data->setData("application/x-audiointerface",b);
    return data;
}

void ListAudioInterfacesModel::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains("state")) {
        for(int i=0; i<rowCount(); i++) {
            if(index(i,0).data(UserRoles::value).toInt() == msg.prop["api"].toInt()) {
                QModelIndex apiIdx = index(i,0);
                for(int j=0; j<rowCount(apiIdx); j++) {
                    if(index(j,0,apiIdx).data(UserRoles::value).toInt() == msg.prop["dev"].toInt()) {
                        if(msg.prop["state"].toBool())
                            itemFromIndex(index(j,3,apiIdx))->setCheckState(Qt::Checked);
                        else
                            itemFromIndex(index(j,3,apiIdx))->setCheckState(Qt::Unchecked);
                    }
                    return;
                }
            }
        }
        return;
    }

    if(msg.prop.contains("fullUpdate")) {
        invisibleRootItem()->removeRows(0, rowCount());
        expandedIndex.clear();
//        QStringList headerLabels;
//        headerLabels << "Name";
//        headerLabels << "In";
//        headerLabels << "Out";
//        headerLabels << "InUse";
//        setHorizontalHeaderLabels(  headerLabels );

        foreach(const MsgObject &msgApi, msg.children) {
            QStandardItem *apiItem = new QStandardItem(msgApi.prop["name"].toString());
            apiItem->setData( msgApi.objIndex, UserRoles::value );
            apiItem->setDragEnabled(false);
            apiItem->setSelectable(false);
//            if(msgApi.prop.contains("expand"))
//                expandedIndex << apiItem->index();

            foreach(const MsgObject &msgDevice, msgApi.children) {
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

                QStandardItem *inUseItem = new QStandardItem();
                inUseItem->setCheckable(true);
                inUseItem->setEditable(false);
                if(msgDevice.prop["state"].toBool())
                    inUseItem->setCheckState(Qt::Checked);
                else
                    inUseItem->setCheckState(Qt::Unchecked);
                listItems << inUseItem;

                apiItem->appendRow( listItems );
            }

            invisibleRootItem()->appendRow(apiItem);
        }
    }
}

void ListAudioInterfacesModel::Update()
{
    MsgObject msg(-1,GetIndex());
    msg.prop["fullUpdate"]=1;
    msgCtrl->SendMsg(msg);
}

void ListAudioInterfacesModel::Rescan()
{
    MsgObject msg(-1,GetIndex());
    msg.prop["rescan"]=1;
    msgCtrl->SendMsg(msg);
}
