#include "cursor.h"
#include "mainhost.h"

Cursor::Cursor(MainHost *host, float value) :
    MsgHandler(host, host->objFactory->GetNewObjId()),
    value(value)
{
}

void Cursor::SetValue(float val)
{
    value=val;

    if(!MsgEnabled())
        return;
    MsgObject msg(GetIndex());
    msg.prop[MsgObject::Value]=value;
    msgCtrl->SendMsg(msg);
}

void Cursor::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Value)) {
        emit valueChanged( msg.prop[MsgObject::Value].toFloat() );
    }
}
