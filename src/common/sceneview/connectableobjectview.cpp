/**************************************************************************
#    Copyright 2010-2011 Rapha�l Fran�ois
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

#include "connectableobjectview.h"
#include "connectablepinview.h"
#include "../connectables/objectfactory.h"
#include "../connectables/container.h"

using namespace View;

ConnectableObjectView::ConnectableObjectView(MainHost *myHost,QAbstractItemModel *model,MainContainerView * parent, Qt::WindowFlags wFlags ) :
    ObjectView(myHost,model,parent,wFlags),
    dropReplace(0),
    dropAttachLeft(0),
    dropAttachRight(0),
    moving(false)
{
    titleText = new QGraphicsSimpleTextItem(QString("Title"),this);
    titleText->moveBy(2,1);
    titleText->setBrush( config->GetColor(ColorGroups::Object,Colors::Text) );

    layout = new QGraphicsGridLayout() ;
    layout->setSpacing(0);
    layout->setHorizontalSpacing(5);
    layout->setContentsMargins(0,15,0,0);
    setLayout(layout);

//    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFocusPolicy(Qt::StrongFocus);

    listAudioIn = new ListPinsView(this);
    listAudioOut = new ListPinsView(this);
    listMidiIn = new ListPinsView(this);
    listMidiOut = new ListPinsView(this);
    listParametersIn = new ListPinsView(this);
    listParametersOut = new ListPinsView(this);

    layout->addItem(listAudioIn,0,0,Qt::AlignLeft | Qt::AlignTop);
    layout->addItem(listAudioOut,0,1,Qt::AlignRight | Qt::AlignTop);
    layout->addItem(listMidiIn,1,0,Qt::AlignLeft | Qt::AlignTop);
    layout->addItem(listMidiOut,1,1,Qt::AlignRight | Qt::AlignTop);
    layout->addItem(listParametersIn,2,0,Qt::AlignLeft | Qt::AlignTop);
    layout->addItem(listParametersOut,2,1,Qt::AlignRight | Qt::AlignTop);

    dropReplace = new ObjectDropZone(this,parent->GetParking());
    connect(parent,SIGNAL(ParkingChanged(QWidget*)),
            dropReplace,SLOT(SetParking(QWidget*)));
    connect(dropReplace, SIGNAL(ObjectDropped(QGraphicsSceneDragDropEvent*)),
            this,SLOT(ObjectDropped(QGraphicsSceneDragDropEvent*)));
    connect(this, SIGNAL(heightChanged()),
            dropReplace, SLOT(UpdateHeight()));

    dropAttachLeft = new ObjectDropZone(this,parent->GetParking());
    connect(parent,SIGNAL(ParkingChanged(QWidget*)),
            dropAttachLeft,SLOT(SetParking(QWidget*)));
    connect(dropAttachLeft, SIGNAL(ObjectDropped(QGraphicsSceneDragDropEvent*)),
            this,SLOT(ObjectDropped(QGraphicsSceneDragDropEvent*)));
    connect(this, SIGNAL(heightChanged()),
            dropAttachLeft, SLOT(UpdateHeight()));

    dropAttachRight = new ObjectDropZone(this,parent->GetParking());
    connect(parent,SIGNAL(ParkingChanged(QWidget*)),
            dropAttachLeft,SLOT(SetParking(QWidget*)));
    connect(dropAttachRight, SIGNAL(ObjectDropped(QGraphicsSceneDragDropEvent*)),
            this,SLOT(ObjectDropped(QGraphicsSceneDragDropEvent*)));
    connect(this, SIGNAL(heightChanged()),
            dropAttachRight, SLOT(UpdateHeight()));

    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Object,Colors::HighlightBackground) );
    dropReplace->setPalette( pal );
    dropAttachLeft->setPalette( pal );
    dropAttachRight->setPalette( pal );

    setGeometry(QRectF(0,0,105,15));
}

void ConnectableObjectView::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    ObjectView::resizeEvent(event);

    if(dropReplace)
        dropReplace->setGeometry( 20,0,size().width()-40, event->newSize().height());
    if(dropAttachLeft)
        dropAttachLeft->setGeometry(-10,0,30, event->newSize().height());
    if(dropAttachRight)
        dropAttachRight->setGeometry(event->newSize().width()-20,0,30, event->newSize().height());
}

void ConnectableObjectView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
    QGraphicsWidget::mousePressEvent(event);
    if(!event->isAccepted()) {

        const KeyBind::MoveBind b = config->keyBinding->GetMoveSortcuts(KeyBind::moveObject);
        if(b.input == KeyBind::mouse && b.modifier == event->modifiers() && b.buttons == event->buttons()) {
            event->accept();
            moveOffset=pos()-mapToParent(mapFromScene(event->scenePos()));
            startDragMousePos = event->screenPos();
            moving=true;
            return;
        }
    }

    if(moving) {
        moving=false;
        model->setData(objIndex,pos(),UserRoles::position);
        return;
    }
}

void ConnectableObjectView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
    QGraphicsWidget::mouseMoveEvent(event);
    if(event->isAccepted())
        return;

    if(moving && QLineF(event->screenPos(), startDragMousePos).length() > QApplication::startDragDistance()) {
        event->accept();
        setPos(mapToParent(mapFromScene(event->scenePos()))+moveOffset);
    }
}

/*!
  Reimplements QGraphicsWidget::mouseReleaseEvent \n
  update the model with the new position
  */
void ConnectableObjectView::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    if(moving) {
        moving=false;
        model->setData(objIndex,pos(),UserRoles::position);
        event->accept();
        return;
    }
    QGraphicsWidget::mouseReleaseEvent(event);

}

void ConnectableObjectView::ObjectDropped(QGraphicsSceneDragDropEvent *event)
{
    QPointF dropPos(0,0);
    int col=0;

    if(sender()==dropAttachLeft) {
        col=1;
        dropPos.rx()-=(geometry().width()+10);
    }
    if(sender()==dropReplace){
        col=2;
    }
    if(sender()==dropAttachRight){
        col=3;
        dropPos.rx()+=(geometry().width()+10);
    }

    MainContainerView *cnt = static_cast<MainContainerView*>(parentItem());
    if(cnt)
        cnt->SetDropPos( mapToScene(dropPos) );
    event->setAccepted(model->dropMimeData(event->mimeData(), event->dropAction(), 0, col, objIndex));
}

void ConnectableObjectView::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId==ColorGroups::Object && colorId==Colors::HighlightBackground) {
        QPalette pal(palette());
        pal.setColor(QPalette::Window, color );
        dropReplace->setPalette( pal );
        dropAttachLeft->setPalette( pal );
        dropAttachRight->setPalette( pal );
        return;
    }
    ObjectView::UpdateColor(groupId,colorId,color);
}
