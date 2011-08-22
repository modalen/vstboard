#ifndef COMDISCONNECTPIN_H
#define COMDISCONNECTPIN_H

#include "meta/metapin.h"

class MainHost;
class ComDisconnectPin : public QUndoCommand
{
public:
    ComDisconnectPin(MainHost *myHost,
                     const MetaPin &pinInfo,
                     QUndoCommand  *parent=0);
    void undo ();
    void redo ();

private:
    MainHost *myHost;
    MetaPin pinInfo;
    QList<MetaPin>listConnectedPins;

    int currentGroup;
    int currentProg;
};

#endif // COMDISCONNECTPIN_H
