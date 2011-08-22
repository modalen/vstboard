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
#include "objectview.h"

using namespace View;

MinMaxPinView::MinMaxPinView(const MetaInfo &info, float angle, QGraphicsItem * parent, ViewConfig *config) :
    ConnectablePinView(info,angle,parent,config),
    cursorCreated(false),
    inMin(0),
    inMax(0),
    outMin(0),
    outMax(0),
    scaledView(0),
    changingValue(false)
{
    if(Meta(MetaInfos::LimitEnabled).isValid())
        CreateCursors();

    UpdateModelIndex(info);
}

void MinMaxPinView::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    ConnectablePinView::resizeEvent(event);

    if(cursorCreated) {
        inMin->setPos( Meta(MetaInfos::LimitInMin).toFloat()*event->newSize().width(), 0 );
        inMax->setPos( Meta(MetaInfos::LimitInMax).toFloat()*event->newSize().width(), 0 );
        outMin->setPos( Meta(MetaInfos::LimitOutMin).toFloat()*event->newSize().width(), 0 );
        outMax->setPos( Meta(MetaInfos::LimitOutMax).toFloat()*event->newSize().width(), 0 );
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
    inMin->SetValue( Meta(MetaInfos::LimitInMin).toFloat() );
    connect(inMin, SIGNAL(valChanged(MetaInfos::Enum,float)),
            this, SLOT(LimitChanged(MetaInfos::Enum,float)));

    inMax=new CursorView(MetaInfos::LimitInMax,this,config);
    inMax->setPos(rect().topRight());
    inMax->SetValue( Meta(MetaInfos::LimitInMax).toFloat() );
    connect(inMax, SIGNAL(valChanged(MetaInfos::Enum,float)),
            this, SLOT(LimitChanged(MetaInfos::Enum,float)));

    outMin=new CursorView(MetaInfos::LimitOutMin,this,config);
    outMin->setPos(rect().bottomLeft());
    outMin->SetValue( Meta(MetaInfos::LimitOutMin).toFloat() );
    connect(outMin, SIGNAL(valChanged(MetaInfos::Enum,float)),
            this, SLOT(LimitChanged(MetaInfos::Enum,float)));

    outMax=new CursorView(MetaInfos::LimitOutMax,this,config);
    outMax->setPos(rect().bottomRight());
    outMax->SetValue( Meta(MetaInfos::LimitOutMax).toFloat() );
    connect(outMax, SIGNAL(valChanged(MetaInfos::Enum,float)),
            this, SLOT(LimitChanged(MetaInfos::Enum,float)));
}

void MinMaxPinView::LimitChanged(MetaInfos::Enum type,float val)
{
    SetMeta(type,val);
}

void MinMaxPinView::UpdateModelIndex(const MetaInfo &info)
{
    ConnectablePinView::UpdateModelIndex(info);

    //avoid "jumps"
    if(!changingValue)
        value = info.Meta(MetaInfos::Value).toFloat();

    float newVu = geometry().width() * value;
    rectVu->setRect(0,0, newVu, geometry().height());

    if(cursorCreated) {
        inMin->SetValue( Meta(MetaInfos::LimitInMin).toFloat() );
        inMax->SetValue( Meta(MetaInfos::LimitInMax).toFloat() );
        outMin->SetValue( Meta(MetaInfos::LimitOutMin).toFloat() );
        outMax->SetValue( Meta(MetaInfos::LimitOutMax).toFloat() );
        UpdateScaleView();
    }

    ObjectView *parentObj = static_cast<ObjectView*>(parentWidget()->parentWidget());
    if(parentObj) {
        if(info.Meta(MetaInfos::PinNumber).toInt() == FixedPinNumber::editorVisible) {
            parentObj->SetEditorPin(this, value);
        }
        if(info.Meta(MetaInfos::PinNumber).toInt() == FixedPinNumber::learningMode) {
            parentObj->SetLearnPin(this, value);
        }
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

void MinMaxPinView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(config->EditMode()!=EditMode::Value) {
        ConnectablePinView::mousePressEvent(event);
        return;
    }

    changingValue=true;
    startDragPos=event->screenPos();
    grabMouse();
}

void MinMaxPinView::mouseMoveEvent ( QGraphicsSceneMouseEvent  * event )
{
    if(changingValue) {
        float mouseSensibility = 1.0f/size().width();
        if(event->modifiers() & Qt::ControlModifier)
            mouseSensibility /= 10;
        if(event->modifiers() & Qt::ShiftModifier)
            mouseSensibility /= 10;
        if(event->modifiers() & Qt::AltModifier)
            mouseSensibility /= 10;

        int increm = event->screenPos().x() - startDragPos.x();
        startDragPos=event->screenPos();

        if(increm==0)
            return;

        value += mouseSensibility*increm;
        value = std::max(.0f,value);
        value = std::min(1.0f,value);

        float newVu = geometry().width() * value;
        rectVu->setRect(0,0, newVu, geometry().height());
        SetMeta(MetaInfos::Value,value);
    } else {
        ConnectablePinView::mouseMoveEvent(event);
    }
}

void MinMaxPinView::mouseReleaseEvent ( QGraphicsSceneMouseEvent  * event )
{
    if(changingValue) {
        ungrabMouse();
        changingValue=false;
    }
    ConnectablePinView::mouseReleaseEvent(event);
}
