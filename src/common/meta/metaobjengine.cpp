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

#include "metaobjengine.h"
#include "metatransporter.h"
#include "events.h"

MetaObjEngine::MetaObjEngine(const MetaType::Enum type, MetaTransporter *transport) :
    MetaData(),
    type(type),
    objId(0),
    objName("nd"),
    parentId(0),
    containerId(0),
    parentObjectId(0),
    transporter(transport),
    parentInfo(0),
    containerInfo(0)
{
    SetMeta<MetaType::Enum>(metaT::ObjType, type);
}

MetaObjEngine::MetaObjEngine(const MetaData &data, MetaTransporter *transport) :
    MetaData(data),
    transporter(transport),
    parentInfo(0),
    containerInfo(0)
{
    type = data.GetMetaData<MetaTypes::Enum>(metaT::ObjType);
    objId = data.GetMetaData<quint32>(metaT::ObjId);
    ObjName = data.GetMetaData<QString>(metaT::ObjName);
    parentId = data.GetMetaData<quint32>(metaT::ParentId);
    containerId = data.GetMetaData<quint32>(metaT::ContainerId);
    parentObjectId = data.GetMetaData<quint32>(metaT::ParentObjId);
}

MetaObjEngine::~MetaObjEngine()
{
    if(parentInfo) {
        parentInfo->childrenInfo.removeAll(this);
        parentInfo=0;
    }
    foreach(ObjectInfo *info, childrenInfo) {
        info->SetParent(0);
    }
}

void MetaObjEngine::SetContainer(MetaObjEngine *container)
{
    //a bridge pin is located in the parent container, it allows communication between a container and its parent container
    if(GetMetaData<bool>(metaT::Bridge))
        container = container->ContainerInfo();

    containerInfo = container;

    if(container)
        SetContainerId(container->ObjId());
    else
        SetContainerId(0);

    if(Type()!=MetaType::container) {
        foreach(MetaObjEngine *info, childrenInfo) {
            info->SetContainer(container);
        }
    }
}

void MetaObjEngine::SetParent(MetaObjEngine *parent)
{
    //if the object already has a parent, remove itself from the parent
    if(parentInfo) {
        parentInfo->childrenInfo.removeAll(this);
    }

    parentInfo = parent;

    if(parentInfo) {
        SetParentId(parent->ObjId());
        parentInfo->childrenInfo << this;

        SetTransporter(parent->Transporter());

        //the new parent can be the parentObject, or else get the parentObject from the new parent
        if(parentInfo->Type()==MetaType::object || parentInfo->Type()==MetaType::container || parentInfo->Type()==MetaType::bridge)
            SetParentObjectId(ParentId());
        else
            SetParentObjectId(ParentInfo()->ParentObjectId());
    } else {
        SetParentId(0);
        SetTransporter(0);
        SetParentObjectId(0);
        SetContainerId(0);
        containerInfo=0;
    }

    foreach(MetaObjEngine *info, childrenInfo) {
        info->SetParent(this);
    }
}

/*!
  Send the object to the view, with all the children
  */
void MetaObjEngine::AddToView()
{
    if(!ContainerId())
        return;

    if(GetMetaData<bool>(metaT::Hidden))
        return;

    Events::sendObj *event = new Events::sendObj( this , Events::typeNewObj);
    if(transporter) {
        transporter->PostEvent(event);
    }  else {
        LOG("transporter not set");
    }

    foreach(ObjectInfo *info, childrenInfo) {
        info->AddToView();
    }
}

/*!
  Remove the object from the view
  */
void MetaObjEngine::RemoveFromView()
{
    Events::delObj *event = new Events::delObj(ObjId());
    if(transporter)
        transporter->PostEvent(event);
    else {
        LOG("transporter not set")
    }
}

/*!
  Send the object, not the children
  */
void MetaObjEngine::UpdateView()
{
    if(!ContainerId())
        return;

    if(data.GetMetaData<bool>(metaT::Hidden))
        return;

    Events::sendObj *event = new Events::sendObj(this, Events::typeUpdateObj);
    if(transporter)
        transporter->PostEvent(event);
    else {
        LOG("transporter not set");
    }
}



