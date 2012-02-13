#include "parkingmodel.h"

ParkingModel::ParkingModel(MsgController *msgCtrl, int objId, QObject *parent) :
    QStandardItemModel(parent),
    MsgHandler(msgCtrl, objId)
{
}

void ParkingModel::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Add)) {
        ObjectInfo info = msg.prop[MsgObject::ObjInfo].value<ObjectInfo>();
        QStandardItem *item = new QStandardItem( info.name );
        item->setData(msg.prop[MsgObject::Add].toInt());
        item->setData(QVariant::fromValue(info),UserRoles::objInfo);
        invisibleRootItem()->appendRow(item);
        return;
    }

    if(msg.prop.contains(MsgObject::Remove)) {
        int nb=rowCount();
        for(int i=0; i<nb; ++i) {
            if(invisibleRootItem()->child(i)->data().toInt()==msg.prop[MsgObject::Remove].toInt()) {
                invisibleRootItem()->removeRow(i);
                return;
            }
        }
        return;
    }

    if(msg.prop.contains(MsgObject::Clear)) {
        clear();
        return;
    }
}
