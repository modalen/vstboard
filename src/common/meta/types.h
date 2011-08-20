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

#ifndef TYPES_H
#define TYPES_H

namespace metaT {
    enum Enum {
        ND,

        INT_BEGIN = 1,
        ObjId,
        ObjName,
        ParentId,
        ParentObjId,
        ContainerId,
        ObjType,
        Direction,
        Media,
        BridgeMedia,
        ObjIdIn,
        ObjIdOut,
        devId,
        apiId,
        duplicateNamesCounter,
        nbInputs,
        nbOutputs,
        PinNumber,
        ValueStep,
        DefaultValueStep,
        EditorVScroll,
        EditorHScroll,
        INT_END,

        FLOAT_BEGIN = 30,
        Value,
        DefaultValue,
        StepSize,
        LimitInMin,
        LimitInMax,
        LimitOutMin,
        LimitOutMax,
        FLOAT_END,

        BOOL_BEGIN = 50,
        Hidden,
        Removable,
        Bridge,
        Dirty,
        DoublePrecision,
        EditorVisible,
        LimitEnabled,
        BOOL_END,

        STRING_BEGIN = 70,
        Filename,
        devName,
        apiName,
        errorMessage,
        bankFile,
        programFile,
        displayedText,
        STRING_END,

        OTHER_BEGIN = 90,
        Position,
        EditorSize,
        EditorPosition,
        OTHER_END,

        END
    };

//#ifndef QT_NO_DEBUG
    const char* INTNames[];
    const char* FLOATNames[];
    const char* BOOLNames[];
    const char* STRINGNames[];
    const char* OTHERNames[];
//#endif

}

namespace MetaType {
    enum Enum {
        ND,
        object,
        container,
        bridge,
        listPin,
        pin,
        cable
    };
}

namespace ObjTypes {
    enum Enum {
        ND,
        Dummy,
        VstPlugin,
        HostController,
        MidiToAutomation,
        MidiSender,
        Script,
        AudioInterface,
        VstAutomation,
        MidiInterface
    };
}

namespace MediaTypes {
    enum Enum {
        ND,
        Audio,
        Midi,
        Parameter,
        Bridge
    };
}

namespace Directions {
    enum Enum {
        ND,
        Input,
        Output,
        Send,
        Return
    };
}

namespace FixedObjIds {
    enum Enum {
        ND,
        mainContainer,
        hostContainer,
        hostContainerIn,
        hostContainerOut,
        hostContainerSend,
        hostContainerReturn,
        projectContainer,
        projectContainerIn,
        projectContainerOut,
        projectContainerSend,
        projectContainerReturn,
        programContainer,
        programContainerIn,
        programContainerOut,
        programContainerSend,
        programContainerReturn,
        groupContainer,
        groupContainerIn,
        groupContainerOut,
        groupContainerSend,
        groupContainerReturn,
        parkingContainer
    };
}

#endif // TYPES_H
