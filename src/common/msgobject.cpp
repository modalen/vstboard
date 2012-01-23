#include "msgobject.h"

MsgObject::MsgObject() :
    objIndex(0),
    parentIndex(0)
{
}

MsgObject::MsgObject(int parentIndex, int objIndex) :
    objIndex(objIndex),
    parentIndex(parentIndex)
{

}
