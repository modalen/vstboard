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
#include "public.sdk/source/main/pluginfactoryvst3.h"
#include "vstboardcontroller.h"
#include "vstboardprocessor.h"
#include "ids.h"
#include "vsttest.h"

#define stringPluginName "VstBoard"


BEGIN_FACTORY_DEF ("CtrlBrk",
                   "http://vstboard.blogspot.com",
                   "mailto:ctrlbrk76@gmail.com")

        DEF_CLASS2 (INLINE_UID_FROM_FUID(Steinberg::Vst::VstBoardProcessorUID),
                                PClassInfo::kManyInstances,
                                kVstAudioEffectClass,
                                stringPluginName,
                                Vst::kDistributable,
                                "Fx|Other",
                                "FULL_VERSION_STR",
                                kVstVersionString,
                                Steinberg::Vst::VstBoardProcessor::createInstance)

        DEF_CLASS2 (INLINE_UID_FROM_FUID(Steinberg::Vst::VstBoardControllerUID),
                                PClassInfo::kManyInstances,
                                kVstComponentControllerClass,
                                stringPluginName "Controller",	// controller name (could be the same than component name)
                                0,						// not used here
                                "",						// not used here
                                "FULL_VERSION_STR",		// Plug-in version (to be changed)
                                kVstVersionString,
                                Steinberg::Vst::VstBoardController::createInstance)

        // add Test Factory
        DEF_CLASS2 (INLINE_UID_FROM_FUID(Steinberg::Vst::VstBoardTestFactory::cid),
                                PClassInfo::kManyInstances,
                                kTestClass,
                                stringPluginName "Test Factory",
                                0,
                                "",
                                "",
                                "",
                                Steinberg::Vst::VstBoardTestFactory::createInstance)

END_FACTORY

bool InitModule () { return true; }
bool DeinitModule () { return true; }

