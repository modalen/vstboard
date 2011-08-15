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

#ifndef PARAMETERPIN_H
#define PARAMETERPIN_H

#include "precomp.h"
#include "pin.h"
#include "connectables/objectprogram.h"
#include "connectables/objectparameter.h"

namespace Connectables {

    class Object;
    class ParameterPin : public Pin
    {
    Q_OBJECT
    Q_PROPERTY(QVariant value READ GetVariantValue WRITE SetVariantValue)

    public:
        ParameterPin(Object *parent, MetaInfo &info, float defaultValue);
        ParameterPin(Object *parent, MetaInfo &info, const QVariant &defaultVariantValue, QList<QVariant> *listValues);

        void ReceiveMsg(const PinMessage::Enum msgType,void *data=0);

        void ChangeValue(float val, bool fromObj=false);
        void ChangeValue(int index, bool fromObj=false);
        void ChangeValue(const QVariant &variant, bool fromObj=false);
        void Load(const ObjectParameter &param);

        void OnValueChanged();

        inline int GetIndex() {return outStepIndex;}
        QVariant GetVariantValue();
        void SetVariantValue(const QVariant &val);
        void GetDefault(ObjectParameter &param);
        void GetValues(ObjectParameter &param);
        void SetRemoveable();
        void SetLimitsEnabled(bool enable);

    private:
        inline int IntFromFloat(float val);
        inline float FloatFromInt(int step);
        float ScaleValue(float val);
        QList<QVariant> *listValues;
        int internStepIndex;
        float stepSize;
        float defaultValue;
        int defaultIndex;
        bool loading;
        bool dirty;

        int outStepIndex;
        float outValue;

    signals:
        void SetProgDirty();
    };
}

#endif // PARAMETERPIN_H
