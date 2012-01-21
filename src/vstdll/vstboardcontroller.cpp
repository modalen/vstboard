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

#include "vstboardcontroller.h"
#include "ids.h"
#include "pluginterfaces/base/ustring.h"
#include "gui.h"
#include "mainwindowvst.h"
#include "settings.h"

namespace Steinberg {
namespace Vst {

//-----------------------------------------------------------------------------
tresult PLUGIN_API VstBoardController::initialize (FUnknown* context)
{
    mainWindow=0;

    tresult result = EditController::initialize (context);
    if (result != kResultTrue)
        return result;

    QCoreApplication::setOrganizationName("CtrlBrk");
    QCoreApplication::setApplicationName("VstBoard");

    parameters.addParameter (STR16 ("Delay"), STR16 ("sec"), 0, 1, ParameterInfo::kCanAutomate, kDelayTag);

    return kResultTrue;
}

IPlugView* PLUGIN_API VstBoardController::createView (const char* name)
{
    if (name && strcmp (name, "editor") == 0)
    {
        ViewRect viewRect (0, 0, 850, 600);
        Gui* view = new Gui(&viewRect);
        Settings *set = new Settings("plugin/",qApp);
        mainWindow = new MainWindowVst(this,set);
        view->SetMainWindow(mainWindow);
        return view;
    }
    return 0;
}

void VstBoardController::editorAttached (EditorView* editor)
{
    Gui* view = dynamic_cast<Gui*> (editor);
    if (view)
        listGui << view;
}

void VstBoardController::editorRemoved (EditorView* editor)
{
    Gui* view = dynamic_cast<Gui*> (editor);
    if (view)
        listGui.removeAll(view);
}

tresult PLUGIN_API VstBoardController::notify (IMessage* message)
{
    if (!message)
        return kInvalidArgument;

    const void* data;
    uint32 size;
    if (message->getAttributes ()->getBinary ("data", data, size) == kResultOk)
    {
        if(!mainWindow)
            return kResultOk;

        if (!strcmp (message->getMessageID (), "msglist")) {
            QByteArray ba((char*)data,size);
            mainWindow->ReceiveMsg(message->getMessageID(),ba);
            return kResultOk;
        }

        if (!strcmp (message->getMessageID (), "msg")) {
            QByteArray ba((char*)data,size);
            mainWindow->ReceiveMsg( QVariant(ba).value<MsgObject>() );
            return kResultOk;
        }

        return kResultOk;
    }

    return EditController::notify(message);
}
}}
