#include "comaddobject.h"
#include "mainhost.h"
#include "connectables/objects/container.h"
#include "comremoveobject.h"
#include "models/programsmodel.h"

ComAddObject::ComAddObject(MainHost *myHost,
                           const MetaInfo &objInfo,
                           const MetaInfo &targetInfo,
                           InsertionType::Enum insertType,
                           QUndoCommand  *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    objectInfo(objInfo),
    targetInfo(targetInfo),
    insertType(insertType),
    currentGroup(0),
    currentProg(0)
{
    setText(QObject::tr("Add object"));

    currentGroup = myHost->programsModel->GetCurrentMidiGroup();
    currentProg =  myHost->programsModel->GetCurrentMidiProg();
}

void ComAddObject::undo ()
{
    myHost->programsModel->ChangeProgNow(currentGroup,currentProg);

    //get the object
    QSharedPointer<Connectables::Object> obj = myHost->objFactory->GetObjectFromId( objectInfo.ObjId() );
    if(!obj)
        return;

    //get the container
    QSharedPointer<Connectables::Container> container = myHost->objFactory->GetObjectFromId( objectInfo.ContainerId() ).staticCast<Connectables::Container>();
    if(!container)
        return;

    QDataStream stream(&objState, QIODevice::ReadWrite);
    obj->SaveProgram();
    obj->toStream( stream );
    obj->GetContainerAttribs(attr);

    //remove the object
    container->UserParkObject(obj);

    //add the replaced object
    if(insertType==InsertionType::Replace) {
        QSharedPointer<Connectables::Object> target = myHost->objFactory->GetObjectFromId( targetInfo.ObjId() );
        if(!target) {
            //the target has been deleted, create it back
            target = myHost->objFactory->NewObject( targetInfo );
        }
        if(target) {
            if(targetState.size()!=0) {
                QDataStream stream(&targetState, QIODevice::ReadWrite);
                target->fromStream( stream );
                targetState.resize(0);
            }

            container->UserAddObject(target);
            if(targetAttr.position!=QPointF(0.0f,0.0f)) {
                target->SetContainerAttribs(targetAttr);
            }
        }
    }

    //remove cables added at creation
    QPair<MetaInfo,MetaInfo>pair;
    foreach( pair, listAddedCables) {
        myHost->objFactory->UpdatePinInfo( pair.first );
        myHost->objFactory->UpdatePinInfo( pair.second );
        container->UserRemoveCable(pair);
    }
    //add cables removed at creation
    foreach( pair, listRemovedCables) {
        myHost->objFactory->UpdatePinInfo( pair.first );
        myHost->objFactory->UpdatePinInfo( pair.second );
        container->UserAddCable(pair);
    }

    listAddedCables.clear();
    listRemovedCables.clear();
}

void ComAddObject::redo ()
{
    myHost->programsModel->ChangeProgNow(currentGroup,currentProg);

    //get the object
    QSharedPointer<Connectables::Object> obj = myHost->objFactory->GetObjectFromId( objectInfo.ObjId() );
    if(!obj) {
        //object was deleted, create a new one
        if(objectInfo.ObjId()==0)
            objectInfo.SetObjId(MetaInfo::GetNextId());

        //add a temporary object on the view
        ObjectInfo tmpMeta(objectInfo,myHost);
        tmpMeta.AddToView();

        obj = myHost->objFactory->NewObject( objectInfo );
    }
    if(!obj)
        return;

    setText(QObject::tr("Add %1").arg(obj->objectName()));
    objectInfo = obj->info();

    //get the container
    QSharedPointer<Connectables::Container> container = myHost->objFactory->GetObjectFromId( objectInfo.ContainerId() ).staticCast<Connectables::Container>();
    if(!container)
        return;

    //get the target
    QSharedPointer<Connectables::Object> target = myHost->objFactory->GetObjectFromId( targetInfo.ObjId() );
    if(target) {
        targetInfo = target->info();
        QDataStream stream(&targetState, QIODevice::ReadWrite);
        target->SaveProgram();
        target->toStream( stream );
        target->GetContainerAttribs(targetAttr);
    }

    if(objState.size()!=0) {
        QDataStream stream(&objState, QIODevice::ReadWrite);
        obj->fromStream( stream );
        objState.resize(0);
    }

    //add the object to the container
    container->UserAddObject( obj, insertType, &listAddedCables, &listRemovedCables, target );

    if(attr.position!=QPointF(0.0f,0.0f)) {
        obj->SetContainerAttribs(attr);
    }
}

void ComAddObject::ReloadObject(const MetaInfo &info)
{
    objectInfo = info;
    redo();
}
