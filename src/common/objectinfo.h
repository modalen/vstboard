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
#include "models/hostmodel.h"

namespace MetaTypes {
    enum Enum {
        ND,
        object,
        container,
        bridge,
        listPin,
        pin,
        cable
    };
}

namespace MetaInfos {
    enum Enum {
        ND,
        ObjType,
        Direction,
        Media,
        Filename,
        devId,
        devName,
        apiId,
        apiName,
        duplicateNamesCounter,
        nbInputs,
        nbOutputs,
        errorMessage,
        PinNumber,
        Hidden,
        Removable,
        Bridge,
        Value,
        StepSize,
        Dirty,
        DoublePrecision,
        Position,
        bankFile,
        programFile,
        displayedText,
        EditorVisible,
        EditorSize,
        EditorPosition,
        EditorVScroll,
        EditorHScroll,
        LimitInMin,
        LimitInMax,
        LimitOutMin,
        LimitOutMax,
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

class ObjectInfo;
typedef QMultiMap < ObjectInfo, ObjectInfo > mapCables;

class MainHost;
class ObjectInfo;
class MetaInfo
{
    public:
    MetaInfo();
    MetaInfo(const MetaTypes::Enum type);

    inline MetaTypes::Enum Type() const {
        return objType;
    }
    inline void SetType(const MetaTypes::Enum type) {
        objType=type;
    }

    inline void SetObjId(const quint32 id) {
        objId = id;
    }
    inline const quint32 ObjId() const {
        return objId;
    }

    inline void SetParentId(const quint32 id) {
        parentId = id;
    }
    inline quint32 ParentId() const {
        return parentId;
    }

    inline void SetContainerId(const quint32 id) {
        containerId = id;
    }
    inline quint32 ContainerId() const {
        return containerId;
    }

    inline void SetParentObjectId(const quint32 id) {
        parentObjectId = id;
    }
    inline quint32 ParentObjectId() const {
        return parentObjectId;
    }

    inline void SetName(const QString &name) {
        if(!name.isEmpty())
            objName = name;
    }
    inline const QString & Name() const {
        return objName;
    }

    inline const QVariant Meta(MetaInfos::Enum inf) const {
        return listInfos.value(inf);
    }
    inline void SetMeta(MetaInfos::Enum inf, const QVariant &val) {
        listInfos[inf]=val;
        if(model)
            model->valueChanged(*this,inf,val);
    }
    inline void DelMeta(MetaInfos::Enum inf) {
        listInfos.remove(inf);
    }

    inline void SetModel(HostModel *m) {
        model = m;
    }
//    inline HostModel *Model() const {
//        return model;
//    }

    bool DropMime(const QMimeData *data, InsertionType::Enum insertType = InsertionType::NoInsertion) {
        if(!model)
            return false;
        return model->dropMime(data,MetaInfo(*this),insertType);
    }

    inline const MetaInfo & info() const {return *this;}

    bool CanConnectTo(const MetaInfo &c) const;

    QString toString() const;
    QString toStringFull() const;

    QDataStream & toStream(QDataStream& stream) const;
    QDataStream & fromStream(QDataStream& stream);

private:
    MetaTypes::Enum objType;
    quint32 objId;
    quint32 parentId;
    quint32 parentObjectId;
    quint32 containerId;
    QString objName;
    QMap<MetaInfos::Enum,QVariant>listInfos;
    HostModel *model;

};


Q_DECLARE_METATYPE(MetaInfo);

QDataStream & operator<< (QDataStream& stream, const MetaInfo& info);
QDataStream & operator>> (QDataStream& stream, MetaInfo& info);


class ObjectInfo : public MetaInfo
{
public:
    ObjectInfo();
    virtual ~ObjectInfo();
//    ObjectInfo( MetaTypes::Enum metaType, ObjTypes::Enum objType=ObjTypes::ND, int id=0, QString objName="");
    ObjectInfo(const MetaInfo &c);

    inline ObjectInfo * ParentInfo() const {
        return parentInfo;
    }

    inline ObjectInfo * ContainerInfo() const {
        return containerInfo;
    }

    void SetParent(ObjectInfo *parent);
    virtual void SetContainer(ObjectInfo *container);

    void AddToView(MainHost *myHost);
    void RemoveFromView(MainHost *myHost);
    void UpdateView(MainHost *myHost);

private:
    ObjectInfo* parentInfo;
    ObjectInfo* containerInfo;
    QList<ObjectInfo*>childrenInfo;
};

inline bool operator==(const ObjectInfo &c1, const ObjectInfo &c2) {
    return c1.ObjId() == c2.ObjId();
}

inline bool operator<(const ObjectInfo &c1, const ObjectInfo &c2) {
    return c1.ObjId() < c2.ObjId();
}

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
