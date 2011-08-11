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
#include "precomp.h"
#include "minmaxpinview.h"
#include "objectinfo.h"

using namespace View;

MinMaxPinView::MinMaxPinView(float angle, QAbstractItemModel *model,QGraphicsItem * parent, const ObjectInfo &pinInfo, ViewConfig *config) :
        ConnectablePinView(angle,model,parent,pinInfo,config),
        cursorCreated(false)
{

}

void MinMaxPinView::CreateCursors()
{
    cursorCreated=true;

    scaledView = new QGraphicsPolygonItem(this);
    scaledView->setBrush(QColor(0,0,0,30));

    inMin=new CursorView(model,false,false,this,config);
    inMin->setPos(rect().topLeft());
    inMin->SetValue(.0f);

    inMax=new CursorView(model,true,false,this,config);
    inMax->setPos(rect().topRight());
    inMax->SetValue(1.0f);

    outMin=new CursorView(model,false,true,this,config);
    outMin->setPos(rect().bottomLeft());
    outMin->SetValue(.0f);

    outMax=new CursorView(model,true,true,this,config);
    outMax->setPos(rect().bottomRight());
    outMax->SetValue(1.0f);
}

void MinMaxPinView::SetLimitModelIndex(QPersistentModelIndex index)
{
    if(!cursorCreated)
        CreateCursors();

    ObjectInfo info = index.data(UserRoles::objInfo).value<ObjectInfo>();
    int direction = info.Meta(MetaInfos::Direction).toInt();
    int limit = info.Meta(MetaInfos::LimitType).toInt();
    if(direction == Directions::Input) {
        if(limit == LimitTypes::Min) {
            inMin->SetModelIndex(index);
        } else if(limit == LimitTypes::Max) {
            inMax->SetModelIndex(index);
        }
    } else if(direction == Directions::Output) {
        if(limit == LimitTypes::Min) {
            outMin->SetModelIndex(index);
        } else if(limit == LimitTypes::Max) {
            outMax->SetModelIndex(index);
        }
    }
}

void MinMaxPinView::UpdateLimitModelIndex(const QModelIndex &index)
{
    inMin->SetValue( index.child(0,0).data(UserRoles::value).toFloat() );
    inMax->SetValue( index.child(1,0).data(UserRoles::value).toFloat() );
    outMin->SetValue( index.child(2,0).data(UserRoles::value).toFloat() );
    outMax->SetValue( index.child(3,0).data(UserRoles::value).toFloat() );
    UpdateScaleView();
}

void MinMaxPinView::UpdateModelIndex(const QModelIndex &index)
{
    ConnectablePinView::UpdateModelIndex(index);
    if(cursorCreated)
        UpdateScaleView();
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
