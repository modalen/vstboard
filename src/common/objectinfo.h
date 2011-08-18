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
#include "mutexdebug.h"
#include <QDebug>

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

        INT_BEGIN = 1,
        ObjType,
        Direction,
        Media,
        BridgeMedia,
        devId,
        apiId,
        duplicateNamesCounter,
        nbInputs,
        nbOutputs,
        PinNumber,
        ValueStep,
        DefaultValueStep,
        INT_END,

        FLOAT_BEGIN = 30,
        Value,
        DefaultValue,
        StepSize,
        EditorVScroll,
        EditorHScroll,
        LimitInMin,
        LimitInMax,
        LimitOutMin,
        LimitOutMax,
        FLOAT_END,

        BOOL_BEGIN = 50,
        Hidden,
        Removable,
        Bridge,
        Dirty,
        DoublePrecision,
        EditorVisible,
        LimitEnabled,
        BOOL_END,

        STRING_BEGIN = 70,
        Filename,
        devName,
        apiName,
        errorMessage,
        bankFile,
        programFile,
        displayedText,
        STRING_END,

        QPOINTF_BEGIN = 90,
        Position,
        EditorSize,
        EditorPosition,
        QPOINTF_END,

        END
    };

//#ifndef QT_NO_DEBUG
    const char* INTNames[];
    const char* FLOATNames[];
    const char* BOOLNames[];
    const char* STRINGNames[];
    const char* QPOINTFNames[];
//#endif

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
        parkingContainer
    };
}

class ObjectInfo;
class MetaInfo;

typedef QMultiMap < MetaInfo, MetaInfo > mapCables;

class MetaTransporter
{
public:
    MetaTransporter::MetaTransporter() :
        autoUpdate(false)
    {}

//    bool dropMime ( const QMimeData * data, MetaInfo & senderInfo, QPointF &pos, InsertionType::Enum insertType=InsertionType::NoInsertion ) =0;
    void ValueChanged( const MetaInfo & senderInfo, int type, const QVariant &value);
    void PostEvent( QEvent * event);
    void AddListener(QObject *obj) { listeners << obj; }
    void RemoveListener(QObject *obj) { listeners.removeAll(obj); }
    void RemoveAllListeners() { listeners.clear(); }

protected:
    bool autoUpdate;

private:
    QList<QObject*>listeners;
};

class MetaData
{
public:
    MetaData() {}
    MetaData(const MetaData &c) {
        *this=c;
    }

    MetaData& operator=(const MetaData& c);

    ~MetaData() {
//        qDebug()<<toString();

        qDeleteAll(listInfos);

//        QMap<MetaInfos::Enum, QPair<int, void*> >::const_iterator j = listComplexInfos.constBegin();
//        while(j!=listComplexInfos.constEnd()) {
//            delete j.value().second;
//            ++j;
//        }
    }

    template<class T> const T GetMeta(const MetaInfos::Enum type) const {
        return static_cast<const T>(listInfos.value(type,0));
    }

    template<class T> T GetMetaData(const MetaInfos::Enum type) const {
        if(!listInfos.contains(type))
            return 0;
        return *static_cast<const T*>(listInfos.value(type));
    }

    template<class T> void SetMeta(const MetaInfos::Enum type, const T &value) {
        if(listInfos.contains(type))
            DelMeta(type);
        AddMeta(type,value);
    }

    template<> void SetMeta(const MetaInfos::Enum type, const float &value) {
        SetMetaFast(type,value);
    }
    template<> void SetMeta(const MetaInfos::Enum type, const int &value) {
        SetMetaFast(type,value);
    }
    template<> void SetMeta(const MetaInfos::Enum type, const bool &value) {
        SetMetaFast(type,value);
    }

    void DelMeta(const MetaInfos::Enum type) {
//        if(type<MetaInfos::COMPLEX_START)
            delete listInfos.take(type);
//        else {
//            delete listComplexInfos.take(type).second;
//        }
    }

//#ifndef QT_NO_DEBUG
    QString toString();
    static QString keyName(const MetaInfos::Enum type);
//#endif

    QDataStream & toStream(QDataStream& stream) const;
    QDataStream & fromStream(QDataStream& stream);

    friend bool operator==(const MetaData &c1, const MetaData &c2);
//    friend bool operator<(const MetaData &c1, const MetaData &c2);

private:
    template<class T> void SetMetaFast(const MetaInfos::Enum type, const T &value) {
        if(listInfos.contains(type))
            ReplaceMeta(type,value);
        else
            AddMeta(type,value);
    }

