/**************************************************************************
#    Copyright 2010-2011 Rapha�l Fran�ois
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

#ifndef CONTAINER_H
#define CONTAINER_H

#include "object.h"
#include "bridge.h"
#include "containerprogram.h"
#include "models/hostmodel.h"

namespace Connectables {

    class Container : public Object
    {
    Q_OBJECT
    public:
        Container(MainHost *myHost,int index, const ObjectInfo &info);
        virtual ~Container();

        void Hide();
        void ConnectObjects(QSharedPointer<Object> fromObjOutputs, QSharedPointer<Object> toObjInputs, bool hiddenCables);
        void SetContainerId(quint16 id);
        const QModelIndex &GetCablesIndex();

        QDataStream & toStream (QDataStream &) const;
        bool fromStream (QDataStream &);

        void ProgramToStream (int progId, QDataStream &out);
        void ProgramFromStream (int progId, QDataStream &in);

        void OnChildDeleted(Object *obj);

        /*!
          Set this container as the holder of the rendering map
          */
        void SetOptimizerFlag(bool opt) { optimizerFlag=opt; }

        virtual bool Close();
        virtual void AddObject(QSharedPointer<Object> objPtr);
        virtual void ParkObject(QSharedPointer<Object> objPtr);

        void AddParkedObject(QSharedPointer<Object> objPtr);
        void CopyCablesFromObj(QSharedPointer<Object> newObjPtr, QSharedPointer<Object> ObjPtr);
        void MoveOutputCablesFromObj(QSharedPointer<Object> newObjPtr, QSharedPointer<Object> ObjPtr);
        void MoveInputCablesFromObj(QSharedPointer<Object> newObjPtr, QSharedPointer<Object> ObjPtr);
        void GetListOfConnectedPinsTo(const ConnectionInfo &pin, QList<ConnectionInfo> &list);
        bool IsDirty() {
            return currentContainerProgram->IsDirty();
        }
        inline void SetDirty() {
            currentContainerProgram->SetDirty();
        }
        void SetSleep(bool sleeping);

        /// shared pointer to the bridge in object
        QSharedPointer<Object> bridgeIn;

        /// shared pointer to the bridge out object
        QSharedPointer<Object> bridgeOut;

        QSharedPointer<Object> bridgeSend;
        QSharedPointer<Object> bridgeReturn;

        /// model for the parking storage
        HostModel parkModel;

        QWeakPointer<Container>childContainer;
        QWeakPointer<Container>parentContainer;

        void SetLoadingMode(bool active=true) {
            loadingMode = active;

            //end of loading mode, update renderer map if needed
            if(!active) {

            }
        }

        void UpdateModificationTime() {
            if(loadingMode)
                return;

            if(currentContainerProgram)
                currentContainerProgram->lastModificationTime = QTime::currentTime();
            if(childContainer)
                childContainer.toStrongRef()->UpdateModificationTime();

        }

        const QTime GetLastModificationTime();

        int GetProgramToSet() { if(progToSet==-1) return currentProgId; else return progToSet; }

        inline ContainerProgram * GetCurrentProgram() {return currentContainerProgram;}

        void NewRenderLoop();

    protected:
        void AddChildObject(QSharedPointer<Object> objPtr);
        void ParkChildObject(QSharedPointer<Object> objPtr);
        bool LoadProgram(ContainerProgram *newProg);

        /// true if this container saves the solver status
        bool optimizerFlag;

        /// list of programs
        QHash<int,ContainerProgram*>listContainerPrograms;

        /// pointer to the current program
        ContainerProgram* currentContainerProgram;

        /// list of static objects (bridges are static)
        QList< QSharedPointer< Object > >listStaticObjects;

        /// a child model index for the cables
        QPersistentModelIndex cablesNode;

        /// list of all loaded Objects
        QList<Object*>listLoadedObjects;

        /// store the objects while loading, preventing them from being deleted. (the objects are loaded before the programs using them)
        QList< QSharedPointer< Object > >listLoadingObjects;

        /// id of the progam to change on the next rendering loop
        int progToSet;

        bool loadingMode;

        bool loadHeaderStream (QDataStream &);
        bool loadObjectFromStream (QDataStream &);
        bool loadProgramFromStream (QDataStream &);

        QMutex progLoadMutex;

        QList<int> listProgToRemove;

    public slots:
        void UserAddObject(const QSharedPointer<Object> &objPtr,
                           InsertionType::Enum insertType = InsertionType::NoInsertion,
                           QList< QPair<ConnectionInfo,ConnectionInfo> > *listOfAddedCables=0,
                           QList< QPair<ConnectionInfo,ConnectionInfo> > *listOfRemovedCables=0,
                           const QSharedPointer<Object> &targetPtr=QSharedPointer<Object>());
        void UserParkObject(QSharedPointer<Object> objPtr,
                            RemoveType::Enum removeType = RemoveType::RemoveWithCables,
                            QList< QPair<ConnectionInfo,ConnectionInfo> > *listOfAddedCables=0,
                            QList< QPair<ConnectionInfo,ConnectionInfo> > *listOfRemovedCables=0);
        void UserAddCable(const ConnectionInfo &outputPin, const ConnectionInfo &inputPin);
        void UserAddCable(const QPair<ConnectionInfo,ConnectionInfo>&pair);
        void UserRemoveCableFromPin(const ConnectionInfo &pin);
        void UserRemoveCable(const ConnectionInfo &outputPin, const ConnectionInfo &inputPin);
        void UserRemoveCable(const QPair<ConnectionInfo,ConnectionInfo>&pair);

        void AddCable(const ConnectionInfo &outputPin, const ConnectionInfo &inputPin, bool hidden=false);
        void RemoveCableFromPin(const ConnectionInfo &pin);
        void RemoveCable(const ConnectionInfo &outputPin, const ConnectionInfo &inputPin);

        void SaveProgram();
        void UnloadProgram();
        void LoadProgram(int prog);

        void SetProgram(const QModelIndex &idx);
        void RemoveProgram(const QModelIndex &idx = QModelIndex());
        void PostRender();

        void SetBufferSize(unsigned long size);
        void SetSampleRate(float rate=44100.0);

        friend class ContainerProgram;
    };


}

#endif // CONTAINER_H
