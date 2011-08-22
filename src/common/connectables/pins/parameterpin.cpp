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
ParameterPin::ParameterPin(Object *parent, MetaPin &info, float defaultValue) :
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
    SetMeta(metaT::StepSize, stepSize );
    SetMeta(metaT::DefaultValue,defaultValue);
    SetLimitsEnabled(true);

    internValue = defaultValue;
    loading=true;
    ChangeValue(defaultValue,true);
    loading=false;
}

//parameter is a int with a list of possible values
ParameterPin::ParameterPin(Object *parent, MetaPin &info, const QVariant &defaultVariantValue, QList<QVariant> *listValues) :
        Pin(parent,info),
        listValues(listValues),
        defaultValue( .0f ),
        loading(false),
        dirty(false),
        outStepIndex(0),
        outValue(.0f)
{
    stepSize=1.0f/(listValues->size()-1);
    SetMeta(metaT::StepSize, stepSize );
    SetLimitsEnabled(true);

    internStepIndex=listValues->indexOf(defaultVariantValue);
    defaultIndex=internStepIndex;
    SetMeta(metaT::ValueStep,internStepIndex);
    SetMeta(metaT::DefaultValueStep,defaultIndex);

    ChangeValue(internStepIndex*stepSize,true);
    loading=false;
}

void ParameterPin::SetLimitsEnabled(bool enable)
{
    if(enable) {
        SetMeta(metaT::LimitEnabled,true);
        SetMeta(metaT::LimitInMin, .0f);
        SetMeta(metaT::LimitInMax, 1.0f);
        SetMeta(metaT::LimitOutMin, .0f);
        SetMeta(metaT::LimitOutMax, 1.0f);
    } else {
        DelMeta(metaT::LimitEnabled);
        DelMeta(metaT::LimitInMin);
        DelMeta(metaT::LimitInMax);
        DelMeta(metaT::LimitOutMin);
        DelMeta(metaT::LimitOutMax);
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
    SetMeta(metaT::Removable,true);
}

void ParameterPin::GetDefault(ObjectParameter &param)
{
    param.index=GetMetaData<int>(metaT::DefaultValueStep);//defaultIndex;
    param.value=GetMetaData<float>(metaT::DefaultValue);//defaultValue;
    param.visible = !GetMetaData<bool>(metaT::Hidden);
//    param.visible=defaultVisible;
}

void ParameterPin::GetValues(ObjectParameter &param)
{
    param.index=GetMetaData<float>(metaT::ValueStep);//internStepIndex;
    param.value=GetMetaData<float>(metaT::Value);
    param.visible=!GetMetaData<bool>(metaT::Hidden);
    param.limitInMin=GetMetaData<float>(metaT::LimitInMin);
    param.limitInMax=GetMetaData<float>(metaT::LimitInMax);
    param.limitOutMin=GetMetaData<float>(metaT::LimitOutMin);
    param.limitOutMax=GetMetaData<float>(metaT::LimitOutMax);
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

    SetMeta(metaT::Value,internValue);
    OnValueChanged();

    if(!fromObj)
        parent->OnParameterChanged( Meta(),outValue);
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

    SetMeta(metaT::ValueStep,internStepIndex);
    SetMeta(metaT::Value,FloatFromInt(index));
    OnValueChanged();

    if(!fromObj)
        parent->OnParameterChanged(Meta(),outValue);
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

    SetMeta(metaT::LimitInMin,param.limitInMin);
    SetMeta(metaT::LimitInMax,param.limitInMax);
    SetMeta(metaT::LimitOutMin,param.limitOutMin);
    SetMeta(metaT::LimitOutMax,param.limitOutMax);

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

    if(!loading && !dirty && GetMetaData<Directions::Enum>(metaT::Direction)==Directions::Input) {
        dirty=true;
        parent->OnProgramDirty();
    }

    if(!GetMetaData<bool>(metaT::Hidden)) {
        if(nameCanChange)
            SetName(parent->GetParameterName(Meta()));

        if(listValues)
            SetMeta(metaT::displayedText, QString("%1:%2").arg(ObjName()).arg(listValues->at(outStepIndex).toString()) );
    }

    if(GetMetaData<Directions::Enum>(metaT::Direction)==Directions::Output)
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
    float limitInMin = GetMetaData<float>(metaT::LimitInMin);
    float limitInMax = GetMetaData<float>(metaT::LimitInMax);
    float limitOutMin = GetMetaData<float>(metaT::LimitOutMin);
    float limitOutMax = GetMetaData<float>(metaT::LimitOutMax);

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
