#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include "msgcontroller.h"

class MsgObject;
class MsgHandler
{
public:
    MsgHandler(MsgController *msgCtrl, int objId);
    virtual ~MsgHandler();
    virtual void GetInfos(MsgObject &) {}
    int GetIndex() const {return objId;}
    bool SetIndex(int id);
    virtual void ReceiveMsg(const MsgObject &) {}
    MsgController *msgCtrl;
    int objId;
};

#endif // MSGHANDLER_H
