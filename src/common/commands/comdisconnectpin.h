#ifndef COMDISCONNECTPIN_H
#define COMDISCONNECTPIN_H

#include "objectinfo.h"

class MainHost;
class ComDisconnectPin : public QUndoCommand
{
public:
    ComDisconnectPin(MainHost *myHost,
                     const MetaInfo &pinInfo,
                     QUndoCommand  *parent=0);
    void undo ();
    void redo ();

private:
    MainHost *myHost;
    MetaInfo pinInfo;
    QList<MetaInfo>listConnectedPins;

    int currentGroup;
    int currentProg;
};

#endif // COMDISCONNECTPIN_H
