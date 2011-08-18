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

//#ifndef QT_NO_DEBUG

#define stringify( name ) # name

const char* MetaInfos::INTNames[] =
{
    stringify( ObjType ),
    stringify( Direction ),
    stringify( Media ),
    stringify( BridgeMedia ),
    stringify( devId ),
    stringify( apiId ),
    stringify( duplicateNamesCounter ),
    stringify( nbInputs ),
    stringify( nbOutputs ),
    stringify( PinNumber ),
    stringify( ValueStep ),
    stringify( DefaultValueStep )
};

const char* MetaInfos::FLOATNames[]=
{
    stringify( Value ),
    stringify( DefaultValue ),
    stringify( StepSize ),
    stringify( EditorVScroll ),
    stringify( EditorHScroll )
};
const char* MetaInfos::BOOLNames[]=
{
    stringify( Hidden ),
    stringify( Removable ),
    stringify( Bridge ),
    stringify( Dirty ),
    stringify( DoublePrecision ),
    stringify( EditorVisible ),
    stringify( LimitEnabled ),
    stringify( LimitInMin ),
    stringify( LimitInMax ),
    stringify( LimitOutMin ),
    stringify( LimitOutMax ),
};
const char* MetaInfos::STRINGNames[]=
{
    stringify( Filename ),
    stringify( devName ),
    stringify( apiName ),
    stringify( errorMessage ),
    stringify( bankFile ),
    stringify( programFile ),
    stringify( displayedText ),
};
const char* MetaInfos::QPOINTFNames[]=
{
    stringify( Position ),
    stringify( EditorSize ),
    stringify( EditorPosition ),
};

//#endif

//#ifndef QT_NO_DEBUG
QString MetaData::toString() {
    QString str;
    qDebug() << this;

    QMap<MetaInfos::Enum,void*>::const_iterator i = listInfos.constBegin();
    while(i!=listInfos.constEnd()) {

        str += QString("%1(%2)=").arg( i.key() ).arg( keyName(i.key()) );

        if(i.key()<MetaInfos::INT_END) {
            str += QString::number(*static_cast<int*>(i.value()));
        } else if(i.key()<MetaInfos::FLOAT_END) {
            str += QString::number(*static_cast<float*>(i.value()));
        } else if(i.key()<MetaInfos::BOOL_END) {
            str.append(*static_cast<bool*>(i.value()));
        } else if(i.key()<MetaInfos::STRING_END) {
            str.append(*static_cast<QString*>(i.value()));
        } else if(i.key()<MetaInfos::QPOINTF_END) {
            str += QString("%1:%2")
                .arg(static_cast<QPointF*>(i.value())->x())
                .arg(static_cast<QPointF*>(i.value())->y());
        }
//            str+=" " + QString::number((long)i.value(),16);
//        str+="\n";
        str+="|";
        ++i;
    }

//        QMap<MetaInfos::Enum, QPair<int, void*> >::const_iterator j = listComplexInfos.constBegin();
//        while(j!=listComplexInfos.constEnd()) {

//            str += QString("%1 (%2,size:%3) = ")
//                    .arg(MetaInfos::Names[j.key()])
//                    .arg(j.key())
//                    .arg(j.value().first);

//            str+= QString::fromRawData( static_cast<const QChar*>(j.value().second) ,j.value().first);
//            str+=" " + QString::number((long)j.value().second,16);
//            str+="\n";
//            ++j;
//        }
    return str;
}
//#endif

QString MetaData::keyName(const MetaInfos::Enum type)
{
    if(type<MetaInfos::INT_END)
        return MetaInfos::INTNames[type-MetaInfos::INT_BEGIN-1];
    if(type<MetaInfos::FLOAT_END)
        return MetaInfos::FLOATNames[type-MetaInfos::FLOAT_BEGIN-1];
    if(type<MetaInfos::BOOL_END)
        return MetaInfos::BOOLNames[type-MetaInfos::BOOL_BEGIN-1];
    if(type<MetaInfos::STRING_END)
        return MetaInfos::STRINGNames[type-MetaInfos::STRING_BEGIN-1];
    if(type<MetaInfos::QPOINTF_END)
        return MetaInfos::QPOINTFNames[type-MetaInfos::QPOINTF_BEGIN-1];
    return "nd";
}

