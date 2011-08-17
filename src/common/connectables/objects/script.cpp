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


#include "script.h"
#include "mainhost.h"
#include "mainwindow.h"

using namespace Connectables;

DMutex Script::mutexScript;

Script::Script(MainHost *host, MetaInfo &info) :
    Object(host,info),
    editorWnd(0)
{
    SetName("Script");
    setObjectName( QString("objScript%1").arg(ObjId()) );
    objScriptName = objectName();
    objScriptName.append("sc");

    scriptThisObj = myHost->scriptEngine->newQObject(this);
    //myHost->scriptEngine->globalObject().setProperty(objScriptName, scriptThisObj);

    listEditorVisible << "hide";
    listEditorVisible << "show";
    listParameterPinIn->AddPin(FixedPinNumber::editorVisible);

    connect(this, SIGNAL(_dspMsg(QString,QString)),
            this, SLOT(DspMsg(QString,QString)),
            Qt::QueuedConnection);

    QMetaObject::invokeMethod(myHost->mainWindow,"CreateNewScriptEditor",
                               Qt::BlockingQueuedConnection,
                               Q_ARG(QObject*, this));
}

void Script::SetEditorWnd(QWidget *wnd)
{
    if(editorWnd) {
        LOG("editor already set"<<toStringFull());
        return;
    }

    editorWnd = static_cast<View::ScriptEditor*>(wnd);

    editorWnd->SetObject(this);
    connect(editorWnd, SIGNAL(Execute(QString)),
            this,SLOT(ReplaceScript(QString)));

    connect(this, SIGNAL(ShowEditorWindow()),
            editorWnd, SLOT(show()));
    connect(this,SIGNAL(HideEditorWindow()),
            editorWnd,SLOT(hide()));
    connect(editorWnd, SIGNAL(Hide()),
            this, SLOT(OnEditorClosed()));
    connect(editorWnd, SIGNAL(destroyed()),
            this, SLOT(EditorDestroyed()));
    connect(this, SIGNAL(ScriptChanged(QString)),
            editorWnd, SLOT(SetScript(QString)));

}

bool Script::Open()
{
    if(scriptText.isEmpty()) {
        scriptText = "\
({\n\
open: function(obj) {\n\
    obj.listParameterPinIn.nbPins=1;\n\
    obj.listParameterPinOut.nbPins=1;\n\
    obj.listAudioPinIn.nbPins=1;\n\
    obj.listAudioPinOut.nbPins=1;\n\
},\n\
\n\
render: function(obj) {\n\
    obj.ParamOut0.value=obj.ParamIn0.value;\n\
    obj.AudioOut0.buffer=obj.AudioIn0.buffer;\n\
}\n\
})";
    }

    emit ScriptChanged(scriptText);

    QMutexLocker l(&mutexScript);

    QScriptSyntaxCheckResult chk = myHost->scriptEngine->checkSyntax(scriptText);
    if(chk.state()!=QScriptSyntaxCheckResult::Valid) {
        comiledScript="";
        DspMsg(tr("Script syntax error"),
               tr("line %1\n%2").arg(chk.errorLineNumber()).arg(chk.errorMessage()));
        return true;
    }

//    comiledScript = QString( "function %1class(t) { obj=t; %2 }  %1m = new %1class(%1);" ).arg(objScriptName).arg(scriptText);
//    QScriptValue result = myHost->scriptEngine->evaluate(comiledScript);

//    myHost->scriptEngine->evaluate( objScriptName+"m.open();" );

    objScript = myHost->scriptEngine->evaluate(scriptText);
    if(myHost->scriptEngine->hasUncaughtException()) {
        comiledScript="";
        int line = myHost->scriptEngine->uncaughtExceptionLineNumber();
        DspMsg(tr("Script exception"),
               tr("line %1\n%2").arg(line).arg(objScript.toString()));
        return true;
    }

    openScript = objScript.property("open");
    renderScript = objScript.property("render");

    QScriptValue result = openScript.call(objScript, QScriptValueList() << scriptThisObj);
    if(myHost->scriptEngine->hasUncaughtException()) {
        comiledScript="";
        int line = myHost->scriptEngine->uncaughtExceptionLineNumber();
        DspMsg(tr("Script exception"),
               tr("line %1\n%2").arg(line).arg(result.toString()));
        return true;
    }

    return true;
}

