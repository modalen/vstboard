/******************************************************************************
#    Copyright 2010 Rapha�l Fran�ois
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
******************************************************************************/

#ifndef CONNECTABLEOBJECT_H
#define CONNECTABLEOBJECT_H

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif

#include "../precomp.h"

#include "audiopinin.h"
#include "audiopinout.h"
#include "midipinin.h"
#include "midipinout.h"
#include "parameterpinin.h"
#include "parameterpinout.h"
#include "bridgepinin.h"
#include "bridgepinout.h"
#include "objectprogram.h"
#include "objectinfo.h"

class SolverNode;

namespace Connectables {

    typedef QHash<int,ObjectProgram*> hashPrograms;
    typedef QHash<ushort,ParameterPin*> hashListParamPin;

    class Object : public QObject
    {
    Q_OBJECT
    public:

        Object(int index, const ObjectInfo &info);
        virtual ~Object();

        virtual bool Open();
        virtual bool Close();
        virtual void Hide();
        virtual void Render() {}
        virtual int GetProcessingTime() {return 0;}

        void setObjectName(const QString &name);

        virtual Pin * GetPin(const ConnectionInfo &pinInfo);
        inline int GetSavedIndex() {return savedIndex;}
        inline void ResetSavedIndex(int id=-2) {savedIndex=id;}

        const ObjectInfo & info() const {return objInfo;}
        SolverNode *GetSolverNode() const {return solverNode;}
        void SetSolverNode(SolverNode *node) {solverNode=node;}

        QList<BridgePinIn*> GetListBridgePinIn() {return listBridgePinIn;}
        QList<BridgePinOut*> GetListBridgePinOut() {return listBridgePinOut;}

        void SetIndex(int i) {index=i;}
        inline const int GetIndex() const {return index;}

        const int GetContainerId() const {return containerId;}

        //don't include in rendering map if sleeping
        virtual void SetSleep(bool sleeping);
        bool GetSleep();

        virtual float SetParameter(int paramId, float value) {return value;}
        virtual void MidiMsgFromInput(long msg) {}

        virtual QString GetParameterName(ConnectionInfo pinInfo) {return "";}

        void NewRenderLoop();

        virtual void SetParkingIndex(const QModelIndex &parentIndex);

        virtual QDataStream & toStream (QDataStream &) const;
        virtual QDataStream & fromStream (QDataStream &);

        inline void Lock() { objMutex.lock();}
        inline void Unlock() { objMutex.unlock();}

        bool GetLearningMode();
        bool hasEditor;
        bool canLearn;

        virtual void SetContainerId(quint16 id);
        virtual void SetParentModeIndex(const QModelIndex &parentIndex);
        virtual void UpdateModelNode();

        int GetCurrentProgId() {return currentProgId;}

        bool listenProgramChanges;

        virtual void SetBridgePinsInVisible(bool visible);
        virtual void SetBridgePinsOutVisible(bool visible);

        virtual void CopyProgram(int ori, int dest);
        virtual void RemoveProgram(int prg);

        QPersistentModelIndex modelIndex;

        virtual void SetContainerAttribs(const ObjectConatinerAttribs &attr);
        virtual void GetContainerAttribs(ObjectConatinerAttribs &attr);

        QString errorMessage;

    protected:
        QList<AudioPinIn*>listAudioPinIn;
        QList<AudioPinOut*>listAudioPinOut;
        QList<MidiPinIn*>listMidiPinIn;
        QList<MidiPinOut*>listMidiPinOut;
        hashListParamPin listParameterPinIn;
        hashListParamPin listParameterPinOut;
        QList<BridgePinIn*>listBridgePinIn;
        QList<BridgePinOut*>listBridgePinOut;

        QMutex objMutex;

        bool parked;

        //used by pathsolver
        SolverNode *solverNode;

        hashPrograms listPrograms;

        int index;
        int savedIndex;

        bool sleep;
        short parameterLearning;
        ObjectProgram *currentProgram;
        bool progIsDirty;

        bool closed;
        quint16 containerId;

        QPersistentModelIndex modelAudioIn;
        QPersistentModelIndex modelAudioOut;
        QPersistentModelIndex modelMidiIn;
        QPersistentModelIndex modelMidiOut;
        QPersistentModelIndex modelParamIn;
        QPersistentModelIndex modelParamOut;
        QPersistentModelIndex modelBridgeIn;
        QPersistentModelIndex modelBridgeOut;

        int currentProgId;

        ObjectInfo objInfo;

    signals:
        void LearningModeChanged(bool learn);
        void UnLearningModeChanged(bool unlearn);
        void CpuLoad(float load);
        void CloseEditorWindow();

    public slots:
        virtual void SaveProgram();
        virtual void UnloadProgram();
        virtual void LoadProgram(int prog);
        void OnProgramDirty() {progIsDirty=true;}

        virtual void SetBufferSize(long size) {}
        virtual void SetSampleRate(float rate=44100.0) {}
        virtual void OnParameterChanged(ConnectionInfo pinInfo, float value) {}
        virtual void OnEditorVisibilityChanged(bool visible) {}

        void CloseSlot() {Close();}

    };


}

QDataStream & operator<< (QDataStream & out, const Connectables::Object& value);
QDataStream & operator>> (QDataStream & in, Connectables::Object& value);

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // CONNECTABLEOBJECT_H