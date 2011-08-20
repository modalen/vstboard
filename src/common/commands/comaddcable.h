#ifndef COMCONNECTPIN_H
#define COMCONNECTPIN_H

#include <QUndoCommand>
#include "objectinfo.h"

class MainHost;
class ComAddCable : public QUndoCommand
{
public:
    ComAddCable(MainHost *myHost,
                const MetaData &outInfo,
                const MetaData &inInfo,
                QUndoCommand  *parent=0);
    void undo ();
    void redo ();

private:
    MainHost *myHost;
    MetaData outInfo;
    MetaData inInfo;
    int currentGroup;
    int currentProg;
};

#endif // COMCONNECTPIN_H
