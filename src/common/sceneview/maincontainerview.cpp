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

#include "maincontainerview.h"
#include "connectables/objectfactory.h"

using namespace View;

MainContainerView::MainContainerView(MainHost *myHost, const MetaInfo &info) :
        ObjectView(info)
{
    content = new ContainerContent(MetaInfo::info(),this);
    content->setAcceptDrops(true);

    bridgeIn = new BridgeView(MetaInfo(), this);
    bridgeOut = new BridgeView(MetaInfo(), this);
    bridgeSend = new BridgeView(MetaInfo(), this);
    bridgeReturn = new BridgeView(MetaInfo(), this);

    setGeometry(0,0,0,0);

    setObjectName(QString("MainContainerView%1").arg(ObjId()));
}

MainContainerView::~MainContainerView()
{
    foreach(CableView *c, listCables) {
        c->scene()->removeItem(c);
        delete c;
    }
}

void MainContainerView::SetConfig(ViewConfig *config)
{
    ObjectView::SetConfig(config);
    content->SetConfig(config);
}

QPointF MainContainerView::GetDropPos()
{
    return mapFromScene( content->GetDropPos() );
}

void MainContainerView::SetDropPos(const QPointF &pt)
{
    content->SetDropPos( pt );
}

//void MainContainerView::SetModelIndex(const MetaInfo &info)
//{
//    objInfo=info;
//    setObjectName(QString("MainContainerView%1").arg(objInfo.ObjId()));
//    content->SetModelIndex(objInfo);
//}

void MainContainerView::SetParking(QWidget *parking)
{
    content->SetParking( parking );
    emit ParkingChanged(parking);
}

QWidget *MainContainerView::GetParking()
{
    return content->GetParking();
}

void MainContainerView::OnViewChanged(QRectF rect)
{
    bridgeIn->setPos(rect.topLeft());
    bridgeOut->setPos(rect.right() - bridgeOut->geometry().width(), rect.top());
    bridgeSend->setPos(rect.left(), rect.bottom() - bridgeSend->geometry().height() );
    bridgeReturn->setPos(rect.right() - bridgeReturn->geometry().width(), rect.bottom() - bridgeReturn->geometry().height());
    content->setGeometry(rect);
}

