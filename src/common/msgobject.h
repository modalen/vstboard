#ifndef MSGOBJECT_H
#define MSGOBJECT_H

#include "precomp.h"
#include "connectables/objectinfo.h"

class MsgObject
{
public:
    MsgObject();
    MsgObject(int parentIndex, int objIndex);

    int objIndex;
    int parentIndex;
    QMap<QString,QVariant>prop;
    QList<MsgObject>children;
};

Q_DECLARE_METATYPE(MsgObject);
typedef QList<MsgObject> ListMsgObj;
Q_DECLARE_METATYPE(ListMsgObj);

#endif // MSGOBJECT_H
