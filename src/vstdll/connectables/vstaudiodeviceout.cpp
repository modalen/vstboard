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

#include "vstaudiodeviceout.h"
#include "globals.h"
#include "audiobuffer.h"
#include "mainhostvst.h"
#include "vst.h"

using namespace Connectables;

VstAudioDeviceOut::VstAudioDeviceOut(MainHost *myHost, MetaData &info) :
    Object(myHost, info)
{
}

VstAudioDeviceOut::~VstAudioDeviceOut()
{
    Close();
}

bool VstAudioDeviceOut::Close()
{
    static_cast<MainHostVst*>(myHost)->myVstPlugin->removeAudioOut(this);
    if(!Object::Close())
        return false;
    return true;
}

void VstAudioDeviceOut::SetBufferSize(unsigned long size)
{
    foreach(Pin *pin, listAudioPinIn->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->SetSize(size);
    }
}

bool VstAudioDeviceOut::Open()
{
    if(!static_cast<MainHostVst*>(myHost)->myVstPlugin->addAudioOut(this))
        return false;

    listAudioPinIn->ChangeNumberOfPins(2);
    SetBufferSize(myHost->GetBufferSize());
    Object::Open();
    return true;
}

void VstAudioDeviceOut::GetBuffers(float **buf, int &cpt, int sampleFrames)
{
    foreach(Pin *pin, listAudioPinIn->listPins) {
        AudioBuffer *abuf= static_cast<AudioPin*>(pin)->GetBuffer();
        abuf->ConsumeStack();
        abuf->DumpToBuffer(buf[cpt],sampleFrames);
        abuf->ResetStackCounter();
        cpt++;
    }
}

void VstAudioDeviceOut::GetBuffersD(double **buf, int &cpt, int sampleFrames)
{
    foreach(Pin *pin, listAudioPinIn->listPins) {
        AudioBuffer *abuf= static_cast<AudioPin*>(pin)->GetBuffer();
        abuf->ConsumeStack();
        abuf->DumpToBuffer(buf[cpt],sampleFrames);
        abuf->ResetStackCounter();
        cpt++;
    }
}

//QStandardItem *VstAudioDeviceOut::GetFullItem()
//{
//    QStandardItem *modelNode = Object::GetFullItem();
//    modelNode->setData(doublePrecision, UserRoles::isDoublePrecision);
//    return modelNode;
//}
