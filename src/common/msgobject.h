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
private:
    QDataStream & toStream (QDataStream &out) const;
    QDataStream & fromStream (QDataStream &in);
    friend QDataStream & operator<< (QDataStream&, const MsgObject&);
    friend QDataStream & operator>> (QDataStream&, MsgObject&);
};

QDataStream & operator<< (QDataStream& out, const MsgObject& value);
QDataStream & operator>> (QDataStream& in, MsgObject& value);

Q_DECLARE_METATYPE(MsgObject);
typedef QList<MsgObject> ListMsgObj;
Q_DECLARE_METATYPE(ListMsgObj);

#endif // MSGOBJECT_H
