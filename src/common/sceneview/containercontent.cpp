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

#include "containercontent.h"
#include "objectview.h"
#include "maincontainerview.h"
#include "pinview.h"
#include "cableview.h"
#include "models/scenemodel.h"

#ifdef _MSC_VER
#pragma warning( disable: 4100 )
#endif

using namespace View;

ContainerContent::ContainerContent(const MetaData &info, MainContainerView * parent, SceneModel *model ) :
    ObjectDropZone(parent),
    MetaData(info),
    model(model),
    rectAttachLeft(0),
    rectAttachRight(0),
    config(0)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setGeometry(0,0,50,50);

//    dropPos.setX(0);
//    dropPos.setY(0);

}

void ContainerContent::SetConfig(ViewConfig *conf)
{
    config=conf;
    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Object,Colors::HighlightBackground) );
    setPalette( pal );

    connect( config, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
            this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)) );
}

//void ContainerContent::SetModelIndex(const MetaData &info)
//{
//    objInfo = info;
//}

//QPointF ContainerContent::GetDropPos()
//{
//    QPointF p = dropPos;
//    dropPos.rx()+=20;
//    dropPos.ry()+=20;
//    return p;
//}

//void ContainerContent::SetDropPos(const QPointF &pt)
//{
//    dropPos=pt;
//}

void ContainerContent::dragEnterEvent( QGraphicsSceneDragDropEvent *event)
{
    //update connecting cable position
    if(PinView::currentLine && PinView::currentLine->scene()==scene()) {
        PinView::currentLine->setVisible(true);
        event->accept();
        QGraphicsWidget::dragEnterEvent(event);
        return;
    }

    ObjectDropZone::dragEnterEvent(event);
}

void ContainerContent::dragMoveEvent( QGraphicsSceneDragDropEvent *event)
{
    //update connecting cable position
    if(PinView::currentLine && PinView::currentLine->scene()==scene()) {
        PinView::currentLine->UpdatePosition( mapToScene(event->pos()) );
        PinView::currentLine->setVisible(true);
        event->ignore();
        QGraphicsWidget::dragMoveEvent(event);
        return;
    }
    ObjectDropZone::dragMoveEvent(event);
}

void ContainerContent::dragLeaveEvent( QGraphicsSceneDragDropEvent *event)
{
    if(PinView::currentLine)
        PinView::currentLine->setVisible(false);

    ObjectDropZone::dragLeaveEvent(event);
}

void ContainerContent::dropEvent( QGraphicsSceneDragDropEvent *event)
{
    ObjectDropZone::dropEvent(event);
    model->dropMime(event->mimeData(),MetaData(*this),event->scenePos());
}

void ContainerContent::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId==ColorGroups::Panel && colorId==Colors::Background) {
        HighlightStop();
        return;
    }
}

void ContainerContent::HighlightStart()
{
    setAutoFillBackground( true );
    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Panel,Colors::HighlightBackground) );
    setPalette( pal );
}

void ContainerContent::HighlightStop()
{
    QColor color = config->GetColor(ColorGroups::Panel,Colors::Background);
    if(color.alpha()==0) {
        setAutoFillBackground( false );
    } else {
        setAutoFillBackground( true );
        QPalette pal(palette());
        pal.setColor(QPalette::Window, color );
        setPalette( pal );
    }
}