Script::~Script()
{
    Close();
    if(currentProgId!=EMPTY_PROGRAM)
        delete currentProgram;
}

bool Script::Close()
{
   if(editorWnd) {
        editorWnd->disconnect();
        editorWnd->SetObject(0);
        disconnect(editorWnd);
        QTimer::singleShot(0,editorWnd,SLOT(close()));
        editorWnd=0;
    }
    return true;
}

void Script::Render()
{
    mutexScript.lock();

    foreach(Pin *pin, listAudioPinIn->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
        static_cast<AudioPin*>(pin)->NewRenderLoop();
    }
    foreach(Pin *pin, listAudioPinOut->listPins) {
        static_cast<AudioPin*>(pin)->NewRenderLoop();
    }

    QScriptValue result = renderScript.call(objScript, QScriptValueList() << scriptThisObj);

    if(!comiledScript.isEmpty()) {
        QScriptValue result = myHost->scriptEngine->evaluate( objScriptName+"m.render();" );
        if(myHost->scriptEngine->hasUncaughtException()) {
            comiledScript="";

            int line = myHost->scriptEngine->uncaughtExceptionLineNumber();
            emit _dspMsg(
                tr("Script exception"),
                tr("line %1\n%2").arg(line).arg(result.toString())
            );
        }
    }

    foreach(Pin *pin, listAudioPinOut->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
        static_cast<AudioPin*>(pin)->SendAudioBuffer();
    }

    mutexScript.unlock();
}

void Script::alert(const QString &str)
{
    emit _dspMsg(objectName(),str);
}

void Script::DspMsg(const QString &title, const QString &str)
{
    QMetaObject::invokeMethod(myHost->mainWindow,"DisplayMessage",
            Qt::BlockingQueuedConnection,
            Q_ARG(QMessageBox::Icon, QMessageBox::Critical),
            Q_ARG(QString, title),
            Q_ARG(QString, str),
            Q_ARG(QMessageBox::StandardButtons, QMessageBox::Ok),
            Q_ARG(QMessageBox::StandardButton, QMessageBox::Ok)
        );
}

void Script::ReplaceScript(const QString &str)
{
    scriptText = str;
    emit ScriptChanged(scriptText);
    OnProgramDirty();
    Open();
}

Pin* Script::CreatePin(MetaInfo &info)
{
    Pin *newPin = Object::CreatePin(info);
    if(newPin)
        return newPin;

    if(info.Meta(MetaInfos::Media).toInt() == MediaTypes::Parameter) {
        switch(info.Meta(MetaInfos::Direction).toInt()) {
            case Directions::Input :
                if(info.Meta(MetaInfos::PinNumber).toInt() == FixedPinNumber::editorVisible) {
                    info.SetName(tr("Editor"));
                    ParameterPin *newPin = new ParameterPin(this,info,"hide",&listEditorVisible);
                    newPin->SetLimitsEnabled(false);
                    return newPin;
                }
                info.SetName( QString("ParamIn%1").arg(info.Meta(MetaInfos::PinNumber).toInt()) );
                break;
            case Directions::Output :
                info.SetName( QString("ParamOut%1").arg(info.Meta(MetaInfos::PinNumber).toInt()) );
        }

        newPin = new ParameterPin(this,info,0);
        newPin->SetNameCanChange(true);
        return newPin;
    }

    return 0;
}

void Script::OnShowEditor()
{
    if(!editorWnd || editorWnd->isVisible())
        return;

    emit ShowEditorWindow();
//    editorWnd->SetScript(scriptText);
}

void Script::OnHideEditor()
{
    if(!editorWnd || !editorWnd->isVisible())
        return;
    emit HideEditorWindow();
}

void Script::OnEditorClosed()
{
    ToggleEditor(false);
}

void Script::EditorDestroyed()
{
    editorWnd=0;
    ToggleEditor(false);
}

void Script::SaveProgram()
{
    if(!currentProgram || !currentProgram->IsDirty())
        return;

    Object::SaveProgram();

    currentProgram->listOtherValues.insert(0, scriptText);
}

void Script::LoadProgram(int prog)
{
    Object::LoadProgram(prog);
    if(!currentProgram)
        return;

    scriptText = currentProgram->listOtherValues.value(0,"").toString();
//    if(editorWnd)
//        editorWnd->SetScript(scriptText);
    Open();

    if(editorWnd && editorWnd->isVisible())
        emit ScriptChanged(scriptText);
}

