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

#include "objectinfo.h"
#include "mainhost.h"
#include "events.h"

ObjectInfo::ObjectInfo() :
    metaType(MetaTypes::ND),
    objId(0),
    objName(""),
    parentInfo(0),
    containerInfo(0)
{
}

ObjectInfo::ObjectInfo( MetaTypes::Enum nodeType, ObjTypes::Enum objType, int id, QString name) :
    metaType(nodeType),
    objId(id),
    objName(name),
    parentInfo(0),
    containerInfo(0)
{
    if(objType)
        SetMeta(MetaInfos::ObjType,objType);
}

ObjectInfo::ObjectInfo(const ObjectInfo &c)
{
    *this = c;
    if(parentInfo)
        parentInfo->childrenInfo << this;
}

ObjectInfo::~ObjectInfo()
{
    if(parentInfo) {
        parentInfo->childrenInfo.removeAll(this);
        parentInfo=0;
    }
}

void ObjectInfo::UpdateView(MainHost *myHost)
{
    Events::newObj *event = new Events::newObj(info());
    myHost->PostEvent(event);

    foreach(ObjectInfo *info, childrenInfo) {
        info->UpdateView(myHost);
    }
}


void ObjectInfo::SetContainer(ObjectInfo *container) {
    containerInfo = container;

    if(container)
        containerId = container->ObjId();
    else
        containerId = 0;

    if(metaType!=MetaTypes::container) {
        foreach(ObjectInfo *info, childrenInfo) {
            info->SetContainer(container);
        }
    }
}

void ObjectInfo::SetParent(ObjectInfo *parent)
{
    if(parentInfo) {
        parentInfo->childrenInfo.removeAll(this);
    }

    parentInfo = parent;
    parentId = parent->ObjId();
    parentObjectId = ParentObjectInfo()->ObjId();

    if(parentInfo) {
        parentInfo->childrenInfo << this;

        if(parentInfo->Meta()==MetaTypes::container)
            SetContainer(parentInfo);
        else
            SetContainer(parentInfo->ContainerInfo());
    } else {
        SetContainer(0);
    }
}


bool ObjectInfo::CanConnectTo(const ObjectInfo &c) const
{

    //don't connect object to itself
//    if(objId == c.objId)
//        return false;

    //must be the same type (audio/midi/automation) or a bridge pin
    if(Meta(MetaInfos::Media)!=MediaTypes::Bridge && c.Meta(MetaInfos::Media)!=MediaTypes::Bridge && Meta(MetaInfos::Media) != c.Meta(MetaInfos::Media))
        return false;


    //must be opposite directions
    if(Meta(MetaInfos::Direction) == c.Meta(MetaInfos::Direction))
        return false;

    int cntA = ContainerInfo()->ObjId();
    int cntB = c.ContainerInfo()->ObjId();

    //if it's a bridge : get the container's container id
    if(Meta(MetaInfos::Media)==MediaTypes::Bridge)
        cntA = ContainerInfo()->ContainerInfo()->ObjId();

    if(c.Meta(MetaInfos::Media)!=MediaTypes::Bridge)
        cntB = c.ContainerInfo()->ContainerInfo()->ObjId();

    //must be in the same container
    if(cntA == cntB)
        return true;

    return false;
}

QDataStream & ObjectInfo::toStream(QDataStream& stream) const
{
    stream << (quint8)metaType;
    stream << objId;
    stream << objName;
    stream << parentInfo->ObjId();
    stream << containerInfo->ObjId();

    stream << (quint16)listInfos.size();
    QMap<MetaInfos::Enum,QVariant>::iterator i = listInfos.begin();
    while(i != listInfos.end()) {
        stream << (quint16)i.key();
        stream << i.value();
        ++i;
    }

    stream << (quint16)childrenInfo.size();
    foreach(ObjectInfo* o, childrenInfo) {
        stream << o->ObjId();
    }

//    stream << (quint8)metaType;
//    stream << (quint8)objType;
//    stream << id;
//    stream << name;
//    stream << filename;
//    stream << inputs;
//    stream << outputs;
//    stream << duplicateNamesCounter;
//    stream << apiName;
//    stream << api;
//    stream << objId;
    return stream;
}

QDataStream & ObjectInfo::fromStream(QDataStream& stream)
{
    quint8 type;
    stream >> type;
    metaType=(MetaTypes::Enum)type;
    stream >> objId;
    stream >> objName;
    quint32 id;
    stream >> id;
    stream >> id;

    quint16 nb;
    stream >> nb;
    for(int i=0; i<nb; i++) {
        quint16 id;
        QVariant val;
        stream >> id;
        stream >> val;
        listInfos.insert((MetaInfos::Enum)id,val);
        ++i;
    }

    stream >> nb;
    for(int i=0; i<nb; i++) {
        quint32 id;
        stream >> id;
    }

//    stream >> (quint8&)metaType;
//    stream >> (quint8&)objType;
//    stream >> id;
//    stream >> name;
//    stream >> filename;
//    stream >> inputs;
//    stream >> outputs;
//    stream >> duplicateNamesCounter;
//    stream >> apiName;
//    stream >> api;
//    stream >> objId;
    return stream;
}

QDataStream & operator<< (QDataStream& stream, const ObjectInfo& objInfo)
{
   return objInfo.toStream(stream);
}

QDataStream & operator>> (QDataStream& stream, ObjectInfo& objInfo)
{
    return objInfo.fromStream(stream);
}

QDataStream & ObjectContainerAttribs::toStream (QDataStream& out) const
{
    out << position;
    out << editorVisible;
    out << editorPosition;
    out << editorSize;
    out << editorHScroll;
    out << editorVScroll;

    return out;
}

QDataStream & ObjectContainerAttribs::fromStream (QDataStream& in)
{
    in >> position;
    in >> editorVisible;
    in >> editorPosition;
    in >> editorSize;
    in >> editorHScroll;
    in >> editorVScroll;
    return in;
}

QDataStream & operator<< (QDataStream & out, const ObjectContainerAttribs& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream & in, ObjectContainerAttribs& value)
{
    return value.fromStream(in);
}
