#ifndef SCRIPT_H
#define SCRIPT_H

#include "object.h"
#include "views/scripteditor.h"

namespace Connectables {

    class Script : public Object
    {
    Q_OBJECT

    public:
        Script(MainHost *host, MetaData &info);
        virtual ~Script();
        bool Open();
        bool Close();
        void Render();
        Pin* CreatePin(MetaData &info);

    protected:
        /// list of values used by the editor pin (0 and 1)
        QList<QVariant>listEditorVisible;

        QString objScriptName;
        QString scriptText;
        QString comiledScript;
        View::ScriptEditor *editorWnd;
        static DMutex mutexScript;

        QScriptValue scriptThisObj;
        QScriptValue objScript;
        QScriptValue openScript;
        QScriptValue renderScript;

    signals :
        void _dspMsg(const QString &title, const QString &str);
        void ScriptChanged(const QString &txt);

    public slots:
        void SaveProgram();
        void LoadProgram(int prog);
        void ReplaceScript(const QString &str);
        void alert(const QString &str);
        void OnShowEditor();
        void OnHideEditor();
        void OnEditorClosed();
        void EditorDestroyed();
        void SetEditorWnd(QWidget *wnd);

    private slots:
        void DspMsg(const QString &title, const QString &str);

        friend class View::ScriptEditor;
    };
}

#endif // SCRIPT_H
