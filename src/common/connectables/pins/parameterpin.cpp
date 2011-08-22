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
ParameterPin::ParameterPin(Object *parent, MetaInfo &info, float defaultValue) :
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
    SetMeta(MetaInfos::StepSize, stepSize );
    SetMeta(MetaInfos::DefaultValue,defaultValue);
    SetLimitsEnabled(true);

    internValue = defaultValue;
    loading=true;
    ChangeValue(defaultValue,true);
    loading=false;
}

//parameter is a int with a list of possible values
ParameterPin::ParameterPin(Object *parent, MetaInfo &info, const QVariant &defaultVariantValue, QList<QVariant> *listValues) :
        Pin(parent,info),
        listValues(listValues),
        defaultValue( .0f ),
        loading(false),
        dirty(false),
        outStepIndex(0),
        outValue(.0f)
{
    stepSize=1.0f/(listValues->size()-1);
    SetMeta(MetaInfos::StepSize, stepSize );
    SetLimitsEnabled(true);

    internStepIndex=listValues->indexOf(defaultVariantValue);
    defaultIndex=internStepIndex;
    SetMeta(MetaInfos::ValueStep,internStepIndex);
    SetMeta(MetaInfos::DefaultValueStep,defaultIndex);

    ChangeValue(internStepIndex*stepSize,true);
    loading=false;
}

void ParameterPin::SetLimitsEnabled(bool enable)
{
    if(enable) {
        SetMeta(MetaInfos::LimitEnabled,true);
        SetMeta(MetaInfos::LimitInMin, .0f);
        SetMeta(MetaInfos::LimitInMax, 1.0f);
        SetMeta(MetaInfos::LimitOutMin, .0f);
        SetMeta(MetaInfos::LimitOutMax, 1.0f);
    } else {
        DelMeta(MetaInfos::LimitEnabled);
        DelMeta(MetaInfos::LimitInMin);
        DelMeta(MetaInfos::LimitInMax);
        DelMeta(MetaInfos::LimitOutMin);
        DelMeta(MetaInfos::LimitOutMax);
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
    SetMeta(MetaInfos::Removable,true);
}

void ParameterPin::GetDefault(ObjectParameter &param)
{
    param.index=Meta(MetaInfos::DefaultValueStep).toInt();//defaultIndex;
    param.value=Meta(MetaInfos::DefaultValue).toFloat();//defaultValue;
    param.visible = !Meta(MetaInfos::Hidden).toBool();
//    param.visible=defaultVisible;
}

void ParameterPin::GetValues(ObjectParameter &param)
{
    param.index=Meta(MetaInfos::ValueStep).toFloat();//internStepIndex;
    param.value=Meta(MetaInfos::Value).toFloat();
    param.visible=!Meta(MetaInfos::Hidden).toBool();
    param.limitInMin=Meta(MetaInfos::LimitInMin).toFloat();
    param.limitInMax=Meta(MetaInfos::LimitInMax).toFloat();
    param.limitOutMin=Meta(MetaInfos::LimitOutMin).toFloat();
    param.limitOutMax=Meta(MetaInfos::LimitOutMax).toFloat();
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
    LOG(std::abs(oldVal-outValue));
    if(!loading && std::abs(oldVal-outValue)<0.0001f)
        return;

    SetMeta(MetaInfos::Value,internValue);
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

    SetMeta(MetaInfos::ValueStep,internStepIndex);
    SetMeta(MetaInfos::Value,FloatFromInt(index));
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

    SetMeta(MetaInfos::LimitInMin,param.limitInMin);
    SetMeta(MetaInfos::LimitInMax,param.limitInMax);
    SetMeta(MetaInfos::LimitOutMin,param.limitOutMin);
    SetMeta(MetaInfos::LimitOutMax,param.limitOutMax);

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

    if(!loading && !dirty && Meta(MetaInfos::Direction).toInt()==Directions::Input) {
        dirty=true;
        parent->OnProgramDirty();
    }

    if(!Meta(MetaInfos::Hidden).toBool()) {
        if(nameCanChange)
            SetName(parent->GetParameterName(info()));

        if(listValues)
            SetMeta(MetaInfos::displayedText, QString("%1:%2").arg(Name()).arg(listValues->at(outStepIndex).toString()) );
    }

    if(Meta(MetaInfos::Direction).toInt()==Directions::Output)
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
    float limitInMin = Meta(MetaInfos::LimitInMin).toFloat();
    float limitInMax = Meta(MetaInfos::LimitInMax).toFloat();
    float limitOutMin = Meta(MetaInfos::LimitOutMin).toFloat();
    float limitOutMax = Meta(MetaInfos::LimitOutMax).toFloat();

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
