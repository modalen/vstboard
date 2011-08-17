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

#ifndef CONNECTABLEOBJECT_H
#define CONNECTABLEOBJECT_H

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif

#include "precomp.h"

#include "connectables/pins/audiopin.h"
#include "connectables/pins/midipinin.h"
#include "connectables/pins/midipinout.h"
#include "connectables/pins/parameterpin.h"
#include "connectables/pins/bridgepinin.h"
#include "connectables/pins/bridgepinout.h"
#include "connectables/objectprogram.h"
#include "connectables/pinslist.h"
#include "objectinfo.h"

class SolverNode;
class MainHost;

#define FIXED_PIN_STARTINDEX 0xf000
namespace FixedPinNumber {
    enum Enum {
        editorVisible = FIXED_PIN_STARTINDEX,
        learningMode,
        vstProgNumber,
        numberOfPins
    };
}

namespace LearningMode {
    enum Enum {
        off,
        learn,
        unlearn
    };
}

namespace Connectables {

    typedef QHash<int,ObjectProgram*> hashPrograms;

    class Object : public QObject, public ObjectInfo
    {
    Q_OBJECT
    public:

        Object(MainHost *host, MetaInfo &info);
        virtual ~Object();

        /*!
          When saving a project the index of the object is saved,
            used when loading a project file to map the connections with the current index
          \return saved index
          */
        inline quint32 GetSavedIndex() {return savedIndex;}

        /// Reset the savedIndex to the current index, when the file is loaded or before saving
        inline void ResetSavedIndex(quint32 id=0) {savedIndex=id;}

        PinsList* GetPinList(Directions::Enum dir, MediaTypes::Enum type) const;

        bool GetSleep();
        virtual void NewRenderLoop();

        LearningMode::Enum GetLearningMode();
//        QStandardItem *GetParkingItem();
//        virtual QStandardItem *GetFullItem();

        /*!
          Get the current program id
          \return program id
          */
        int GetCurrentProgId() {return currentProgId;}

        /// \return a pointer to the MainHost
        inline MainHost *getHost() {return myHost;}

        virtual bool Open();
        virtual bool Close();
        virtual void Hide();
        virtual Pin * GetPin(const MetaInfo &pinInfo);
        virtual void SetSleep(bool sleeping);
        virtual QDataStream & toStream (QDataStream &) const;
        virtual bool fromStream (QDataStream &);
        virtual void RemoveProgram(int prg);
        virtual void SetContainerAttribs(const ObjectContainerAttribs &attr);
        virtual void GetContainerAttribs(ObjectContainerAttribs &attr);
        virtual void CopyStatusTo(QSharedPointer<Object>objPtr);
        virtual Pin* CreatePin(MetaInfo &info);
        virtual bool IsDirty();

        /// Render the object, can be called multiple times if the rendering needs multiple passes
        virtual void Render() {}

        /// Called by the midi pins when a midi message is received
        virtual void MidiMsgFromInput(long msg) {}

        /*!
          Get the name of a parameter pin
          \param pinInfo
          \return the name
          */
        virtual QString GetParameterName(const MetaInfo &pinInfo) {return "";}

        /// the current model index
        QPersistentModelIndex modelIndex;

        /// true if the object is parked (no rendering)
        bool parked;

        /// true if the object is rendered at double precision
        bool doublePrecision;

        /// true if the object is programmable
        /// \todo cleaup container and maincontainer and remove this
        bool listenProgramChanges;

        virtual void ProgramToStream (int progId, QDataStream &out);
        virtual void ProgramFromStream (int progId, QDataStream &in);

        /// global object mutex
        DMutex objMutex;

    protected:

        /// pointer to the MainHost
        MainHost *myHost;

        /// list of all the pins
        QList<PinsList*>pinLists;

        /// list og audio pin in
        PinsList *listAudioPinIn;

        /// list of audio pin out
        PinsList *listAudioPinOut;

        /// list of midi pin in
        PinsList *listMidiPinIn;

        /// list of midi pin out
        PinsList *listMidiPinOut;

        /// list of parameters input
        PinsList *listParameterPinIn;

        /// list of parameters output
        PinsList *listParameterPinOut;

        /// list of programs
        hashPrograms listPrograms;

        /// the index the object had when the project was saved
        quint32 savedIndex;

        /// true if sleeping
        bool sleep;

        /// pointer to the currently loaded program
        ObjectProgram *currentProgram;

        /// the current program is
        int currentProgId;

        /// true if the object is closed or is closing
        bool closed;

    signals:
        void ShowEditorWindow();
        /// Sent to the editor window when we want to close it
        void HideEditorWindow();

        void UndoStackPush(QUndoCommand *cmd);

    public slots:
        void SuspendIfParked() {if(!parked) return; SetSleep(true);}
        void Suspend() {SetSleep(true);}
        void Resume() {SetSleep(false);}
        virtual void SaveProgram();
        virtual void UnloadProgram();
        virtual void LoadProgram(int prog);
        void OnProgramDirty();

        /// set the buffer size
        virtual void SetBufferSize(unsigned long size);

        /// set the sampling rate
        virtual void SetSampleRate(float rate=44100.0) {}

        virtual void OnParameterChanged(const MetaInfo &pinInfo, float value);

        void ToggleEditor(bool visible);

        /// to show the editor window from another thread
        virtual void OnShowEditor() {}

        /// to hide the editor window from another thread
        virtual void OnHideEditor() {}

        virtual void UserRemovePin(const MetaInfo &info);
        virtual void UserAddPin(const MetaInfo &info);

        virtual void SetEditorWnd(QWidget *wnd) {}
    };
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // CONNECTABLEOBJECT_H
