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
#include "object.h"
#include "../globals.h"
#include "mainhost.h"

using namespace Connectables;

//parameter is a float
ParameterPin::ParameterPin(Object *parent, Directions::Enum direction, int number, float defaultValue, const QString &name, bool nameCanChange, bool isRemoveable, bool bridge) :
        Pin(parent,MediaTypes::Parameter,direction,number,bridge),
        listValues(0),
        stepIndex(0),
        defaultVisible(true),
        defaultValue(defaultValue),
        defaultIndex(0),
        loading(false),
        nameCanChange(nameCanChange),
        dirty(false),
        limitInMin(.0f),
        limitInMax(1.0f),
        limitOutMin(.0f),
        limitOutMax(1.0f),
        outStepIndex(0),
        outValue(.0f),
        limitsEnabled(true)
{
    SetVisible(true);
    connectInfo.isRemoveable=isRemoveable;
    value = defaultValue;
    setObjectName(name);
    loading=true;
    OnValueChanged( defaultValue );
    loading=false;
}

//parameter is a int with a list of possible values
ParameterPin::ParameterPin(Object *parent, Directions::Enum direction, int number, const QVariant &defaultVariantValue, QList<QVariant> *listValues, const QString &name, bool nameCanChange, bool isRemoveable, bool bridge) :
        Pin(parent,MediaTypes::Parameter,direction,number,bridge),
        listValues(listValues),
        defaultVisible(true),
        defaultValue( .0f ),
        loading(false),
        nameCanChange(nameCanChange),
        dirty(false),
        limitInMin(.0f),
        limitInMax(1.0f),
        limitOutMin(.0f),
        limitOutMax(1.0f),
        outStepIndex(0),
        outValue(.0f),
        limitsEnabled(true)
{
    SetVisible(true);
    connectInfo.isRemoveable=isRemoveable;
    setObjectName(name);
    stepSize=1.0f/(listValues->size()-1);
    stepIndex=listValues->indexOf(defaultVariantValue);
    defaultIndex=stepIndex;
    OnValueChanged( .0f);//stepIndex*stepSize );
    loading=false;
}

void ParameterPin::SetRemoveable()
{
    connectInfo.isRemoveable=true;
}

void ParameterPin::GetDefault(ObjectParameter &param)
{
    param.index=defaultIndex;
    param.value=defaultValue;
    param.visible=defaultVisible;
}

void ParameterPin::GetValues(ObjectParameter &param)
{
    param.index=stepIndex;
    param.value=value;
    param.visible=visible;
    param.limitInMin=limitInMin;
    param.limitInMax=limitInMax;
    param.limitOutMin=limitOutMin;
    param.limitOutMax=limitOutMax;
}

//from float
void ParameterPin::ChangeValue(float val, bool fromObj)
{
    if(listValues) {
        //we got a float, but we need a int
        ChangeValue((int)( 0.5f + val/stepSize ));
        return;
    }

    val = std::min(val,1.0f);
    val = std::max(val,.0f);

    if(!loading && std::abs(val-value)<0.001f)
        return;

    OnValueChanged(val);

    if(!fromObj)
        parent->OnParameterChanged(connectInfo,outValue);
}

//from int
void ParameterPin::ChangeValue(int index, bool fromObj)
{
    index = std::min(index,listValues->size()-1);
    index = std::max(index,0);

    if(!loading && index==stepIndex)
        return;

    stepIndex=index;
    OnValueChanged(stepSize*index);

    outStepIndex=(int)( 0.5f + outValue/stepSize );

    if(!fromObj)
        parent->OnParameterChanged(connectInfo,outValue);
}

