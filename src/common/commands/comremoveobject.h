#ifndef COMREMOVEOBJECT_H
#define COMREMOVEOBJECT_H

#include "precomp.h"
#include "connectables/objects/container.h"

class MainHost;
class ComRemoveObject : public QUndoCommand
{
public:
    ComRemoveObject( MainHost *myHost,
                     const MetaData &objectInfo,
                     RemoveType::Enum removeType,
                     QUndoCommand  *parent=0);
    void undo ();
    void redo ();

private:
    MainHost *myHost;

    MetaData objectInfo;
    RemoveType::Enum removeType;
    ObjectContainerAttribs attr;

    QList< QPair<MetaData,MetaData> >listAddedCables;
    QList< QPair<MetaData,MetaData> >listRemovedCables;

    QByteArray objState;

    int currentGroup;
    int currentProg;
};

#endif // COMREMOVEOBJECT_H
