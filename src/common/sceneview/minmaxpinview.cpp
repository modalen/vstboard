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

MinMaxPinView::MinMaxPinView(const MetaData &info, float angle, QGraphicsItem * parent, ViewConfig *config) :
    ConnectablePinView(info,angle,parent,config),
    cursorCreated(false),
    inMin(0),
    inMax(0),
    outMin(0),
    outMax(0),
    scaledView(0)
{
    if(MetaData::GetMetaData<bool>(metaT::LimitEnabled))
        CreateCursors();

    UpdateModelIndex(info);
}

void MinMaxPinView::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    ConnectablePinView::resizeEvent(event);

    if(cursorCreated) {
        inMin->setPos( MetaData::GetMetaData<float>(metaT::LimitInMin)*event->newSize().width(), 0 );
        inMax->setPos( MetaData::GetMetaData<float>(metaT::LimitInMax)*event->newSize().width(), 0 );
        outMin->setPos( MetaData::GetMetaData<float>(metaT::LimitOutMin)*event->newSize().width(), 0 );
        outMax->setPos( MetaData::GetMetaData<float>(metaT::LimitOutMax)*event->newSize().width(), 0 );
        UpdateScaleView();
    }
}

void MinMaxPinView::CreateCursors()
{
    cursorCreated=true;

    scaledView = new QGraphicsPolygonItem(this);
    scaledView->setBrush(QColor(0,0,0,30));

    inMin=new CursorView(metaT::LimitInMin,this,config);
    inMin->setPos(rect().topLeft());
    inMin->SetValue( MetaData::GetMetaData<float>(metaT::LimitInMin) );
    connect(inMin, SIGNAL(valChanged(metaT::Enum,float)),
            this, SLOT(LimitChanged(metaT::Enum,float)));

    inMax=new CursorView(metaT::LimitInMax,this,config);
    inMax->setPos(rect().topRight());
    inMax->SetValue( MetaData::GetMetaData<float>(metaT::LimitInMax) );
    connect(inMax, SIGNAL(valChanged(metaT::Enum,float)),
            this, SLOT(LimitChanged(metaT::Enum,float)));

    outMin=new CursorView(metaT::LimitOutMin,this,config);
    outMin->setPos(rect().bottomLeft());
    outMin->SetValue( MetaData::GetMetaData<float>(metaT::LimitOutMin) );
    connect(outMin, SIGNAL(valChanged(metaT::Enum,float)),
            this, SLOT(LimitChanged(metaT::Enum,float)));

    outMax=new CursorView(metaT::LimitOutMax,this,config);
    outMax->setPos(rect().bottomRight());
    outMax->SetValue( MetaData::GetMetaData<float>(metaT::LimitOutMax) );
    connect(outMax, SIGNAL(valChanged(metaT::Enum,float)),
            this, SLOT(LimitChanged(metaT::Enum,float)));
}

void MinMaxPinView::LimitChanged(metaT::Enum type,float val)
{
    MetaData::SetMeta(type,val);
}

void MinMaxPinView::UpdateModelIndex(const MetaData &info)
{
    ConnectablePinView::UpdateModelIndex(info);

    if(cursorCreated) {
        inMin->SetValue( MetaData::GetMetaData<float>(metaT::LimitInMin) );
        inMax->SetValue( MetaData::GetMetaData<float>(metaT::LimitInMax) );
        outMin->SetValue( MetaData::GetMetaData<float>(metaT::LimitOutMin) );
        outMax->SetValue( MetaData::GetMetaData<float>(metaT::LimitOutMax) );
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
