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

#ifndef MIDIDEFINES_H
#define MIDIDEFINES_H

namespace MidiConst {
     enum Enum {
        codeMask = 0xf0,
        channelMask = 0x0f,

        noteOff =   0x80, //key, velocity
        noteOn =    0x90, //key, velocity
        aftertouch = 0xa0, //key, value
        ctrl =      0xb0, //ctrl, value
        prog =      0xc0, //prog, nd
        chanpressure = 0xd0, //value, nd
        pitchbend = 0xe0, //lsb, msb
        other = 0xf0 //nd,nd
    };
}


#define MidiStatus(msg) ((msg) & 0xFF)
#define MidiData1(msg) (((msg) >> 8) & 0xFF)
#define MidiData2(msg) (((msg) >> 16) & 0xFF)

#define MidiMessage(status, data1, data2) \
         ((((data2) << 16) & 0xFF0000) | \
          (((data1) << 8) & 0xFF00) | \
          ((status) & 0xFF))

#endif // MIDIDEFINES_H