MetaData & MetaData::operator =(const MetaData &c) {
    if (this == &c)
        return *this;

    qDeleteAll(listInfos);
    listInfos.clear();

    QMap<MetaInfos::Enum,void*>::const_iterator i = c.listInfos.constBegin();
    while(i!=c.listInfos.constEnd()) {
        if(i.key()<MetaInfos::INT_END)
            AddMeta(i.key(),*static_cast<int*>(i.value()));
        else if(i.key()<MetaInfos::FLOAT_END)
            AddMeta(i.key(),*static_cast<float*>(i.value()));
        else if(i.key()<MetaInfos::BOOL_END)
            AddMeta(i.key(),*static_cast<bool*>(i.value()));
        else if(i.key()<MetaInfos::STRING_END)
            AddMeta(i.key(),*static_cast<QString*>(i.value()));
        else if(i.key()<MetaInfos::QPOINTF_END)
            AddMeta(i.key(),*static_cast<QPointF*>(i.value()));
        ++i;
    }

//        QMap<MetaInfos::Enum, QPair<int, void*> >::const_iterator j = listComplexInfos.constBegin();
//        while(j!=listComplexInfos.constEnd()) {
//            delete j.value().second;
//            ++j;
//        }

//    QMap<MetaInfos::Enum, QPair<int, void*> >::const_iterator j = c.listComplexInfos.constBegin();
//    while(j!=c.listComplexInfos.constEnd()) {
//        void* copy = (void*)new char[j.value().first];
//        memcpy(copy, j.value().second, sizeof(copy));
//        listComplexInfos.insert(j.key(), QPair<int,void*>(sizeof(copy),copy) );
//        ++j;
//    }
    return *this;
}

QDataStream & MetaData::toStream(QDataStream& stream) const
{
    stream << static_cast<quint16>(listInfos.size());

    QMap<MetaInfos::Enum,void*>::const_iterator i = listInfos.constBegin();
    while(i!=listInfos.constEnd()) {

        stream << static_cast<quint16>(i.key());

        if(i.key()<MetaInfos::INT_END)
            stream << *static_cast<int*>(i.value());
        else if(i.key()<MetaInfos::FLOAT_END)
            stream << *static_cast<float*>(i.value());
        else if(i.key()<MetaInfos::BOOL_END)
            stream << *static_cast<bool*>(i.value());
        else if(i.key()<MetaInfos::STRING_END)
            stream << *static_cast<QString*>(i.value());
        else if(i.key()<MetaInfos::QPOINTF_END)
            stream << *static_cast<QPointF*>(i.value());

        ++i;
    }
    return stream;
}

QDataStream & MetaData::fromStream(QDataStream& stream)
{
    quint16 size;
    stream >> size;
    for(int i=0; i<size; ++i) {

        quint16 key;
        stream >> key;

        if(key<MetaInfos::INT_END) {
            int i;
            stream >> i;
            SetMeta( static_cast<MetaInfos::Enum>(key), i);
        } else if(key<MetaInfos::FLOAT_END) {
            float f;
            stream >> f;
            SetMeta(static_cast<MetaInfos::Enum>(key), f);
        } else if(key<MetaInfos::BOOL_END) {
            bool b;
            stream >> b;
            SetMeta(static_cast<MetaInfos::Enum>(key), b);
        } else if(key<MetaInfos::STRING_END) {
            QString s;
            stream >> s;
            SetMeta(static_cast<MetaInfos::Enum>(key), s);
        } else if(key<MetaInfos::QPOINTF_END) {
            QPointF p;
            stream >> p;
            SetMeta(static_cast<MetaInfos::Enum>(key), p);
        }
    }
    return stream;
}

