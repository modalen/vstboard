/**************************************************************************
#    Copyright 2010-2011 Raphaël François
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "pin.h"
#include "../mainhost.h"
#include "events.h"

using namespace Connectables;

/*!
  \class Connectables::Pin
  \brief virtual pin
  */

/*!
  Constructor, used by PinsList with the help of Object::CreatePin
  \param parent pointer to the parent Object
  \param type PinType
  \param direction PinDirection
  \param number pin number in the list
  \param bridge true if this pin is a bridge
  */
Pin::Pin(Object *parent, ObjectInfo &info) :
    QObject(parent),
    ObjectInfo(info),
    value(.0f),
    stepSize(.1f),
    parent(parent),
    visible(false),
    closed(false),
    valueChanged(false)
{
}

/*!
  Destructor
  Hide and close
  */
Pin::~Pin()
{
    SetVisible(false);
    Close();
}

/*!
  Send a message to all the connected pins
  \param msgType PinMessage
  \param data data to send
  */
void Pin::SendMsg(const PinMessage::Enum msgType,void *data)
{
    parent->getHost()->SendMsg(info(),(PinMessage::Enum)msgType,data);
}

/*!
  Update the view with the new parent
  \param newParent
  */
void Pin::SetParentModelIndex(const QModelIndex &newParent)
{
    closed=false;

    //moving from another parent (when does it happen ?)
//    if(parentIndex.isValid() && parentIndex != newParent) {
//        SetVisible(false);
//    }

    parentIndex=newParent;
    SetVisible(visible);
}

/*!
  Close the pin before deletion
  \todo can be merged with destructor ?
  */
void Pin::Close()
{
    QMutexLocker l(&objMutex);
    if(parent && parent->getHost() && parent->getHost()->updateViewTimer )
        disconnect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
            this,SLOT(updateView()));
    parentIndex=QModelIndex();
    modelIndex=QModelIndex();
    closed=true;
}

/*!
  Set this pin as a bridge (will allow connection in the parent container)
  \param bridge true is it's a bridge
  */
void Pin::SetBridge(bool bridge)
{
    if(bridge)
        SetMeta(MetaInfos::Media, MediaTypes::Bridge);
    else
        DelMeta(MetaInfos::Media);
}

/*!
  Show or hide the pin
  \param vis true if visible
  */
void Pin::SetVisible(bool vis)
{
    QMutexLocker l(&objMutex);

    visible=vis;

    if(closed)
        return;

//    if(!parentIndex.isValid())
//        return;

    if(visible) {// && !modelIndex.isValid()) {



//        Events::newObj *event = new Events::newPin(objectName(),GetValue(),connectInfo,stepSize,listId);
//        parent->getHost()->PostEvent(event);

//        QStandardItem *item = new QStandardItem("pin");
//        item->setData(objectName(),Qt::DisplayRole);
//        item->setData(GetValue(),UserRoles::value);
//        item->setData( QVariant::fromValue(ObjectInfo(MetaTypes::pin)),UserRoles::objInfo);
//        item->setData(QVariant::fromValue(connectInfo),UserRoles::connectionInfo);
//        item->setData(stepSize,UserRoles::stepSize);

//        QStandardItem *parentItem = parent->getHost()->GetModel()->itemFromIndex(parentIndex);
//        parentItem->appendRow(item);
//        modelIndex = item->index();

//        connect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
//                this,SLOT(updateView()),
//                Qt::UniqueConnection);
    }

    if(!visible && modelIndex.isValid()) {
        //remove cables from pin
        QSharedPointer<Object> cnt = parent->getHost()->objFactory->GetObjectFromId(ContainerInfo()->ObjId());
        if(!cnt.isNull()) {
            static_cast<Container*>(cnt.data())->UserRemoveCableFromPin(info());
        }

        //remove pin
        if(Meta(MetaInfos::Media).toInt()!=MediaTypes::Bridge) {
            disconnect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
                    this,SLOT(updateView()));
        }

//        if(modelIndex.isValid())
//            parent->getHost()->GetModel()->removeRow(modelIndex.row(), modelIndex.parent());
        modelIndex=QModelIndex();
    }

//    MetaInfos::UpdateView();
}

///*!
//  Update the view model
//  Used when moved to a new container
//  */
//void Pin::UpdateModelNode()
//{
//    QStandardItem *item = parent->getHost()->GetModel()->itemFromIndex(modelIndex);
//    if(!item) {
//        LOG("item not found"<<modelIndex);
//        return;
//    }
//    item->setData(objectName(),Qt::DisplayRole);
//    item->setData(GetValue(),UserRoles::value);
//    item->setData(QVariant::fromValue(ObjectInfo(MetaTypes::pin)),UserRoles::objInfo);
//    item->setData(QVariant::fromValue(connectInfo),UserRoles::connectionInfo);
//    item->setData(stepSize,UserRoles::stepSize);
//    modelIndex = item->index();
//    if(connectInfo.type!=MediaTypes::Bridge) {
//        connect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
//                this,SLOT(updateView()),
//                Qt::UniqueConnection);
//    }
//}

/*!
  Update view values
  */
void Pin::updateView()
{
    QMutexLocker l(&objMutex);

    if(closed || !visible | !modelIndex.isValid()) {
        return;
    }

    if(!displayedText.isEmpty()) {
        parent->getHost()->GetModel()->setData(modelIndex, displayedText, Qt::DisplayRole);
        displayedText="";
    }

    float newVu = GetValue();
    if(!valueChanged)
        return;

    valueChanged=false;

    parent->getHost()->GetModel()->setData(modelIndex, newVu, UserRoles::value);


}
