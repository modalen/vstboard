#include "comaddcable.h"
#include "connectables/objectfactory.h"
#include "connectables/objects/container.h"
#include "mainhost.h"
#include "models/programsmodel.h"

ComAddCable::ComAddCable(MainHost *myHost,
                         const MetaPin &outInfo,
                         const MetaPin &inInfo,
                         QUndoCommand  *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    outInfo(outInfo),
    inInfo(inInfo),
    currentGroup(0),
    currentProg(0)
{
    setText(QObject::tr("Add cable"));

    currentGroup = myHost->programsModel->GetCurrentMidiGroup();
    currentProg =  myHost->programsModel->GetCurrentMidiProg();

    if(outInfo.Direction()==Directions::Input) {
        MetaPin tmp(outInfo);
        this->outInfo=MetaPin(inInfo);
        this->inInfo=MetaPin(tmp);
    }
}

void ComAddCable::undo ()
{
    myHost->programsModel->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( inInfo.ContainerId() ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;

    myHost->objFactory->UpdatePinInfo(outInfo);
    myHost->objFactory->UpdatePinInfo(inInfo);
    static_cast<Connectables::Container*>(cntPtr.data())->UserRemoveCable(outInfo,inInfo);
}

void ComAddCable::redo ()
{
    myHost->programsModel->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( inInfo.ContainerId() ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;

    myHost->objFactory->UpdatePinInfo(outInfo);
    myHost->objFactory->UpdatePinInfo(inInfo);
    static_cast<Connectables::Container*>(cntPtr.data())->UserAddCable(outInfo,inInfo);
}
