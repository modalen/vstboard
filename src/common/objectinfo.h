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

#ifndef OBJECTINFO_H
#define OBJECTINFO_H

//#include "precomp.h"
//#include "debugmutex.h"
//#include <QDebug>

//class ObjectInfo;
//class MetaData;

//typedef QMultiMap < MetaData, MetaData > mapCables;

//class MetaData
//{
//    public:
//    MetaData();
////    MetaData(const MetaData &c) {
////        *this=c;
////    }
//    MetaData(const MetaTypes::Enum type);
//    MetaData(const QByteArray &b);




//    QString toStringFull() const;

//    QDataStream & toStream(QDataStream& stream) const;
//    QDataStream & fromStream(QDataStream& stream);

//    MetaData & operator =(const MetaData &c) {
//        objType=c.objType;
//        objId=c.objId;
//        parentId=c.parentId;
//        parentObjectId=c.parentObjectId;
//        containerId=c.containerId;
//        objName=c.objName;
//        transporter=c.transporter;
//        data=c.data;
////        listInfos=c.listInfos;
//        return *this;
//    }

//};


//Q_DECLARE_METATYPE(MetaData);

//QDataStream & operator<< (QDataStream& stream, const MetaData& info);
//QDataStream & operator>> (QDataStream& stream, MetaData& info);
//inline bool operator==(const MetaData &c1, const MetaData &c2) { return c1.ObjId() == c2.ObjId(); }
//inline bool operator<(const MetaData &c1, const MetaData &c2) { return c1.ObjId() < c2.ObjId(); }

//class ObjectInfo : public MetaData
//{
//public:
//    ObjectInfo();
//    ObjectInfo(const ObjectInfo &c);
//    virtual ~ObjectInfo();
////    ObjectInfo( MetaTypes::Enum metaT::ype, ObjTypes::Enum objType=ObjTypes::ND, int id=0, QString objName="");
//    ObjectInfo(const MetaData &c, MetaTransporter *transporter);



//private:

//};



//class ObjectContainerAttribs
//{
//public:
//    ObjectContainerAttribs() :
//        position(QPointF(0.0f,0.0f)),
//        editorVisible(false),
//        editorPosition(QPoint(0.0f,0.0f)),
//        editorSize(QSize(0,0)),
//        editorHScroll(0),
//        editorVScroll(0)
//        {}

//    QPointF position;
//    bool editorVisible;
//    QPoint editorPosition;
//    QSize editorSize;
//    quint16 editorHScroll;
//    quint16 editorVScroll;

//    QDataStream & toStream (QDataStream &) const;
//    QDataStream & fromStream (QDataStream &);
//};

//QDataStream & operator<< (QDataStream & out, const ObjectContainerAttribs& value);
//QDataStream & operator>> (QDataStream & in, ObjectContainerAttribs& value);

//inline bool operator==(const ObjectContainerAttribs &c1, const ObjectContainerAttribs &c2)
//{
//    if(c1.position != c2.position)
//        return false;
//    if(c1.editorVisible != c2.editorVisible)
//        return false;
//    if(c1.editorPosition != c2.editorPosition)
//        return false;
//    if(c1.editorSize != c2.editorSize)
//        return false;
//    if(c1.editorHScroll != c2.editorHScroll)
//        return false;
//    if(c1.editorVScroll != c2.editorVScroll)
//        return false;

//    return true;
//}

//inline bool operator!=(const ObjectContainerAttribs &c1, const ObjectContainerAttribs &c2)
//{
//    return !(c1==c2);
//}

#endif // OBJECTINFO_H
