#ifndef COMCONNECTPIN_H
#define COMCONNECTPIN_H

#include <QUndoCommand>
#include "meta/metapin.h"

class MainHost;
class ComAddCable : public QUndoCommand
{
public:
    ComAddCable(MainHost *myHost,
                const MetaPin &outInfo,
                const MetaPin &inInfo,
                QUndoCommand  *parent=0);
    void undo ();
    void redo ();

private:
    MainHost *myHost;
    MetaPin outInfo;
    MetaPin inInfo;
    int currentGroup;
    int currentProg;
};

#endif // COMCONNECTPIN_H
