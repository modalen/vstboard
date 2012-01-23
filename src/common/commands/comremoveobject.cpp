#include "comremoveobject.h"
#include "mainhost.h"
#include "connectables/container.h"
#include "models/programsmodel.h"

ComRemoveObject::ComRemoveObject( MainHost *myHost,
                                  const QSharedPointer<Connectables::Object> &objPtr,
                                  RemoveType::Enum removeType,
                                  QUndoCommand  *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    removeType(removeType),
    objectInfo(objectInfo),
    currentGroup(0),
    currentProg(0)
{
    ContainerPtr = myHost->objFactory->GetObjectFromId( objPtr->GetContainerId() ).staticCast<Connectables::Container>();
    if(!ContainerPtr)
        return;

    setText(QObject::tr("Remove %1").arg(objPtr->objectName()));

    objectInfo = objPtr->info();

    currentGroup = myHost->programsModel->GetCurrentMidiGroup();
    currentProg =  myHost->programsModel->GetCurrentMidiProg();
}

void ComRemoveObject::undo ()
{
    myHost->programsModel->ChangeProgNow(currentGroup,currentProg);

    //get the container
    QSharedPointer<Connectables::Container> container = ContainerPtr.toStrongRef();
    if(!container)
        return;

    //get the object
    QSharedPointer<Connectables::Object> obj = myHost->objFactory->GetObjectFromId( objectInfo.forcedObjId );
    if(!obj) {
        //object was deleted, create a new one
        obj = myHost->objFactory->NewObject( objectInfo, container->GetIndex() );
    }
    if(!obj)
        return;

    objectInfo = obj->info();



    QDataStream stream(&objState, QIODevice::ReadWrite);
    obj->fromStream( stream );
    objState.resize(0);

    container->UserAddObject( obj );

    obj->SetContainerAttribs(attr);

    //remove cables added at creation
    QPair<ConnectionInfo,ConnectionInfo>pair;
    foreach( pair, listAddedCables) {
        container->UserRemoveCable(pair);
    }
    //add cables removed at creation
    foreach( pair, listRemovedCables) {
        container->UserAddCable(pair);
    }

    listAddedCables.clear();
    listRemovedCables.clear();
}

void ComRemoveObject::redo()
{
    myHost->programsModel->ChangeProgNow(currentGroup,currentProg);

    //get the object
    QSharedPointer<Connectables::Object> obj = myHost->objFactory->GetObjectFromId( objectInfo.forcedObjId );
    if(!obj)
        return;

    //get the container
    QSharedPointer<Connectables::Container> container = ContainerPtr.toStrongRef();
    if(!container)
        return;

    QDataStream stream(&objState, QIODevice::ReadWrite);
    obj->SaveProgram();
    obj->toStream( stream );
    obj->GetContainerAttribs(attr);

    //remove the object
    container->UserParkObject(obj,removeType,&listAddedCables,&listRemovedCables);

}
