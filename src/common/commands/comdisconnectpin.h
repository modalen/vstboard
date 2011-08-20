#ifndef COMDISCONNECTPIN_H
#define COMDISCONNECTPIN_H

#include "objectinfo.h"

class MainHost;
class ComDisconnectPin : public QUndoCommand
{
public:
    ComDisconnectPin(MainHost *myHost,
                     const MetaData &pinInfo,
                     QUndoCommand  *parent=0);
    void undo ();
    void redo ();

private:
    MainHost *myHost;
    MetaData pinInfo;
    QList<MetaData>listConnectedPins;

    int currentGroup;
    int currentProg;
};

#endif // COMDISCONNECTPIN_H
