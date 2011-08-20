#include "comdisconnectpin.h"
#include "connectables/objectfactory.h"
#include "connectables/objects/container.h"
#include "mainhost.h"
#include "models/programsmodel.h"

ComDisconnectPin::ComDisconnectPin(MainHost *myHost,
                                   const MetaData &pinInfo,
                                   QUndoCommand  *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    pinInfo(pinInfo),
    currentGroup(0),
    currentProg(0)
{
    setText(QObject::tr("Disconnect pin"));

    currentGroup = myHost->programsModel->GetCurrentMidiGroup();
    currentProg =  myHost->programsModel->GetCurrentMidiProg();
}

void ComDisconnectPin::undo ()
{
    myHost->programsModel->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( pinInfo.ContainerId() ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;

    foreach(MetaData info, listConnectedPins) {
        myHost->objFactory->UpdatePinInfo( info );
        if(pininfo.GetMetaData<Directions::Enum>(metaT::Direction)==Directions::Output) {
            cntPtr->UserAddCable(pinInfo, info);
        } else {
            cntPtr->UserAddCable(info, pinInfo);
        }
    }
}

void ComDisconnectPin::redo ()
{
    myHost->programsModel->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( pinInfo.ContainerId() ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;

    listConnectedPins.clear();
    cntPtr->GetListOfConnectedPinsTo(pinInfo, listConnectedPins);
    cntPtr->UserRemoveCableFromPin(pinInfo);
}
