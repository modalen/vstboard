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

#include "vstmididevice.h"
#include "globals.h"
#include "mainhostvst.h"
#include "vst.h"

using namespace Connectables;

VstMidiDevice::VstMidiDevice(MainHost *myHost, MetaData &info) :
    Object(myHost, info),
    nbIn(0),
    nbOut(0)
{
}

VstMidiDevice::~VstMidiDevice()
{
    nbIn=0;
    nbOut=0;
    Close();
}

void VstMidiDevice::Render()
{
    if(nbIn>0) {
        objMutex.lock();

        foreach(long msg, midiQueue) {

            foreach(Pin *pin,listMidiPinOut->listPins) {
                //static_cast<MidiPinOut*>(pin)->SendMsg(PinMessage::MidiMsg,(void*)&buffer.message);
                static_cast<MidiPinOut*>(pin)->SendMsg(PinMessage::MidiMsg,(void*)&msg);
            }
        }
        midiQueue.clear();
        objMutex.unlock();
    }
}

void VstMidiDevice::MidiMsgFromInput(long msg) {
    if(nbOut>0) {
        objMutex.lock();
        midiQueue << msg;
        objMutex.unlock();
    }
}

bool VstMidiDevice::Close()
{
    if(nbIn>0)
        static_cast<MainHostVst*>(myHost)->myVstPlugin->removeMidiIn(this);

    if(nbOut>0)
        static_cast<MainHostVst*>(myHost)->myVstPlugin->removeMidiOut(this);

    return Object::Close();
}

bool VstMidiDevice::Open()
{
    nbIn = GetMetaData<int>(metaT::nbInputs);
    nbOut = GetMetaData<int>(metaT::nbOutputs);

    if(nbIn>0)
        static_cast<MainHostVst*>(myHost)->myVstPlugin->addMidiIn(this);

    if(nbOut>0)
        static_cast<MainHostVst*>(myHost)->myVstPlugin->addMidiOut(this);

    listMidiPinOut->ChangeNumberOfPins(nbIn);
    listMidiPinIn->ChangeNumberOfPins(nbOut);

    Object::Open();
    return true;
}
