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

#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <QWaitCondition>

#include "precomp.h"
#include "connectables/object.h"
#include "connectables/objectinfo.h"
#include "portaudio.h"
#if WIN32
#include "pa_win_wmme.h"
#include "pa_win_ds.h"
#endif
#include "../circularbuffer.h"

#ifndef bzero
#define bzero(memArea, len)  memset((memArea), 0, (len))
#endif

class MainHostHost;
namespace Connectables {

    class AudioDeviceIn;
    class AudioDeviceOut;
    class AudioDevice : public QObject
    {
        Q_OBJECT
    public:
        AudioDevice(MainHostHost *myHost,const ObjectInfo &info, QObject *parent=0);
        ~AudioDevice();

        bool Open();
        bool Close();
        float GetCpuUsage();
        bool SetObjectInput(AudioDeviceIn *obj);
        bool SetObjectOutput(AudioDeviceOut *obj);
        void SetSleep(bool sleeping);
        static bool FindDeviceByName(ObjectInfo &objInfo, PaDeviceInfo *devInfo=0);

        /// global audio devices mutex
        static QMutex listDevMutex;

    protected:

        static int paCallback( const void *inputBuffer, void *outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo* timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void *userData );

        static void paStreamFinished( void* userData );

        bool OpenStream(double sampleRate);
        bool CloseStream();

        void DeleteCircualBuffers();

        /// true if the device is currently closing
        bool isClosing;

        /// true if the AudioDeviceOut is closing
        bool devOutClosing;

        /// current sample rate
        float sampleRate;

        /// current buffre size
        unsigned long bufferSize;

        /// pointer to PortAudio stream
        PaStream *stream;

        /// PortAudio device informations
        PaDeviceInfo devInfo;

        /// objcet description
        ObjectInfo objInfo;

        /// pointer to the AudioDeviceIn, can be null
        AudioDeviceIn *devIn;

        /// pointer to the AudioDeviceOut, can be null
        AudioDeviceOut *devOut;

        /// true if the device is closed
        bool closed;

        /// mutex for this device
        QMutex devicesMutex;

#if WIN32
        /// windows mme stream options
        PaWinMmeStreamInfo wmmeStreamInfo;

        /// windows directsound stream options
        PaWinDirectSoundStreamInfo directSoundStreamInfo;
#endif
        /// list of input ring buffers
        QList<CircularBuffer*>listCircularBuffersIn;

        /// list of output ring buffers
        QList<CircularBuffer*>listCircularBuffersOut;

        /// pointer to the MainHost
        MainHostHost *myHost;

        /// the number of opened devices
        static int countInputDevices;

        /// the number of devices ready to render. launch a rendering loop when all the devices are ready
        static int countDevicesReady;

        /// check if this device has been counted in the list of ready devices
        bool bufferReady;

    signals:
        /*!
          emitted when the device is opened or closed, used by AudioDevices
          \param objInfo object description
          \param inUse true if the device is in use
          */
        void InUseChanged(const ObjectInfo &objInfo, bool inUse);

    public slots:
        void SetSampleRate(float rate=44100.0);
        void DeleteIfUnused();

        friend class AudioDeviceIn;
        friend class AudioDeviceOut;
    };
}

#endif // AUDIODEVICE_H
