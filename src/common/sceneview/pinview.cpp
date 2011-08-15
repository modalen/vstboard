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

#include "pinview.h"
#include "cableview.h"
#include "connectables/pins/pin.h"
#include "connectables/objectfactory.h"

using namespace View;

CableView *PinView::currentLine = 0;

/*!
  \class View::PinView
  \brief base class for the pins views
  */

/*!
  \param angle angle in rad (0=output, pi=input) used by CableView
  \param model pointer to the model
  \param parent pointer to the parent object view
  \param pinInfo description of the pin
  \todo the model parameter can be removed
  */
PinView::PinView(const MetaInfo &info, float angle, QGraphicsItem * parent, ViewConfig *config) :
    QGraphicsWidget(parent),
    ObjectInfo(info),
    outline(0),
    highlight(0),
    pinAngle(angle),
    config(config)
{
    setAcceptDrops(true);
    setCursor(Qt::PointingHandCursor);
    connect( config, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)) ,
            this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)) );

    actDel = new QAction(QIcon(":/img16x16/delete.png"),tr("Remove"),this);
    actDel->setShortcut( Qt::Key_Delete );
    actDel->setShortcutContext(Qt::WidgetShortcut);
    connect(actDel,SIGNAL(triggered()),
            this,SLOT(RemovePin()));

    if(Meta(MetaInfos::Removable).toBool())
        addAction(actDel);

    actUnplug = new QAction(QIcon(":/img16x16/editcut.png"),tr("Unplug"),this);
    actUnplug->setShortcut( Qt::Key_Backspace );
    actUnplug->setShortcutContext(Qt::WidgetShortcut);
    actUnplug->setEnabled(false);
    connect(actUnplug,SIGNAL(triggered()),
            this,SLOT(Unplug()));
    addAction(actUnplug);

    setFocusPolicy(Qt::WheelFocus);
}

/*!
  Reimplements QGraphicsWidget::contextMenuEvent \n
  create a menu with all the actions
  */
void PinView::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(actions().size()==0)
        return;

    QMenu menu;
    menu.exec(actions(),event->screenPos(),actions().at(0),event->widget());
}

/*!
  Update the position of all connected cables, called by PinView::itemChange and ListPinsView::UpdateCablesPosition
  */
void PinView::UpdateCablesPosition()
{
    foreach (CableView *cable, connectedCables) {
        cable->UpdatePosition(info(), pinAngle, mapToScene(pinPos()) );
    }
}

/*!
  Reimplements QGraphicsWidget::itemChange \n
  calls PinView::UpdateCablesPosition when needed
  */
QVariant PinView::itemChange ( GraphicsItemChange change, const QVariant & value )
{
    if(!scene())
        return QGraphicsWidget::itemChange(change, value);

    if(change == QGraphicsItem::ItemScenePositionHasChanged)
        UpdateCablesPosition();

    return QGraphicsWidget::itemChange(change, value);
}

/*!
  Reimplements QGraphicsWidget::mousePressEvent \n
  set the closed hand cursor
  */
void PinView::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    setCursor(Qt::ClosedHandCursor);
}

/*!
  Reimplements QGraphicsWidget::mouseMoveEvent \n
  Starts a drag&drop when the conditions are met
  */
void PinView::mouseMoveEvent ( QGraphicsSceneMouseEvent  * event )
{

    if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length() < QApplication::startDragDistance()) {
        return;
    }

    QDrag *drag = new QDrag(event->widget());

    QMimeData *mime = new QMimeData;

    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    info().toStream(stream);
    mime->setData("application/x-pin",bytes);
    drag->setMimeData(mime);

    if(!currentLine) {
        currentLine = new CableView(info(),event->pos(),this,config);
        AddCable(currentLine);
    }

    drag->exec(Qt::CopyAction);

    if(currentLine) {
        RemoveCable(currentLine);
        delete currentLine;
        currentLine = 0;
    }
    setCursor(Qt::OpenHandCursor);
}

/*!
  Reimplements QGraphicsWidget::mouseReleaseEvent \n
  back to the open hand cursor
  */
void PinView::mouseReleaseEvent ( QGraphicsSceneMouseEvent  * /*event*/ )
{
    setCursor(Qt::OpenHandCursor);
}

/*!
  Reimplements QGraphicsWidget::mouseDoubleClickEvent \n
  remove all connected cables
  */
void PinView::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * /*event*/ )
{
    Unplug();
}

void PinView::Unplug()
{
    emit RemoveCablesFromPin(info());
}

void PinView::RemovePin()
{
    if(Meta(MetaInfos::Removable).toBool())
        emit RemovePin(info());
}

/*!
  Reimplements QGraphicsWidget::dragMoveEvent \n
  if the dragged object is connectable with us : create a temporary cable and highlight the pin
  */
void PinView::dragEnterEvent ( QGraphicsSceneDragDropEvent * event )
{
    if(event->mimeData()->hasFormat("application/x-pin")) {
        QByteArray bytes = event->mimeData()->data("application/x-pin");
        QDataStream stream(&bytes, QIODevice::ReadOnly);
        MetaInfo otherInfo;
        otherInfo.fromStream(stream);

        if(!CanConnectTo(otherInfo)) {
            event->ignore();
            return;
        }

        event->acceptProposedAction();

        if(currentLine) {
            currentLine->UpdatePosition(info(),pinAngle,mapToScene(pinPos()));
            currentLine->setVisible(true);
        }
        if(highlight)
            highlight->setVisible(true);
    } else {
        event->ignore();
    }
}

void PinView::dragMoveEvent ( QGraphicsSceneDragDropEvent * /*event*/ )
{
    if(currentLine)
        currentLine->setVisible(true);
}

/*!
  Reimplements QGraphicsWidget::dragLeaveEvent \n
  hide the temporary cable, remove highlight on the pin
  */
void PinView::dragLeaveEvent( QGraphicsSceneDragDropEvent  * /*event*/ )
{
    if(highlight)
        highlight->setVisible(false);
    if(currentLine)
        currentLine->setVisible(false);
}

/*!
  Reimplements QGraphicsWidget::dropEvent \n
  a connectable pin was dropped on us, remove the highlight, emit a ConnectPins
  */
void PinView::dropEvent ( QGraphicsSceneDragDropEvent  * event )
{
    if(highlight)
        highlight->setVisible(false);
    QByteArray bytes = event->mimeData()->data("application/x-pin");
    QDataStream stream(&bytes, QIODevice::ReadOnly);
    MetaInfo otherInfo;
    otherInfo.fromStream(stream);
    emit ConnectPins(info(), otherInfo);
}

/*!
  Get the pin connection point
  \return the QPointF
  */
const QPointF PinView::pinPos() const
{
    qreal x = 0;
    if(Meta(MetaInfos::Direction).toInt()==Directions::Output)
        x = geometry().width();
    return QPointF(x,geometry().height()/2);
}

/*!
  Add a cable to the list of connected cables and update its position
  \param cable pointer to the cable
  */
void PinView::AddCable(CableView *cable)
{
    cable->UpdatePosition(info(), pinAngle, mapToScene(pinPos()) );
    connectedCables.append(cable);
    actUnplug->setEnabled(true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
}

/*!
  Remove a cable from the list of connected cables
  */
void PinView::RemoveCable(CableView *cable)
{
    connectedCables.removeAll(cable);
    if(connectedCables.isEmpty()) {
        setFlag(QGraphicsItem::ItemSendsScenePositionChanges, false);
        actUnplug->setEnabled(false);
    }
}