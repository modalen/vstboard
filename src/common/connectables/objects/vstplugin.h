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

#ifndef VSTPLUGIN_H
#define VSTPLUGIN_H

#include "object.h"
#include "vst/ceffect.h"
#include "views/vstshellselect.h"

namespace View {
    class VstPluginWindow;
}


namespace Connectables {

    class VstPlugin : public Object , public vst::CEffect
    {
    Q_OBJECT

    public:
        VstPlugin(MainHost *myHost, MetaInfo & info);
        ~VstPlugin();
        bool Open();
        bool Close();
        void Render();
        VstIntPtr OnMasterCallback(long opcode, long index, long value, void *ptr, float opt, long currentReturnCode);
        void SetSleep(bool sleeping);
        void MidiMsgFromInput(long msg);
        QString GetParameterName(const MetaInfo &pinInfo);
        inline AEffect* GetPlugin() {return pEffect;}

        static QMap<AEffect*,VstPlugin*>mapPlugins;
        static VstPlugin *pluginLoading;

        static View::VstShellSelect *shellSelectView;

        void SetContainerAttribs(const ObjectContainerAttribs &attr);
        void GetContainerAttribs(ObjectContainerAttribs &attr);
        Pin* CreatePin(MetaInfo &info);

        bool DropFile(const QString &filename);

        QDataStream & toStream (QDataStream &) const;
        bool fromStream (QDataStream &);

    protected:
        bool initPlugin();
        void processEvents(VstEvents* events);
        void onVstProgramChanged();
        float sampleRate;
        unsigned long bufferSize;
        VstEvents *listEvnts;

        /// list of values used by the editor pin (0 and 1)
        QList<QVariant>listEditorVisible;

        /// list of values used by the learn pin (off, learn, unlearn)
        QList<QVariant>listIsLearning;

        DMutex midiEventsMutex;
        QList<VstMidiEvent*>listVstMidiEvents;
        QList<QVariant>listValues;

        char *savedChunk;
        quint32 savedChunkSize;

    private:
        View::VstPluginWindow *editorWnd;

    signals:
        void WindowSizeChange(int newWidth, int newHeight);

    public slots:
        void SetBufferSize(unsigned long size);
        void SetSampleRate(float rate=44100.0);
        void RaiseEditor();
        void EditorDestroyed();
        void EditIdle();
        void OnParameterChanged(const MetaInfo &pinInfo, float value);
        void OnShowEditor();
        void OnHideEditor();
        void OnEditorClosed();
        bool LoadBank(const QString &filename);
        void SaveBank(const QString &filename);
        bool LoadProgram(const QString &filename);
        void SaveProgram(const QString &filename);
//        void TakeScreenshot();
        void UserRemovePin(const MetaInfo &info);
        void UserAddPin(const MetaInfo &info);

        void SetEditorWnd(QWidget *wnd);

        friend class View::VstPluginWindow;
        friend class View::VstShellSelect;
    };

}

#endif // VSTPLUGIN_H
