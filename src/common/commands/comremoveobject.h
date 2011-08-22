#ifndef COMREMOVEOBJECT_H
#define COMREMOVEOBJECT_H

#include "precomp.h"
#include "connectables/objects/container.h"

class MainHost;
class ComRemoveObject : public QUndoCommand
{
public:
    ComRemoveObject( MainHost *myHost,
                     const MetaObjEngine &objectInfo,
                     RemoveType::Enum removeType,
                     QUndoCommand  *parent=0);
    void undo ();
    void redo ();

private:
    MainHost *myHost;

    MetaObjEngine objectInfo;
    RemoveType::Enum removeType;
    MetaObjViewAttrib attr;

    QList< QPair<MetaPin,MetaPin> >listAddedCables;
    QList< QPair<MetaPin,MetaPin> >listRemovedCables;

    QByteArray objState;

    int currentGroup;
    int currentProg;
};

#endif // COMREMOVEOBJECT_H
