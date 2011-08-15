#ifndef NODE_H
#define NODE_H

//#include "precomp.h"
#include "connectables/objects/object.h"

class Node
{
public:
    Node();
    Node(const Node &c);
    int minRenderOrder;
    int maxRenderOrder;
    QList< QWeakPointer<Connectables::Object> >listOfObj;
};

#endif // NODE_H
