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
#include "mainhost.h"
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
Pin::Pin(Object *parent, MetaInfo &info) :
    QObject(parent),
    ObjectInfo(info),
    parent(parent),
    closed(false),
    valueChanged(false),
    nameCanChange(false)
{
    SetType(MetaTypes::pin);
    setObjectName(Name());

    if(!Meta(MetaInfos::Hidden).toBool())
        SetVisible(true);
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
        SetMeta(MetaInfos::Bridge, true);
    else
        DelMeta(MetaInfos::Bridge);
}

/*!
  Show or hide the pin
  \param vis true if visible
  */
void Pin::SetVisible(bool visible)
{
    QMutexLocker l(&objMutex);

    if(closed)
        return;

    if(!ContainerId())
        return;

    if(visible) {
        DelMeta(MetaInfos::Hidden);
        connect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
                this,SLOT(updateView()),
                Qt::UniqueConnection);

        AddToView(parent->getHost());
//        UpdateView(parent->getHost());

    } else {

        SetMeta(MetaInfos::Hidden,true);
        //remove cables from pin
        if(ContainerInfo()) {
            QSharedPointer<Object> cnt = parent->getHost()->objFactory->GetObjectFromId(ContainerInfo()->ObjId());
            if(!cnt.isNull()) {
                static_cast<Container*>(cnt.data())->UserRemoveCableFromPin(info());
            }
        }

        //remove pin
        if(Meta(MetaInfos::Media).toInt()!=MediaTypes::Bridge) {
            disconnect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
                    this,SLOT(updateView()));
        }

        RemoveFromView(parent->getHost());
//        parent->getHost()->PostEvent( new Events::delObj(info().ObjId()) );
    }


}

/*!
  Update view values
  */
void Pin::updateView()
{
    QMutexLocker l(&objMutex);

    if(closed || Meta(MetaInfos::Hidden).toBool()) {
        return;
    }

    float newVu = GetValue();
    if(!valueChanged)
        return;

    valueChanged=false;

    SetMeta(MetaInfos::Value,newVu);
    UpdateView(parent->getHost());
}
