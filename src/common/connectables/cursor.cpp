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
    MsgObject a(-1,GetIndex());
    a.prop["actionType"]="update";
    a.prop["value"]=value;
    msgCtrl->SendMsg(a);
}

void Cursor::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains("value")) {
        emit valueChanged( msg.prop.value("value").toFloat() );
    }
}
