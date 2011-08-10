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

#ifndef MYEVENTS_H
#define MYEVENTS_H

#include "precomp.h"
#include "connectables/objectinfo.h"
#include "connectables/connectioninfo.h"

namespace Events {

    enum type {
        typeNewObj = QEvent::User,
        typeNewPin
    };

    class newObj : public QEvent
    {
    public:
        newObj(const ObjectInfo &objInfo, int parentIndex) :
            QEvent((QEvent::Type)typeNewObj),
            objInfo(objInfo),
            parentIndex(parentIndex)
        {}

        QStandardItem *CreateItem() {
            QStandardItem *item = new QStandardItem(objInfo.name);
            item->setData(QVariant::fromValue(objInfo), UserRoles::objInfo);
            item->setData(objInfo.forcedObjId, UserRoles::value);
            item->setData(errorMessage, UserRoles::errorMessage);
            return item;
        }

        ObjectInfo objInfo;
        int parentIndex;
        QString errorMessage;
    };

    class newPin : public QEvent
    {
    public:
        newPin(const QString &name, const QVariant &value,const ConnectionInfo &connectionInfo, float stepSize, int parentIndex) :
            QEvent((QEvent::Type)typeNewPin),
            name(name),
            value(value),
            connectionInfo(connectionInfo),
            stepSize(stepSize),
            parentIndex(parentIndex)
        {}

        QStandardItem *CreateItem() {
            QStandardItem *item = new QStandardItem("pin");
            item->setData(name,Qt::DisplayRole);
            item->setData(value,UserRoles::value);
            item->setData( QVariant::fromValue(ObjectInfo(NodeType::pin)),UserRoles::objInfo);
            item->setData(QVariant::fromValue(connectionInfo),UserRoles::connectionInfo);
            item->setData(stepSize,UserRoles::stepSize);
            return item;
        }

        ConnectionInfo connectionInfo;
        int parentIndex;
        QString name;
        QVariant value;
        float stepSize;
    };
}

#endif // MYEVENTS_H
