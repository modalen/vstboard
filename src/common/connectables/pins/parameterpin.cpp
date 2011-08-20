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


#include "parameterpin.h"
#include "connectables/objects/object.h"
#include "globals.h"
#include "mainhost.h"

using namespace Connectables;

//parameter is a float
ParameterPin::ParameterPin(Object *parent, MetaData &info, float defaultValue) :
        Pin(parent,info),
        listValues(0),
        internStepIndex(0),
        defaultValue(defaultValue),
        defaultIndex(0),
        loading(false),
        dirty(false),
        outStepIndex(0),
        outValue(.0f)
{
    stepSize=.1f;
    data.SetMeta(metaT::StepSize, stepSize );
    data.SetMeta(metaT::DefaultValue,defaultValue);
    SetLimitsEnabled(true);

    internValue = defaultValue;
    loading=true;
    ChangeValue(defaultValue,true);
    loading=false;
}

//parameter is a int with a list of possible values
ParameterPin::ParameterPin(Object *parent, MetaData &info, const QVariant &defaultVariantValue, QList<QVariant> *listValues) :
        Pin(parent,info),
        listValues(listValues),
        defaultValue( .0f ),
        loading(false),
        dirty(false),
        outStepIndex(0),
        outValue(.0f)
{
    stepSize=1.0f/(listValues->size()-1);
    data.SetMeta(metaT::StepSize, stepSize );
    SetLimitsEnabled(true);

    internStepIndex=listValues->indexOf(defaultVariantValue);
    defaultIndex=internStepIndex;
    data.SetMeta(metaT::ValueStep,internStepIndex);
    data.SetMeta(metaT::DefaultValueStep,defaultIndex);

    ChangeValue(internStepIndex*stepSize,true);
    loading=false;
}

void ParameterPin::SetLimitsEnabled(bool enable)
{
    if(enable) {
        data.SetMeta(metaT::LimitEnabled,true);
        data.SetMeta(metaT::LimitInMin, .0f);
        data.SetMeta(metaT::LimitInMax, 1.0f);
        data.SetMeta(metaT::LimitOutMin, .0f);
        data.SetMeta(metaT::LimitOutMax, 1.0f);
    } else {
        data.DelMeta(metaT::LimitEnabled);
        data.DelMeta(metaT::LimitInMin);
        data.DelMeta(metaT::LimitInMax);
        data.DelMeta(metaT::LimitOutMin);
        data.DelMeta(metaT::LimitOutMax);
    }
}

void ParameterPin::ReceiveMsg(const PinMessage::Enum msgType,void *data)
{
    if(msgType == PinMessage::ParameterValue) {
        ChangeValue(*(float*)data);
    }
}

void ParameterPin::SetRemoveable()
{
    data.SetMeta(metaT::Removable,true);
}

void ParameterPin::GetDefault(ObjectParameter &param)
{
    param.index=data.GetMetaData<int>(metaT::DefaultValueStep);//defaultIndex;
    param.value=data.GetMetaData<float>(metaT::DefaultValue);//defaultValue;
    param.visible = !data.GetMetaData<bool>(metaT::Hidden);
//    param.visible=defaultVisible;
}

void ParameterPin::GetValues(ObjectParameter &param)
{
    param.index=data.GetMetaData<float>(metaT::ValueStep);//internStepIndex;
    param.value=data.GetMetaData<float>(metaT::Value);
    param.visible=!data.GetMetaData<bool>(metaT::Hidden);
    param.limitInMin=data.GetMetaData<float>(metaT::LimitInMin);
    param.limitInMax=data.GetMetaData<float>(metaT::LimitInMax);
    param.limitOutMin=data.GetMetaData<float>(metaT::LimitOutMin);
    param.limitOutMax=data.GetMetaData<float>(metaT::LimitOutMax);
}

