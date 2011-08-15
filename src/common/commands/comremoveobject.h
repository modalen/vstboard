#ifndef COMREMOVEOBJECT_H
#define COMREMOVEOBJECT_H

#include "precomp.h"
#include <QUndoCommand>
#include "connectables/objects/container.h"


class MainHost;
class ComRemoveObject : public QUndoCommand
{
public:
    ComRemoveObject( MainHost *myHost,
                     const MetaInfo &objectInfo,
                     RemoveType::Enum removeType,
                     QUndoCommand  *parent=0);
    void undo ();
    void redo ();

private:
    MainHost *myHost;

    MetaInfo objectInfo;
    RemoveType::Enum removeType;
    ObjectContainerAttribs attr;

    QList< QPair<MetaInfo,MetaInfo> >listAddedCables;
    QList< QPair<MetaInfo,MetaInfo> >listRemovedCables;

    QByteArray objState;

    int currentGroup;
    int currentProg;
};

#endif // COMREMOVEOBJECT_H
