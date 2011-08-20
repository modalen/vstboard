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
#include "events.h"



//MetaData::MetaData() :
//    objType(MetaTypes::ND),
//    objId(0),
//    objName(""),
//    parentId(0),
//    containerId(0),
//    parentObjectId(0),
//    transporter(0)
//{
//}

//MetaData::MetaData(const MetaTypes::Enum type) :
//    objType(type),
//    objId(0),
//    objName(""),
//    parentId(0),
//    containerId(0),
//    parentObjectId(0),
//    transporter(0)
//{
//}

//MetaData::MetaData(const QByteArray &b)
//{
//    QDataStream stream(b);
//    fromStream(stream);
//}

//ObjectInfo::ObjectInfo() :
//    MetaData(),
//    parentInfo(0),
//    containerInfo(0)
//{
//}

//ObjectInfo::ObjectInfo(const ObjectInfo &c) :
//    MetaData(c)
//{
//    parentInfo=c.parentInfo;
//    containerInfo=c.containerInfo;
//}

//ObjectInfo::ObjectInfo(const MetaData &c, MetaTransporter *transporter) :
//    MetaData(c),
//    parentInfo(0),
//    containerInfo(0)
//{
//    SetTransporter(transporter);
//}

//ObjectInfo::~ObjectInfo()
//{
//    if(parentInfo) {
//        parentInfo->childrenInfo.removeAll(this);
//        parentInfo=0;
//    }
//    foreach(ObjectInfo *info, childrenInfo) {
//        info->SetParent(0);
//    }
//}



//QString MetaData::toString() const
//{
//    return QString("type:%1 id:%2 name:%3 parent:%4 container:%5 obj:%6")
//            .arg(objType)
//            .arg(objId)
//            .arg(objName)
//            .arg(parentId)
//            .arg(containerId)
//            .arg(parentObjectId);
//}

//QString MetaData::toStringFull() const
//{
//    QString str(toString());
//    str.append( data.toString() );
//    QMap<metaT::Enum,QVariant>::iterator i = listInfos.begin();
//    while(i != listInfos.end()) {
//        str.append( QString(" %1:%2").arg(i.key()).arg(i.value().toString()));
//        ++i;
//    }
//    return str;
//}

//QDataStream & MetaData::toStream(QDataStream& stream) const
//{
//    stream << (quint8)objType;
//    stream << objId;
//    stream << objName;
//    stream << parentId;
//    stream << containerId;
//    stream << parentObjectId;

//    stream << data;
//    mutexListInfos.lock();
//    stream << (quint16)listInfos.size();
//    QMap<metaT::Enum,QVariant>::iterator i = listInfos.begin();
//    while(i != listInfos.end()) {
//        stream << (quint16)i.key();
//        stream << i.value();
//        ++i;
//    }
//    mutexListInfos.unlock();
//    return stream;
//}

//QDataStream & MetaData::fromStream(QDataStream& stream)
//{
//    quint8 type;
//    stream >> type;
//    objType=(MetaTypes::Enum)type;
//    stream >> objId;
//    stream >> objName;
//    stream >> parentId;
//    stream >> containerId;
//    stream >> parentObjectId;

//    stream >> data;

//    quint16 nb;
//    stream >> nb;

//    mutexListInfos.lock();
//    for(int i=0; i<nb; i++) {
//        quint16 id;
//        QVariant val;
//        stream >> id;
//        stream >> val;
//        listInfos.insert((metaT::Enum)id,val);
//    }
//    mutexListInfos.unlock();

//    return stream;
//}

//QDataStream & operator<< (QDataStream& stream, const MetaData& objInfo)
//{
//   return objInfo.toStream(stream);
//}

//QDataStream & operator>> (QDataStream& stream, MetaData& objInfo)
//{
//    return objInfo.fromStream(stream);
//}

//QDataStream & ObjectContainerAttribs::toStream (QDataStream& out) const
//{
//    out << position;
//    out << editorVisible;
//    out << editorPosition;
//    out << editorSize;
//    out << editorHScroll;
//    out << editorVScroll;

//    return out;
//}

//QDataStream & ObjectContainerAttribs::fromStream (QDataStream& in)
//{
//    in >> position;
//    in >> editorVisible;
//    in >> editorPosition;
//    in >> editorSize;
//    in >> editorHScroll;
//    in >> editorVScroll;
//    return in;
//}

//QDataStream & operator<< (QDataStream & out, const ObjectContainerAttribs& value)
//{
//    return value.toStream(out);
//}

//QDataStream & operator>> (QDataStream & in, ObjectContainerAttribs& value)
//{
//    return value.fromStream(in);
//}
