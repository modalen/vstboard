#ifndef VST3HOST_H
#define VST3HOST_H

#include "public.sdk/source/vst/hosting/hostclasses.h"

class Vst3Host : public Steinberg::Vst::HostApplication
{
public:
    Vst3Host();
    Steinberg::tresult PLUGIN_API getName (Steinberg::Vst::String128 name);
};

#endif // VST3HOST_H
