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

#ifndef CONTAINERPROGRAM_H
#define CONTAINERPROGRAM_H

//#include "precomp.h"
#include "cable.h"
#include "meta/metaobjviewattrib.h"

class MainHost;

class RendererNode;
class MetaPin;
namespace Connectables {

    class Object;
    class Container;
    class ContainerProgram
    {
    public:
        ContainerProgram(MainHost *myHost, Container *container);
        ContainerProgram(const ContainerProgram & c);
        ~ContainerProgram();
        void Load(int progId);
        void Unload();
        void Save(bool saveChildPrograms=true);
        void ParkAllObj();
        void Remove(int prgId);

        void AddObject(QSharedPointer<Object> objPtr);
        void RemoveObject(QSharedPointer<Object> objPtr);
        void ReplaceObject(QSharedPointer<Object> newObjPtr, QSharedPointer<Object> replacedObjPtr);

        bool AddCable(const MetaPin &outputPin, const MetaPin &inputPin, bool hidden=false);
        void RemoveCable(Cable *cab);
//        void RemoveCable(const QModelIndex & index);
        void RemoveCable(const MetaPin &outputPin, const MetaPin &inputPin);
        void RemoveCableFromPin(const MetaPin &pin);
        void RemoveCableFromObj(int objId);
        void CreateBridgeOverObj(int objId);
        void CopyCablesFromObj(int newObjId, int oldObjId);
        void MoveOutputCablesFromObj(int newObjId, int oldObjId);
        void MoveInputCablesFromObj(int newObjId, int oldObjId);
        void GetListOfConnectedPinsTo(const MetaPin &pin, QList<MetaPin> &list);

        bool IsDirty();
        inline void SetDirty() {
            dirty=true;
        }

        void SaveRendererState();
        void LoadRendererState();

        void CollectCableUpdates(QList< QPair<MetaPin,MetaPin> > *addedCables=0,
                                QList< QPair<MetaPin,MetaPin> > *removedCables=0) {
            collectedListOfAddedCables=addedCables;
            collectedListOfRemovedCables=removedCables;
        }

        QDataStream & toStream (QDataStream &) const;
        QDataStream & fromStream (QDataStream &);

        QList<RendererNode*>listOfRendererNodes;
        QTime lastModificationTime;
        QTime savedTime;
        static QTime unsavedTime;

    private:
        inline void ResetDirty() { dirty=false; }

        bool CableExists(const MetaPin &outputPin, const MetaPin &inputPin);
        bool PinExistAndVisible(const MetaPin &info);

        Container *container;
        bool dirty;
        MainHost *myHost;

        QList< QSharedPointer<Object> >listObjects;
        QList<Cable*>listCables;

        QMap<int,MetaObjViewAttrib>mapObjAttribs;

        QList< QPair<MetaPin,MetaPin> > *collectedListOfAddedCables;
        QList< QPair<MetaPin,MetaPin> > *collectedListOfRemovedCables;

        friend class Container;
        friend class ParkingContainer;
    };
}

QDataStream & operator<< (QDataStream & out, const Connectables::ContainerProgram& value);
QDataStream & operator>> (QDataStream & in, Connectables::ContainerProgram& value);




#endif // CONTAINERPROGRAM_H
