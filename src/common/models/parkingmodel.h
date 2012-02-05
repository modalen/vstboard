#ifndef PARKINGMODEL_H
#define PARKINGMODEL_H

#include "precomp.h"
#include "msghandler.h"

class ParkingModel : public QStandardItemModel, public MsgHandler
{
public:
    explicit ParkingModel( MsgController *msgCtrl, int objId, QObject *parent=0);
    void ReceiveMsg(const MsgObject &msg);
    
};

#endif // PARKINGMODEL_H