//from float
void ParameterPin::ChangeValue(float val, bool fromObj)
{
    if(listValues) {
        //we got a float, but we need a int
        ChangeValue(IntFromFloat(val));
        return;
    }

    val = std::min(val,1.0f);
    val = std::max(val,.0f);

    float oldVal = outValue;

    internValue = val;
    outValue = ScaleValue(val);

    if(!loading && std::abs(oldVal-outValue)<0.001f)
        return;

    data.SetMeta(metaT::Value,internValue);
    OnValueChanged();

    if(!fromObj)
        parent->OnParameterChanged(info(),outValue);
}

//from int
void ParameterPin::ChangeValue(int index, bool fromObj)
{
    index = std::min(index,listValues->size()-1);
    index = std::max(index,0);

    int oldVal=outStepIndex;
    internValue = FloatFromInt(index);
    outValue = ScaleValue( internValue );

    internStepIndex = index;
    outStepIndex = IntFromFloat(outValue);

    if(!loading && oldVal==outStepIndex)
        return;

    data.SetMeta(metaT::ValueStep,internStepIndex);
    data.SetMeta(metaT::Value,FloatFromInt(index));
    OnValueChanged();

    if(!fromObj)
        parent->OnParameterChanged(info(),outValue);
}

//from variant
void ParameterPin::ChangeValue(const QVariant &variant, bool fromObj)
{
    if(listValues)
        ChangeValue(listValues->indexOf(variant),fromObj);
    else
        ChangeValue(variant.toFloat(),fromObj);
}

QVariant ParameterPin::GetVariantValue()
{
    if(!listValues)
        return outValue;
    return listValues->at(outStepIndex);
}

void ParameterPin::SetVariantValue(const QVariant &val)
{
    if(!listValues) {
        ChangeValue(val.toFloat());
        return;
    }

    int index=listValues->indexOf(val);
    if(index==-1)
        return;

    ChangeValue(listValues->value(index));
}

//load from program
void ParameterPin::Load(const ObjectParameter &param)
{
    loading = true;

    data.SetMeta(metaT::LimitInMin,param.limitInMin);
    data.SetMeta(metaT::LimitInMax,param.limitInMax);
    data.SetMeta(metaT::LimitOutMin,param.limitOutMin);
    data.SetMeta(metaT::LimitOutMax,param.limitOutMax);

    if(listValues)
        ChangeValue(param.index);
    else
        ChangeValue(param.value);

    SetVisible(param.visible);

    dirty = false;
    loading = false;
}

void ParameterPin::OnValueChanged()
{
    valueChanged=true;

    if(!loading && !dirty && data.GetMetaData<Directions::Enum>(metaT::Direction)==Directions::Input) {
        dirty=true;
        parent->OnProgramDirty();
    }

    if(!data.GetMetaData<bool>(metaT::Hidden)) {
        if(nameCanChange)
            SetName(parent->GetParameterName(info()));

        if(listValues)
            data.SetMeta(metaT::displayedText, QString("%1:%2").arg(Name()).arg(listValues->at(outStepIndex).toString()) );
    }

    if(data.GetMetaData<Directions::Enum>(metaT::Direction)==Directions::Output)
        SendMsg(PinMessage::ParameterValue,(void*)&outValue);
}

float ParameterPin::FloatFromInt(int step)
{
    return step*stepSize;
}

int ParameterPin::IntFromFloat(float val)
{
    return (int)( 0.5f + val/stepSize );
}

float ParameterPin::ScaleValue(float val)
{
    //scale value
    float limitInMin = data.GetMetaData<float>(metaT::LimitInMin);
    float limitInMax = data.GetMetaData<float>(metaT::LimitInMax);
    float limitOutMin = data.GetMetaData<float>(metaT::LimitOutMin);
    float limitOutMax = data.GetMetaData<float>(metaT::LimitOutMax);

    if(val>limitInMax)
        val=limitInMax;
    if(val<limitInMin)
        val=limitInMin;
    val-=limitInMin;
    if(limitInMax!=limitInMin)
        val/=(limitInMax-limitInMin);
    val*=(limitOutMax-limitOutMin);
    val+=limitOutMin;
    return val;
}
