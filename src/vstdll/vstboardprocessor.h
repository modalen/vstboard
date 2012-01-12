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

#ifndef VSTBOARDPROCESSOR_H
#define VSTBOARDPROCESSOR_H

#include "public.sdk/source/vst/vstaudioeffect.h"

#include <QObject>
#include <QtGui/QApplication>
#include <QTranslator>
#include "connectables/vstaudiodevicein.h"
#include "connectables/vstaudiodeviceout.h"
#include "connectables/vstmididevice.h"
#include "connectables/vstautomation.h"
#include "mainhost.h"
#include "settings.h"

namespace Steinberg {
namespace Vst {

class MainHostVst;
class VstBoardProcessor : public MainHost, public AudioEffect
{
    Q_OBJECT
public:
        VstBoardProcessor (Settings *settings, QObject *parent = 0);

        tresult PLUGIN_API initialize (FUnknown* context);
        tresult PLUGIN_API terminate ();
        tresult PLUGIN_API setBusArrangements (SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts);

        tresult PLUGIN_API setActive (TBool state);
        tresult PLUGIN_API process (ProcessData& data);

        static FUnknown* createInstance (void*) {
            Settings *set = new Settings("plugin/",qApp);
            return (IAudioProcessor*)new VstBoardProcessor (set);
        }

protected:
        QApplication *myApp;

//        ParamValue delay;
//        float** buffer;
//        int32 bufferPos;

public slots:
        void Render();
};

}}

#endif // VSTBOARDPROCESSOR_H
