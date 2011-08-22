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

#ifndef CONNECTABLEPIN_H
#define CONNECTABLEPIN_H

#include "precomp.h"
#include "globals.h"
#include "debugmutex.h"
#include "meta/metapin.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif

namespace Connectables {

//    class PinInfo {
//        quint32 objId;
//        Directions::Enum direction;
//        MediaTypes::Enum mediatype;
//    };

    class Object;
    class Pin : public QObject, public MetaPin
    {
    Q_OBJECT
    public:
        Pin(Object *parent, MetaData &info);
        virtual ~Pin();

        virtual void SendMsg(const PinMessage::Enum msgType,void *data=0);

        /*!
         Message received from another pin
         \param msgType PinMessage
         \param data received data
         */
        virtual void ReceiveMsg(const PinMessage::Enum msgType,void *data=0) {}

        /*!
          Get the current value (vu meter or parameter value)
          \return current value
          */
        virtual float GetValue() {return internValue;}

//        virtual void SetParentModelIndex(const QModelIndex &newParent);

        /// \return true if visible
        virtual void SetVisible(bool vis);
        void SetBridge(bool bridge);
//        void UpdateModelNode();
        void Close();

        /*!
          Prepare for a new rendering
          Called one time at the beginning of the loop
           */
        virtual void NewRenderLoop() {}

        void SetNameCanChange(bool canChange) {nameCanChange = canChange;}

    protected:

        /// current value
        float internValue;

        /// pointer to the parent Object
        Object *parent;

        /// index of the parent in the model
        QPersistentModelIndex parentIndex;

        /// index of this pin in the model
        QPersistentModelIndex modelIndex;

        /// global pin mutex
        DMutex pinMutex;

        /// true if closed or closing (no send, no receive)
        bool closed;

        /// true if the value changes since the laste view update
        bool valueChanged;

        bool nameCanChange;

    public slots:
        virtual void updateView();

    };
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // CONNECTABLEPIN_H