//from variant
void ParameterPin::ChangeValue(const QVariant &variant, bool fromObj)
{
    if(listValues)
        ChangeValue(listValues->indexOf(variant),fromObj);
    else
        ChangeValue(variant.toFloat(),fromObj);
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

void ParameterPin::OnStep(int delta)
{
    if(listValues) {
        ChangeValue(stepIndex + delta);
    } else {
        ChangeValue(value + delta*stepSize);
    }

}

//load from program
void ParameterPin::Load(const ObjectParameter &param)
{
    loading = true;

    limitInMin=param.limitInMin;
    limitInMax=param.limitInMax;
    limitOutMin=param.limitOutMin;
    limitOutMax=param.limitOutMax;

    if(listValues)
        ChangeValue(param.index);
    else
        ChangeValue(param.value);

    SetVisible(param.visible);

    dirty = false;
    loading = false;
}

void ParameterPin::OnValueChanged(float val)
{
//    float test = std::abs(value-val);
//    if(std::abs(value-val)>0.001f)
        valueChanged=true;

    value=val;

    //scale value
    if(val>limitInMax)
        val=limitInMax;
    if(val<limitInMin)
        val=limitInMin;
    val-=limitInMin;
    if(limitInMax!=limitInMin)
        val/=(limitInMax-limitInMin);
    val*=(limitOutMax-limitOutMin);
    val+=limitOutMin;
    outValue=val;

    if(!loading && !dirty && connectInfo.direction==Directions::Input) {
        dirty=true;
        parent->OnProgramDirty();
    }

    if(visible) {
        if(nameCanChange)
            setObjectName(parent->GetParameterName(connectInfo));

        if(listValues) {
            displayedText = QString("%1:%2").arg(objectName()).arg(listValues->at(stepIndex).toString());
        }
    }
}

void ParameterPin::SetFixedName(QString fixedName)
{
    setObjectName(fixedName);
    nameCanChange=false;
}

void ParameterPin::SetLimit(Directions::Enum direction, LimitTypes::Enum limit, float newVal)
{
    if(direction==Directions::Input) {
        if(limit==LimitTypes::Min) {
            limitInMin = newVal;
            if(limitInMax<=limitInMin) {
                limitInMax=limitInMin+0.005;
                if(indexLimitInMax.isValid())
                    parent->getHost()->GetModel()->setData( indexLimitInMax, limitInMax, UserRoles::value);
            }
        } else if(limit==LimitTypes::Max) {
            limitInMax = newVal;
            if(limitInMax<=limitInMin) {
                limitInMin=limitInMax-0.005;
                if(indexLimitInMin.isValid())
                    parent->getHost()->GetModel()->setData( indexLimitInMin, limitInMin, UserRoles::value);
            }
        }
    } else if(direction==Directions::Output) {
        if(limit==LimitTypes::Min) {
            limitOutMin = newVal;
        } else if(limit==LimitTypes::Max) {
            limitOutMax = newVal;
        }
    }

    OnValueChanged(value);
    if(!dirty) {
        dirty=true;
        parent->OnProgramDirty();
    }
}

void ParameterPin::SetVisible(bool vis)
{
//    if(visible==vis)
//        return;

    Pin::SetVisible(vis);

    if(modelIndex.isValid() && limitsEnabled) {
        QStandardItem *pinItem = parent->getHost()->GetModel()->itemFromIndex(modelIndex);
        if(!pinItem)
            return;

        ObjectInfo info;
        info.metaType=MetaTypes::cursor;

        info.listInfos[MetaInfos::Direction]=Directions::Input;

        QStandardItem *item = new QStandardItem("limitInMin");
        info.listInfos[MetaInfos::LimitType]=LimitTypes::Min;
        item->setData(QVariant::fromValue(info),UserRoles::objInfo);
        item->setData(limitInMin,UserRoles::value);
        pinItem->appendRow(item);
        indexLimitInMin=item->index();

        item = new QStandardItem("limitInMax");
        info.listInfos[MetaInfos::LimitType]=LimitTypes::Max;
        item->setData(QVariant::fromValue(info),UserRoles::objInfo);
        item->setData(limitInMax,UserRoles::value);
        pinItem->appendRow(item);
        indexLimitInMax=item->index();

        info.listInfos[MetaInfos::Direction]=Directions::Output;

        item = new QStandardItem("limitOutMin");
        info.listInfos[MetaInfos::LimitType]=LimitTypes::Min;
        item->setData(QVariant::fromValue(info),UserRoles::objInfo);
        item->setData(limitOutMin,UserRoles::value);
        pinItem->appendRow(item);
        indexLimitOutMin=item->index();

        item = new QStandardItem("limitOutMax");
        info.listInfos[MetaInfos::LimitType]=LimitTypes::Max;
        item->setData(QVariant::fromValue(info),UserRoles::objInfo);
        item->setData(limitOutMax,UserRoles::value);
        pinItem->appendRow(item);
        indexLimitOutMax=item->index();
    }
}
