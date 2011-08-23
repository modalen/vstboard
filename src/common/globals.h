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

#ifndef GLOBALS_H
#define GLOBALS_H

//#include "precomp.h"

#define EMPTY_PROGRAM 0xffff
#define TEMP_PROGRAM 0xfffe

#define SETUP_FILE_EXTENSION "vstbsetup"
#define PROJECT_FILE_EXTENSION "vstbproject"
#define NB_RECENT_FILES 5
#define VST_BANK_FILE_EXTENSION "fxb"
#define VST_PROGRAM_FILE_EXTENSION "fxp"

#define MME_DFAULT_FLAGS paWinMmeUseLowLevelLatencyParameters | paWinMmeDontThrottleOverloadedProcessingThread
#define MME_DEFAULT_BUFFER_SIZE 512
#define MME_DEFAULT_BUFFER_COUNT 8

#define WASAPI_DEFAULT_FLAGS 0
#define WASAPI_DEFAULT_INLATENCY 0
#define WASAPI_DEFAULT_OUTLATENCY 0

#define MIMETYPE_METAINFO QLatin1String("application/x-metainfo")
#define MIMETYPE_GROUP QLatin1String("application/x-groupsdata")
#define MIMETYPE_PROGRAM QLatin1String("application/x-programsdata")

namespace UserRoles {
    enum Enum {
        metaInfo = Qt::UserRole+1,
        value,
        id
    };
}

namespace PinMessage {
    enum Enum {
        ND,
        AudioBuffer,
        MidiMsg,
        ParameterValue
    };
}

namespace InsertionType {
    enum Enum {
        NoInsertion,
        InsertBefore,
        InsertAfter,
        Replace,
        AddBefore,
        AddAfter
    };
}

namespace RemoveType {
    enum Enum {
        RemoveWithCables,
        BridgeCables
    };
}
#endif // GLOBALS_H
