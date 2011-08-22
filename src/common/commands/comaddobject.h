#ifndef COMADDOBJECT_H
#define COMADDOBJECT_H

#include "precomp.h"
#include "connectables/objects/container.h"
#include "meta/metadata.h"

class MainHost;
class ComAddObject : public QUndoCommand
{
public:
    ComAddObject(MainHost *myHost,
                 const MetaData &objInfo,
                 const MetaData &targetInfo,
                 InsertionType::Enum insertType = InsertionType::NoInsertion,
                 QUndoCommand  *parent=0);
    void undo ();
    void redo ();

    void ReloadObject(const MetaData &info);

private:
    MainHost *myHost;

    MetaData objectInfo;
    MetaData targetInfo;

    InsertionType::Enum insertType;

    QList< QPair<MetaData,MetaData> >listAddedCables;
    QList< QPair<MetaData,MetaData> >listRemovedCables;

    MetaData attr;
    QByteArray objState;

    MetaData targetAttr;
    QByteArray targetState;

    int currentGroup;
    int currentProg;
};

#endif // COMADDOBJECT_H
