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

#ifndef AUDIODEVICEOUT_H
#define AUDIODEVICEOUT_H

//#include "precomp.h"
#include "connectables/objects/object.h"
#include "objectinfo.h"
#include "circularbuffer.h"

class AudioBuffer;

namespace Connectables {

    class AudioDevice;
    class AudioDeviceOut : public Object
    {
    Q_OBJECT
    public:
        AudioDeviceOut(MainHost *myHost, MetaInfo &info);
        ~AudioDeviceOut();    
        bool Open();
        bool Close();
        void SetParentDevice( AudioDevice *device );
        void SetRingBufferFromPins(QList<CircularBuffer*>listCircularBuffers);
//        QStandardItem *GetFullItem();

    protected:
        /// pointer to the linked AudioDevice
        AudioDevice *parentDevice;

//    friend class AudioDevice;
    };

}

#endif // AUDIODEVICEOUT_H
