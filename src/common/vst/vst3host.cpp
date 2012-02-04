#include "vst3host.h"

Vst3Host::Vst3Host()
{
}

Steinberg::tresult PLUGIN_API Vst3Host::getName (Steinberg::Vst::String128 name)
{
    Steinberg::String str ("VstBoard");
    str.copyTo (name, 0, 127);
    return Steinberg::kResultTrue;
}
