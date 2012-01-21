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

#include "vstboardprocessor.h"
#include "ids.h"

#include "projectfile/projectfile.h"
#include "views/configdialog.h"
#include "settings.h"

#include "connectables/vstaudiodevicein.h"
#include "connectables/vstaudiodeviceout.h"
#include "connectables/objectfactoryvst.h"
#include "msgobject.h"

namespace Steinberg {
namespace Vst {
VstBoardProcessor::VstBoardProcessor (Settings *settings,QObject *parent) :
    MainHost(settings,parent),
    AudioEffect()
{
        setControllerClass (VstBoardControllerUID);
        objFactory = new Connectables::ObjectFactoryVst(this);
}

tresult PLUGIN_API VstBoardProcessor::initialize (FUnknown* context)
{
    tresult result = AudioEffect::initialize (context);
    if (result != kResultTrue)
        return result;

    qRegisterMetaType<ConnectionInfo>("ConnectionInfo");
    qRegisterMetaType<ObjectInfo>("ObjectInfo");
    qRegisterMetaType<MsgObject>("MsgObject");
    qRegisterMetaType<int>("ObjType::Enum");
    qRegisterMetaType<QVariant>("QVariant");
    qRegisterMetaType<AudioBuffer*>("AudioBuffer*");

    qRegisterMetaTypeStreamOperators<ObjectInfo>("ObjectInfo");

    QCoreApplication::setOrganizationName("CtrlBrk");
    QCoreApplication::setApplicationName("VstBoard");

    Open();

    //load default setup file
    QString currentSetupFile = ConfigDialog::defaultSetupFile(settings);
    if(!currentSetupFile.isEmpty()) {
        if(!ProjectFile::LoadFromFile(this,currentSetupFile))
            currentSetupFile = "";
    }

    //load default project file
    QString currentProjectFile = ConfigDialog::defaultProjectFile(settings);
    if(!currentProjectFile.isEmpty()) {
        if(!ProjectFile::LoadFromFile(this,currentProjectFile))
            currentProjectFile = "";
    }

    addAudioInput (STR16 ("AudioInput"), SpeakerArr::kStereo);
    addAudioOutput (STR16 ("AudioOutput"), SpeakerArr::kStereo);

    return kResultTrue;
}

tresult PLUGIN_API VstBoardProcessor::terminate ()
{
    tresult result = AudioEffect::terminate ();
    if (result != kResultTrue)
        return result;

    return result;
}

tresult PLUGIN_API VstBoardProcessor::setBusArrangements (SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts)
{
        if (numIns == 1 && numOuts == 1 && inputs[0] == outputs[0])
                return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
        return kResultFalse;
}

tresult PLUGIN_API VstBoardProcessor::setActive (TBool state)
{
        SpeakerArrangement arr;
        if (getBusArrangement (kOutput, 0, arr) != kResultTrue)
                return kResultFalse;
        int32 numChannels = SpeakerArr::getChannelCount (arr);
        if (numChannels == 0)
                return kResultFalse;

        if (state)
        {

        }
        else
        {

        }
        return AudioEffect::setActive (state);
}

tresult PLUGIN_API VstBoardProcessor::process (ProcessData& data)
{
    Render();
    return kResultTrue;
}

tresult PLUGIN_API VstBoardProcessor::notify (IMessage* message) {
    if (!message)
        return kInvalidArgument;

    const void* data;
    uint32 size;
    if (message->getAttributes ()->getBinary ("data", data, size) == kResultOk)
    {
        if (!strcmp (message->getMessageID (), "msglist")) {
            QByteArray ba((char*)data,size);
            ReceiveMsg(message->getMessageID(),ba);
            return kResultOk;
        }

        if (!strcmp (message->getMessageID (), "msg")) {
            QByteArray ba((char*)data,size);
            ReceiveMsg( QVariant(ba).value<MsgObject>() );
            return kResultOk;
        }
    }

    return AudioEffect::notify(message);
}

void VstBoardProcessor::SendMsg(const MsgObject &msg)
{
    Steinberg::Vst::IMessage* message = allocateMessage();
    if (message)
    {
        message->setMessageID("msg");
        QByteArray br( QVariant::fromValue(msg).toByteArray() );
        message->getAttributes ()->setBinary ("data", br.data(), br.size());
        sendMessage(message);
    }
}

void VstBoardProcessor::SendMsg(const QString &type, const QVariant &data)
{
    Steinberg::Vst::IMessage* message = allocateMessage();
    if (message)
    {
        message->setMessageID("msglist");
        QByteArray br( data.toByteArray() );
        message->getAttributes ()->setBinary ("data", br.data(), br.size());
        sendMessage(message);
    }
}

}}
