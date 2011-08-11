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

#include "precomp.h"
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
        devId,
        devName,
        apiId,
        apiName,
        duplicateNamesCounter,
        nbInputs,
        nbOutputs,
        errorMessage,
        PinNumber,
        Visible,
        Removable
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

class ObjectInfo;
typedef QMultiMap < ObjectInfo, ObjectInfo > mapCables;

class MainHost;
class ObjectInfo
{
public:
    ObjectInfo();
    virtual ~ObjectInfo();
    ObjectInfo( MetaTypes::Enum metaType, ObjTypes::Enum objType=ObjTypes::ND, int id=0, QString objName="");
    ObjectInfo(const ObjectInfo &c);

    inline MetaTypes::Enum Meta() const {
        return metaType;
    }

    inline void SetObjId(const quint32 id) {
        objId = id;
    }
    inline const quint32 ObjId() const {
        return objId;
    }

    inline ObjectInfo * ParentInfo() const {
        return parentInfo;
    }

    inline ObjectInfo * ContainerInfo() const {
        return containerInfo;
    }

    inline quint32 ParentId() const {
        return parentId;
    }
    inline quint32 ParentObjectId() const {
        return parentObjectId;
    }
    inline quint32 ContainerId() const {
        return containerId;
    }

    inline void SetName(const QString &name) {
        objName = name;
    }
    inline const QString & Name() const {
        return objName;
    }

    inline void SetMeta(MetaInfos::Enum inf, const QVariant &val) {
        listInfos[inf]=val;
    }
    inline const QVariant Meta(MetaInfos::Enum inf) const {
        return listInfos.value(inf);
    }
    inline void DelMeta(MetaInfos::Enum inf) {
        listInfos.remove(inf);
    }

    void SetParent(ObjectInfo *parent);
    virtual void SetContainer(ObjectInfo *container);

    inline ObjectInfo & info() {return *this;}
    void UpdateView(MainHost *myHost);

    bool CanConnectTo(const ObjectInfo &c) const;

    QDataStream & toStream(QDataStream& stream) const;
    QDataStream & fromStream(QDataStream& stream);

protected:
    MetaTypes::Enum metaType;

private:
    quint32 objId;
    quint32 parentId;
    quint32 parentObjectId;
    quint32 containerId;
    QString objName;
    ObjectInfo* parentInfo;
    ObjectInfo* containerInfo;
    QMap<MetaInfos::Enum,QVariant>listInfos;
    QList<ObjectInfo*>childrenInfo;

    const ObjectInfo * ParentObjectInfo() const {
        if(metaType==MetaTypes::object || metaType==MetaTypes::container || metaType==MetaTypes::bridge)
            return this;
        else return ParentInfo()->ParentObjectInfo();
    }
};

inline bool operator==(const ObjectInfo &c1, const ObjectInfo &c2) {
    return c1.ObjId() == c2.ObjId();
}

inline bool operator<(const ObjectInfo &c1, const ObjectInfo &c2) {
    return c1.ObjId() < c2.ObjId();
}

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
