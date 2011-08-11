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

#include "globals.h"

namespace MetaTypes {
    enum Enum {
        ND,
        object,
        container,
        bridge,
        listPin,
        pin,
        cable,
        cursor
    };
}

namespace MetaInfos {
    enum Enum {
        ObjType,
        Direction,
        Media,
        Filename,
        LimitType,
        id,
        name,
        apiId,
        apiName,
        duplicateNamesCounter,
        nbInputs,
        nbOutputs
    };
}

namespace ObjTypes {
    enum Enum {
        ND,
        Dummy,
        VstPlugin,
        HostController,
        MidiToAutomation,
        MidiSender,
        Script,
        AudioInterface,
        VstAutomation,
        MidiInterface
    };
}

namespace MediaTypes {
    enum Enum {
        ND,
        Audio,
        Midi,
        Parameter,
        Bridge
    };
}

namespace Directions {
    enum Enum {
        ND,
        Input,
        Output,
        Send,
        Return
    };
}

namespace FixedObjIds {
    enum Enum {
        ND,
        mainContainer,
        hostContainer,
        hostContainerIn,
        hostContainerOut,
        hostContainerSend,
        hostContainerReturn,
        projectContainer,
        projectContainerIn,
        projectContainerOut,
        projectContainerSend,
        projectContainerReturn,
        programContainer,
        programContainerIn,
        programContainerOut,
        programContainerSend,
        programContainerReturn,
        groupContainer,
        groupContainerIn,
        groupContainerOut,
        groupContainerSend,
        groupContainerReturn,
        parkingContainer,
        noContainer=65535
    };
}

namespace LimitTypes {
    enum Enum {
        Min,
        Max
    };
}

class ObjectInfo
{
public:



    ObjectInfo();
    ObjectInfo( MetaTypes::Enum metaType, ObjTypes::Enum objType=ObjTypes::ND, int id=0, QString name="");
    ObjectInfo(const ObjectInfo &c);

    MetaTypes::Enum metaType;
    quint32 objId;
    quint32 parentId;
    QMap<MetaInfos::Enum,QVariant>listInfos;

    inline ObjectInfo & info() {return *this;}


    QDataStream & toStream(QDataStream& stream) const;
    QDataStream & fromStream(QDataStream& stream);

    quint32 id;
    QString name;
    QString filename;
    quint16 inputs;
    quint16 outputs;
    quint16 duplicateNamesCounter;
    quint8 api;
    QString apiName;
};

Q_DECLARE_METATYPE(ObjectInfo);

QDataStream & operator<< (QDataStream& stream, const ObjectInfo& objInfo);
QDataStream & operator>> (QDataStream& stream, ObjectInfo& objInfo);

class ObjectContainerAttribs
{
public:
    ObjectContainerAttribs() :
        position(QPointF(0.0f,0.0f)),
        editorVisible(false),
        editorPosition(QPoint(0.0f,0.0f)),
        editorSize(QSize(0,0)),
        editorHScroll(0),
        editorVScroll(0)
        {}

    QPointF position;
    bool editorVisible;
    QPoint editorPosition;
    QSize editorSize;
    quint16 editorHScroll;
    quint16 editorVScroll;

    QDataStream & toStream (QDataStream &) const;
    QDataStream & fromStream (QDataStream &);
};

QDataStream & operator<< (QDataStream & out, const ObjectContainerAttribs& value);
QDataStream & operator>> (QDataStream & in, ObjectContainerAttribs& value);

inline bool operator==(const ObjectContainerAttribs &c1, const ObjectContainerAttribs &c2)
{
    if(c1.position != c2.position)
        return false;
    if(c1.editorVisible != c2.editorVisible)
        return false;
    if(c1.editorPosition != c2.editorPosition)
        return false;
    if(c1.editorSize != c2.editorSize)
        return false;
    if(c1.editorHScroll != c2.editorHScroll)
        return false;
    if(c1.editorVScroll != c2.editorVScroll)
        return false;

    return true;
}

inline bool operator!=(const ObjectContainerAttribs &c1, const ObjectContainerAttribs &c2)
{
    return !(c1==c2);
}

#endif // OBJECTINFO_H