QDataStream & operator<< (QDataStream& s, const MetaData& data) { return data.toStream(s); }
QDataStream & operator>> (QDataStream& s, MetaData& data) { return data.fromStream(s); }

//only compare ints and bools
bool operator==(const MetaData &c1, const MetaData &c2)
{
    if(c1.listInfos.size()!=c2.listInfos.size())
        return false;

    QMap<MetaInfos::Enum,void*>::const_iterator i = c1.listInfos.constBegin();
    while(i!=c1.listInfos.constEnd()) {
        if(i.key() > MetaInfos::INT_BEGIN && i.key() < MetaInfos::INT_END)
            if( *(int*)i.value() != *(int*)c2.listInfos.value(i.key()) )
                return false;
        if(i.key() > MetaInfos::BOOL_BEGIN && i.key() < MetaInfos::BOOL_END)
            if( *(bool*)i.value() != *(bool*)c2.listInfos.value(i.key()) )
                return false;
        ++i;
    }
    return true;
}
//bool operator<(const MetaData &c1, const MetaData &c2) { if(c1.listInfos < c2.listInfos; }

void MetaTransporter::ValueChanged( const MetaInfo & senderInfo, int type, const QVariant &value)
{
    if(!autoUpdate)
        return;
    Events::valChanged *e = new Events::valChanged(MetaInfo(senderInfo), (MetaInfos::Enum)type, value);
    PostEvent(e);
}

void MetaTransporter::PostEvent( QEvent * event) {
#ifndef QT_NO_DEBUG
    if(listeners.isEmpty()) {
        LOG("no listener")
    }
#endif
    foreach(QObject *obj, listeners) {
        qApp->postEvent(obj,event);
    }
}

MetaInfo::MetaInfo() :
    objType(MetaTypes::ND),
    objId(0),
    objName(""),
    parentId(0),
    containerId(0),
    parentObjectId(0),
    transporter(0)
{
}

MetaInfo::MetaInfo(const MetaTypes::Enum type) :
    objType(type),
    objId(0),
    objName(""),
    parentId(0),
    containerId(0),
    parentObjectId(0),
    transporter(0)
{
}

MetaInfo::MetaInfo(const QByteArray &b)
{
    QDataStream stream(b);
    fromStream(stream);
}

ObjectInfo::ObjectInfo() :
    MetaInfo(),
    parentInfo(0),
    containerInfo(0)
{
}

ObjectInfo::ObjectInfo(const ObjectInfo &c) :
    MetaInfo(c)
{
    parentInfo=c.parentInfo;
    containerInfo=c.containerInfo;
}

ObjectInfo::ObjectInfo(const MetaInfo &c, MetaTransporter *transporter) :
    MetaInfo(c),
    parentInfo(0),
    containerInfo(0)
{
    SetTransporter(transporter);
}

ObjectInfo::~ObjectInfo()
{
    if(parentInfo) {
        parentInfo->childrenInfo.removeAll(this);
        parentInfo=0;
    }
    foreach(ObjectInfo *info, childrenInfo) {
        info->SetParent(0);
    }
}

void ObjectInfo::AddToView()
{
    if(!ContainerId())
        return;

    if(Meta(MetaInfos::Hidden).toBool())
        return;

    Events::sendObj *event = new Events::sendObj(info(), Events::typeNewObj);
    if(transporter) {
        transporter->PostEvent(event);
    }  else {
        LOG("transporter not set");
    }

    foreach(ObjectInfo *info, childrenInfo) {
        info->AddToView();
    }
}

void ObjectInfo::RemoveFromView()
{
    Events::delObj *event = new Events::delObj(ObjId());
    if(transporter)
        transporter->PostEvent(event);
    else {
        LOG("transporter not set")
    }
}

void ObjectInfo::UpdateView()
{
    if(!ContainerId())
        return;

    if(Meta(MetaInfos::Hidden).toBool())
        return;

    Events::sendObj *event = new Events::sendObj(info(), Events::typeUpdateObj);
    if(transporter)
        transporter->PostEvent(event);
    else {
        LOG("transporter not set");
    }
}

