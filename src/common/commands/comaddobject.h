#ifndef COMADDOBJECT_H
#define COMADDOBJECT_H

#include "precomp.h"
#include "connectables/objects/container.h"
#include "meta/metaobjengine.h"

class MainHost;
class ComAddObject : public QUndoCommand
{
public:
    ComAddObject(MainHost *myHost,
                 const MetaObjEngine &objInfo,
                 const MetaObjEngine &targetInfo,
                 InsertionType::Enum insertType = InsertionType::NoInsertion,
                 QUndoCommand  *parent=0);
    void undo ();
    void redo ();

    void ReloadObject(const MetaData &info);

private:
    MainHost *myHost;

    MetaObjEngine objectInfo;
    MetaObjEngine targetInfo;

    InsertionType::Enum insertType;

    QList< QPair<MetaPin,MetaPin> >listAddedCables;
    QList< QPair<MetaPin,MetaPin> >listRemovedCables;

    MetaObjViewAttrib attr;
    QByteArray objState;

    MetaObjViewAttrib targetAttr;
    QByteArray targetState;

    int currentGroup;
    int currentProg;
};

#endif // COMADDOBJECT_H
