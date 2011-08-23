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
#include "objectinfo.h"

namespace Events {

    enum type {
        typeNewObj = QEvent::User,
        typeDelObj,
        typeParkObj,
        typeUpdateObj,
        typeValChanged,
        typeCommand
//        typeDropMime
    };

    class sendObj : public QEvent
    {
    public:
        sendObj(MetaInfo objInfo, Events::type type) :
            QEvent((QEvent::Type)type),
            objInfo(objInfo)
        {
        }

        MetaInfo objInfo;
    };

    class delObj : public QEvent
    {
    public:
        delObj(quint32 objId) :
            QEvent((QEvent::Type)typeDelObj),
            objId(objId)
        {}

        quint32 objId;
    };

    class valChanged : public QEvent
    {
    public:
        valChanged(MetaInfo objInfo, const MetaInfos::Enum type, QVariant value) :
            QEvent((QEvent::Type)typeValChanged),
            objInfo(objInfo),
            type(type),
            value(value)
        {}

        MetaInfo objInfo;
        MetaInfos::Enum type;
        QVariant value;

    };

    class command : public QEvent
    {
    public:
        command(QUndoCommand *cmd) :
            QEvent((QEvent::Type)typeCommand),
            cmd(cmd)
        {}

        QUndoCommand *cmd;
    };

//    class dropMime : public QEvent
//    {
//    public:
//        dropMime(const QMimeData *data, const MetaInfo &senderInfo, InsertionType::Enum insertType = InsertionType::NoInsertion) :
//            QEvent((QEvent::Type)typeDropMime),
//            data(data),
//            senderInfo(senderInfo),
//            insertType(insertType)
//        {}

//        const QMimeData *data;
//        MetaInfo senderInfo;
//        InsertionType::Enum insertType;
//    };
}

#endif // MYEVENTS_H
