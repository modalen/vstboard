/**************************************************************************
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
**************************************************************************/

#ifndef VST_H
#define VST_H

#include <QObject>
#include <QtGui/QApplication>
#include "mainwindow.h"
#include "public.sdk/source/vst2.x/audioeffectx.h"
#include "gui.h"
#include "connectables/vstaudiodevicein.h"
#include "connectables/vstaudiodeviceout.h"

AudioEffect* createEffectInstance (audioMasterCallback audioMaster);

//-------------------------------------------------------------------------------------------------------
class Vst : public QObject, public AudioEffectX
{
Q_OBJECT
public:
        Vst (audioMasterCallback audioMaster);
        ~Vst ();

        // Processing
        virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
        virtual void processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames);

        // Program
        virtual void setProgramName (char* name);
        virtual void getProgramName (char* name);

        // Parameters
        virtual void setParameter (VstInt32 index, float value);
        virtual float getParameter (VstInt32 index);
        virtual void getParameterLabel (VstInt32 index, char* label);
        virtual void getParameterDisplay (VstInt32 index, char* text);
        virtual void getParameterName (VstInt32 index, char* text);

        virtual bool getEffectName (char* name);
        virtual bool getVendorString (char* text);
        virtual bool getProductString (char* text);
        virtual VstInt32 getVendorVersion ();

        bool addDeviceIn(Connectables::VstAudioDeviceIn *dev);
        bool addDeviceOut(Connectables::VstAudioDeviceOut *dev);
        bool removeDeviceIn(Connectables::VstAudioDeviceIn *dev);
        bool removeDeviceOut(Connectables::VstAudioDeviceOut *dev);

        void open();
        void close();
        void setSampleRate(float sampleRate);
        void setBlockSize (VstInt32 blockSize);
        void suspend();
        void resume();
        VstPlugCategory getPlugCategory() {return kPlugCategEffect;}

protected:
        QApplication *myApp;
        MainHost *myHost;
        MainWindow *myWindow;

        char programName[kVstMaxProgNameLen + 1];

        QList<Connectables::VstAudioDeviceIn*>lstDeviceIn;
        QList<Connectables::VstAudioDeviceOut*>lstDeviceOut;
        VstInt32 bufferSize;

        QMutex mutexDevices;

        Gui *qEditor;

};


#endif // VST_H
