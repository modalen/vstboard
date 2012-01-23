#include "msghandler.h"
#include "msgobject.h"

MsgHandler::MsgHandler(MsgController *msgCtrl, int objId) :
    msgCtrl(msgCtrl),
    objId(objId)
{
    if(objId==-1)
        return;
    if(msgCtrl->listObj.contains(objId)) {
        LOG("objId already exists"<<objId)
        return;
    }
    msgCtrl->listObj[objId]=this;
}

MsgHandler::~MsgHandler()
{
    msgCtrl->listObj.remove(objId);
}

bool MsgHandler::SetIndex(int id)
{
    if(objId==id)
        return true;

    if(objId!=-1) {
        LOG("reset id not allowed")
        return false;
    }
    if(msgCtrl->listObj.contains(id)) {
        LOG("objId already exists"<<objId)
        return false;
    }
    objId=id;
    msgCtrl->listObj[objId]=this;
    return true;
}
