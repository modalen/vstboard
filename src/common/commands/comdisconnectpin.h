#ifndef COMDISCONNECTPIN_H
#define COMDISCONNECTPIN_H

#include <QUndoCommand>
#include "objectinfo.h"

class MainHost;
class ComDisconnectPin : public QUndoCommand
{
public:
    ComDisconnectPin(MainHost *myHost,
                     const ObjectInfo &pinInfo,
                     QUndoCommand  *parent=0);
    void undo ();
    void redo ();

private:
    MainHost *myHost;
    ObjectInfo pinInfo;
    QList<ObjectInfo>listConnectedPins;

    int currentGroup;
    int currentProg;
};

#endif // COMDISCONNECTPIN_H
