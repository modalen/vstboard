/**************************************************************************
#    Copyright 2010 Rapha�l Fran�ois
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

#ifndef VSTAUTOMATION_H
#define VSTAUTOMATION_H

#include "../vstboard/precomp.h"
#include "connectables/object.h"

#define VST_AUTOMATION_DEFAULT_NB_PINS 2

namespace Connectables {


    class VstAutomation : public Object
    {
    Q_OBJECT
    public:
        explicit VstAutomation(MainHost *myHost,int index);
        ~VstAutomation();
        bool Open();
        bool Close();
        void Render();
        void ValueFromHost(int pinNum, float value);
        Pin* CreatePin(const ConnectionInfo &info, quint16 nb);
    protected:
        QList<QVariant> listValues;
        QHash<int,float>listChanged;
        int numberOfPins;

    public slots:
        void OnParameterChanged(ConnectionInfo pinInfo, float value);
    };
}
#endif // VSTAUTOMATION_H