void ObjectInfo::SetContainer(ObjectInfo *container) {

    if(Meta(MetaInfos::Bridge).toBool())
        container = container->ContainerInfo();

    containerInfo = container;

    if(container)
        SetContainerId(container->ObjId());
    else
        SetContainerId(0);

    if(Type()!=MetaTypes::container) {
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

    if(parentInfo) {
        SetParentId(parent->ObjId());
        parentInfo->childrenInfo << this;

        SetTransporter(parent->Transporter());

        if(parentInfo->Type()==MetaTypes::object || parentInfo->Type()==MetaTypes::container || parentInfo->Type()==MetaTypes::bridge)
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

    foreach(ObjectInfo *info, childrenInfo) {
        info->SetParent(this);
    }
}


bool MetaInfo::CanConnectTo(const MetaInfo &c) const
{
    //don't connect object to itself
//    if(objId == c.objId)
//        return false;

    if(Type()!=MetaTypes::pin)
        return false;
    if(c.Type()!=MetaTypes::pin)
        return false;

    //must be in the same container
    if(ContainerId() != c.ContainerId())
        return false;

    //must be opposite directions
    if(Meta(MetaInfos::Direction) == c.Meta(MetaInfos::Direction))
        return false;

    //must be the same type (audio/midi/automation) or a bridge pin
    if(Meta(MetaInfos::Media)!=MediaTypes::Bridge
        && c.Meta(MetaInfos::Media)!=MediaTypes::Bridge
        && Meta(MetaInfos::Media) != c.Meta(MetaInfos::Media))
        return false;

    return true;
}

QString MetaInfo::toString() const
{
    return QString("type:%1 id:%2 name:%3 parent:%4 container:%5 obj:%6")
            .arg(objType)
            .arg(objId)
            .arg(objName)
            .arg(parentId)
            .arg(containerId)
            .arg(parentObjectId);
}

QString MetaInfo::toStringFull() const
{
    QString str(toString());
    QMap<MetaInfos::Enum,QVariant>::iterator i = listInfos.begin();
    while(i != listInfos.end()) {
        str.append( QString(" %1:%2").arg(i.key()).arg(i.value().toString()));
        ++i;
    }
    return str;
}

QDataStream & MetaInfo::toStream(QDataStream& stream) const
{
    stream << (quint8)objType;
    stream << objId;
    stream << objName;
    stream << parentId;
    stream << containerId;
    stream << parentObjectId;

//    mutexListInfos.lock();
    stream << (quint16)listInfos.size();
    QMap<MetaInfos::Enum,QVariant>::iterator i = listInfos.begin();
    while(i != listInfos.end()) {
        stream << (quint16)i.key();
        stream << i.value();
        ++i;
    }
//    mutexListInfos.unlock();

//    stream << (quint16)childrenInfo.size();
//    foreach(ObjectInfo* o, childrenInfo) {
//        stream << o->ObjId();
//    }

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

QDataStream & MetaInfo::fromStream(QDataStream& stream)
{
    quint8 type;
    stream >> type;
    objType=(MetaTypes::Enum)type;
    stream >> objId;
    stream >> objName;
    stream >> parentId;
    stream >> containerId;
    stream >> parentObjectId;

    quint16 nb;
    stream >> nb;

//    mutexListInfos.lock();
    for(int i=0; i<nb; i++) {
        quint16 id;
        QVariant val;
        stream >> id;
        stream >> val;
        listInfos.insert((MetaInfos::Enum)id,val);
    }
//    mutexListInfos.unlock();

//    stream >> nb;
//    for(int i=0; i<nb; i++) {
//        quint32 id;
//        stream >> id;
//    }

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

QDataStream & operator<< (QDataStream& stream, const MetaInfo& objInfo)
{
   return objInfo.toStream(stream);
}

QDataStream & operator>> (QDataStream& stream, MetaInfo& objInfo)
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
