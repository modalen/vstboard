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
#include "../connectables/objectfactory.h"

using namespace View;

MainContainerView::MainContainerView(MainHost *myHost, MsgController *msgCtrl, int objId) :
        ObjectView(myHost,msgCtrl,objId)
{
    content = new ContainerContent(myHost,msgCtrl,-1,this);
    content->setAcceptDrops(true);
    connect(content, SIGNAL(ObjectDropped(QGraphicsSceneDragDropEvent*,MsgObject)),
            this,SLOT(ObjectDropped(QGraphicsSceneDragDropEvent*,MsgObject)));

    bridgeIn = new BridgeView(myHost, msgCtrl, -1, this);
    bridgeOut = new BridgeView(myHost, msgCtrl, -1, this);
    bridgeSend = new BridgeView(myHost, msgCtrl, -1, this);
    bridgeReturn = new BridgeView(myHost, msgCtrl, -1, this);

    setGeometry(0,0,0,0);
}

MainContainerView::~MainContainerView()
{
    foreach(CableView *c, listCables) {
        c->scene()->removeItem(c);
        c->deleteLater();
    }
}

void MainContainerView::showEvent(QShowEvent *event)
{
    LOG("container show event"<<GetIndex())
}

QPointF MainContainerView::GetDropPos()
{
    return mapFromScene( content->GetDropPos() );
}

void MainContainerView::SetDropPos(const QPointF &pt)
{
    content->SetDropPos( pt );
}

void MainContainerView::ReceiveMsg(const MsgObject &msg)
{
    content->ReceiveMsg(msg);
}

//void MainContainerView::SetModelIndex(QPersistentModelIndex index)
//{
//    setObjectName(QString("MainContainerView%1").arg(index.data(UserRoles::value).toInt()));
//    content->SetModelIndex(index);
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

void MainContainerView::ObjectDropped(QGraphicsSceneDragDropEvent *event, MsgObject msg)
{
    SetDropPos( event->scenePos() );
    msg.objIndex=GetIndex();
    msg.prop["actionType"]=InsertionType::NoInsertion;
    msgCtrl->SendMsg(msg);
}
