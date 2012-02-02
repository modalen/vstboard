#include "parkingmodel.h"

ParkingModel::ParkingModel(MsgController *msgCtrl, int objId, QObject *parent) :
    QStandardItemModel(parent),
    MsgHandler(msgCtrl, objId)
{
}

void ParkingModel::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains("addObject")) {
        ObjectInfo info = msg.prop["objInfo"].value<ObjectInfo>();
        QStandardItem *item = new QStandardItem( info.name );
        item->setData(msg.prop["addObject"].toInt());
        item->setData(QVariant::fromValue(info),UserRoles::objInfo);
        invisibleRootItem()->appendRow(item);
        return;
    }

    if(msg.prop.contains("removeObject")) {
        int nb=rowCount();
        for(int i=0; i<nb; ++i) {
            if(invisibleRootItem()->child(i)->data().toInt()==msg.prop["removeObject"].toInt()) {
                invisibleRootItem()->removeRow(i);
                return;
            }
        }
        return;
    }

    if(msg.prop.contains("clear")) {
        clear();
        return;
    }
}
