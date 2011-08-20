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

#ifndef METAOBJENGINE_H
#define METAOBJENGINE_H

#include "metadata.h"

class MetaTransporter;
class MetaObjEngine : public MetaData
{
public:
    MetaObjEngine(const MetaType::Enum type=MetaType::ND, MetaTransporter *transport=0);
    MetaObjEngine(const MetaData &data, MetaTransporter *transport=0);
    ~MetaObjEngine();

    MetaType::Enum Type() const {return objType;}
    void SetType(const MetaType::Enum type) {
        objType=type;
        SetMeta<MetaType::Enum>(metaT::ObjType, objType);
    }

    void SetObjId(const quint32 id) {
        objId = id;
        SetMeta<quint32>(metaT::ObjId, id);
    }
    const quint32 ObjId() const {return objId;}


    void SetParentId(const quint32 id) {
        parentId = id;
        SetMeta<quint32>(metaT::ParentId, id);
    }
    quint32 ParentId() const {return parentId;}

    void SetContainerId(const quint32 id) {
        containerId = id;
        SetMeta<quint32>(metaT::ContainerId, id);
    }
    quint32 ContainerId() const {return containerId;}

    void SetParentObjectId(const quint32 id) {
        parentObjectId = id;
        SetMeta<quint32>(metaT::ParentObjId, id);
    }
    quint32 ParentObjectId() const {return parentObjectId;}

    void SetName(const QString &name) {
        if(name.isEmpty()) return;
        objName = name;
        SetMeta<QString>(metaT::ObjName, name);
    }

    void SetTransporter(MetaTransporter *m) {transporter = m;}
    const MetaTransporter * Transporter() const { return transporter;}

    void SetParent(MetaObjEngine *parent);
    const MetaObjEngine * ParentInfo() const {return parentInfo;}

    void SetContainer(MetaObjEngine *container);
    const MetaObjEngine * ContainerInfo() const {return containerInfo;}

    void AddToView();
    void RemoveFromView();
    void UpdateView();

private:
    MetaType::Enum objType;
    quint32 objId;
    quint32 parentId;
    quint32 parentObjectId;
    quint32 containerId;
    QString objName;
    MetaObjEngine* parentInfo;
    MetaObjEngine* containerInfo;
    QList<MetaObjEngine*>childrenInfo;
    MetaTransporter *transporter;
};

#endif // METAOBJENGINE_H
