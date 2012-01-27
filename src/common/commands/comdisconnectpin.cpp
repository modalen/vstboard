#include "comdisconnectpin.h"
#include "connectables/objectfactory.h"
#include "connectables/container.h"
#include "mainhost.h"
#include "programmanager.h"

ComDisconnectPin::ComDisconnectPin(MainHost *myHost,
                                   const ConnectionInfo &pinInfo,
                                   QUndoCommand  *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    pinInfo(pinInfo),
    currentGroup(0),
    currentProg(0)
{
    setText(QObject::tr("Disconnect pin"));

    currentGroup = myHost->programManager->GetCurrentMidiGroup();
    currentProg =  myHost->programManager->GetCurrentMidiProg();
}

void ComDisconnectPin::undo ()
{
    myHost->programManager->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( pinInfo.container ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;

    foreach(ConnectionInfo info, listConnectedPins) {
        if(pinInfo.direction==PinDirection::Output) {
            cntPtr->UserAddCable(pinInfo, info);
        } else {
            cntPtr->UserAddCable(info, pinInfo);
        }
    }
}

void ComDisconnectPin::redo ()
{
    myHost->programManager->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( pinInfo.container ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;

    listConnectedPins.clear();
    cntPtr->GetListOfConnectedPinsTo(pinInfo, listConnectedPins);
    cntPtr->UserRemoveCableFromPin(pinInfo);
}
