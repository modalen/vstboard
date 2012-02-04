#ifndef VST3PLUGIN_H
#define VST3PLUGIN_H

#include "object.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "public.sdk/source/vst/hosting/processdata.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"

namespace Connectables {

class Vst3Plugin : public Object
{
    Q_OBJECT
public:
    explicit Vst3Plugin(MainHost *host, int index, const ObjectInfo &info);
    ~Vst3Plugin();
    bool Open();
    bool Close();
    void Render();
    Pin* CreatePin(const ConnectionInfo &info);
private:
    void Unload();
    QLibrary *pluginLib;
    Steinberg::IPluginFactory* factory;
    Steinberg::Vst::IComponent* processorComponent;
    Steinberg::Vst::IEditController* editController;
    Steinberg::Vst::IAudioProcessor* audioEffect;
    Steinberg::Vst::HostProcessData processData;
    Steinberg::Vst::ParameterChanges vstParamChanges;
    QMap<qint32,qint32>listParamQueue;

    bool hasEditor;

    QList<QVariant>listEditorVisible;
    QList<QVariant>listIsLearning;
    QList<QVariant>listBypass;
    QList<QVariant>listValues;

public slots:
    void OnParameterChanged(ConnectionInfo pinInfo, float value);
};
}
#endif // VST3PLUGIN_H
