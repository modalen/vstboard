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
#include "minmaxpinview.h"
#include "objectinfo.h"

using namespace View;

MinMaxPinView::MinMaxPinView(const MetaInfo &info, float angle, QGraphicsItem * parent, ViewConfig *config) :
    ConnectablePinView(info,angle,parent,config),
    cursorCreated(false),
    inMin(0),
    inMax(0),
    outMin(0),
    outMax(0),
    scaledView(0)
{
    if(MetaInfo::data.GetMetaData<bool>(MetaInfos::LimitEnabled))
        CreateCursors();

    UpdateModelIndex(info);
}

void MinMaxPinView::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    ConnectablePinView::resizeEvent(event);

    if(cursorCreated) {
        inMin->setPos( MetaInfo::data.GetMetaData<float>(MetaInfos::LimitInMin)*event->newSize().width(), 0 );
        inMax->setPos( MetaInfo::data.GetMetaData<float>(MetaInfos::LimitInMax)*event->newSize().width(), 0 );
        outMin->setPos( MetaInfo::data.GetMetaData<float>(MetaInfos::LimitOutMin)*event->newSize().width(), 0 );
        outMax->setPos( MetaInfo::data.GetMetaData<float>(MetaInfos::LimitOutMax)*event->newSize().width(), 0 );
        UpdateScaleView();
    }
}

void MinMaxPinView::CreateCursors()
{
    cursorCreated=true;

    scaledView = new QGraphicsPolygonItem(this);
    scaledView->setBrush(QColor(0,0,0,30));

    inMin=new CursorView(MetaInfos::LimitInMin,this,config);
    inMin->setPos(rect().topLeft());
    inMin->SetValue( MetaInfo::data.GetMetaData<float>(MetaInfos::LimitInMin) );
    connect(inMin, SIGNAL(valChanged(MetaInfos::Enum,float)),
            this, SLOT(LimitChanged(MetaInfos::Enum,float)));

    inMax=new CursorView(MetaInfos::LimitInMax,this,config);
    inMax->setPos(rect().topRight());
    inMax->SetValue( MetaInfo::data.GetMetaData<float>(MetaInfos::LimitInMax) );
    connect(inMax, SIGNAL(valChanged(MetaInfos::Enum,float)),
            this, SLOT(LimitChanged(MetaInfos::Enum,float)));

    outMin=new CursorView(MetaInfos::LimitOutMin,this,config);
    outMin->setPos(rect().bottomLeft());
    outMin->SetValue( MetaInfo::data.GetMetaData<float>(MetaInfos::LimitOutMin) );
    connect(outMin, SIGNAL(valChanged(MetaInfos::Enum,float)),
            this, SLOT(LimitChanged(MetaInfos::Enum,float)));

    outMax=new CursorView(MetaInfos::LimitOutMax,this,config);
    outMax->setPos(rect().bottomRight());
    outMax->SetValue( MetaInfo::data.GetMetaData<float>(MetaInfos::LimitOutMax) );
    connect(outMax, SIGNAL(valChanged(MetaInfos::Enum,float)),
            this, SLOT(LimitChanged(MetaInfos::Enum,float)));
}

void MinMaxPinView::LimitChanged(MetaInfos::Enum type,float val)
{
    MetaInfo::data.SetMeta(type,val);
}

void MinMaxPinView::UpdateModelIndex(const MetaInfo &info)
{
    ConnectablePinView::UpdateModelIndex(info);

    if(cursorCreated) {
        inMin->SetValue( MetaInfo::data.GetMetaData<float>(MetaInfos::LimitInMin) );
        inMax->SetValue( MetaInfo::data.GetMetaData<float>(MetaInfos::LimitInMax) );
        outMin->SetValue( MetaInfo::data.GetMetaData<float>(MetaInfos::LimitOutMin) );
        outMax->SetValue( MetaInfo::data.GetMetaData<float>(MetaInfos::LimitOutMax) );
        UpdateScaleView();
    }
}

void MinMaxPinView::UpdateScaleView()
{
    float limitVal=value;
    if(limitVal<inMin->GetValue())
        limitVal=inMin->GetValue();
    if(limitVal>inMax->GetValue())
        limitVal=inMax->GetValue();

    float outVal=limitVal;
    outVal-=inMin->GetValue();

    if(inMax->GetValue()!=inMin->GetValue())
        outVal/=(inMax->GetValue()-inMin->GetValue());
    outVal*=(outMax->GetValue()-outMin->GetValue());
    outVal+=outMin->GetValue();

    limitVal*=rect().width();
    outVal*=rect().width();

    QPolygonF pol;
    pol << QPointF(limitVal,0) << QPointF(inMin->GetValue(),0)  << QPointF(outMin->GetValue(),rect().height()) << QPointF(outVal,rect().height());
    scaledView->setPolygon(pol);
}
