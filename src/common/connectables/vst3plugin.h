#ifndef VST3PLUGIN_H
#define VST3PLUGIN_H

#include "object.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "public.sdk/source/vst/hosting/processdata.h"

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
    bool hasEditor;
    Steinberg::Vst::HostProcessData processData;

    QList<QVariant>listEditorVisible;
    QList<QVariant>listIsLearning;
    QList<QVariant>listBypass;
    QList<QVariant>listValues;
};
}
#endif // VST3PLUGIN_H