    template<class T> void ReplaceMeta(const MetaInfos::Enum type, const T &value) {
        memcpy(listInfos[type],&value,sizeof(T));
    }

    template<class T> void AddMeta(const MetaInfos::Enum type, const T &value) {
        T *copy = new T(value);
//        if(type<MetaInfos::COMPLEX_START)
            listInfos.insert(type,copy);
//        else
//            listComplexInfos.insert(type, QPair<int,void*>(sizeof(value) ,copy) );
    }

    QMap<MetaInfos::Enum,void*>listInfos;
//    QMap<MetaInfos::Enum, QPair<int, void*> >listComplexInfos;
};


Q_DECLARE_METATYPE(MetaData);

QDataStream & operator<< (QDataStream& stream, const MetaData& info);
QDataStream & operator>> (QDataStream& stream, MetaData& info);



class MetaInfo
{
    public:
    MetaInfo();
//    MetaInfo(const MetaInfo &c) {
//        *this=c;
//    }
    MetaInfo(const MetaTypes::Enum type);
    MetaInfo(const QByteArray &b);

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
        if(name.isEmpty())
            return;
        objName = name;
    }
    inline const QString & Name() const {
        return objName;
    }

    inline const QVariant Meta(MetaInfos::Enum inf) const {
        return listInfos.value(inf);
    }
    inline void SetMeta(MetaInfos::Enum inf, const QVariant &val) {
//            mutexListInfos.lock();
        listInfos[inf]=val;
//        mutexListInfos.unlock();
        if(transporter)
            transporter->ValueChanged(*this,inf,val);
    }
    inline void DelMeta(MetaInfos::Enum inf) {
//        mutexListInfos.lock();
        listInfos.remove(inf);
//        mutexListInfos.unlock();
    }

    inline MetaTransporter * Transporter() const { return transporter;}
    inline void SetTransporter(MetaTransporter *m) {
        transporter = m;
    }
//    inline HostModel *Model() const {
//        return model;
//    }

//    bool DropMime(const QMimeData *data, QPointF &pos, InsertionType::Enum insertType = InsertionType::NoInsertion) {
//        if(!transporter)
//            return false;
//        return transporter->dropMime(data,MetaInfo(*this),pos,insertType);
//    }

    inline const MetaInfo & info() const {return *this;}

    bool CanConnectTo(const MetaInfo &c) const;

    QString toString() const;
    QString toStringFull() const;

    QDataStream & toStream(QDataStream& stream) const;
    QDataStream & fromStream(QDataStream& stream);

    MetaInfo & operator =(const MetaInfo &c) {
        objType=c.objType;
        objId=c.objId;
        parentId=c.parentId;
        parentObjectId=c.parentObjectId;
        containerId=c.containerId;
        objName=c.objName;
        listInfos=c.listInfos;
        transporter=c.transporter;
        return *this;
    }

protected:
    MetaTransporter *transporter;

private:
    MetaTypes::Enum objType;
    quint32 objId;
    quint32 parentId;
    quint32 parentObjectId;
    quint32 containerId;
    QString objName;
    QMap<MetaInfos::Enum,QVariant>listInfos;
//    mutable DMutex *mutexListInfos;
};


Q_DECLARE_METATYPE(MetaInfo);

QDataStream & operator<< (QDataStream& stream, const MetaInfo& info);
QDataStream & operator>> (QDataStream& stream, MetaInfo& info);
inline bool operator==(const MetaInfo &c1, const MetaInfo &c2) { return c1.ObjId() == c2.ObjId(); }
inline bool operator<(const MetaInfo &c1, const MetaInfo &c2) { return c1.ObjId() < c2.ObjId(); }

class ObjectInfo : public MetaInfo
{
public:
    ObjectInfo();
    ObjectInfo(const ObjectInfo &c);
    virtual ~ObjectInfo();
//    ObjectInfo( MetaTypes::Enum metaType, ObjTypes::Enum objType=ObjTypes::ND, int id=0, QString objName="");
    ObjectInfo(const MetaInfo &c, MetaTransporter *transporter);

    inline ObjectInfo * ParentInfo() const {
        return parentInfo;
    }

    inline ObjectInfo * ContainerInfo() const {
        return containerInfo;
    }

    void SetParent(ObjectInfo *parent);
    virtual void SetContainer(ObjectInfo *container);

    void AddToView();
    void RemoveFromView();
    void UpdateView();

private:
    ObjectInfo* parentInfo;
    ObjectInfo* containerInfo;
    QList<ObjectInfo*>childrenInfo;
};



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
