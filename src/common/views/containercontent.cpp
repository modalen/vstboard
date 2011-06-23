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

#ifdef _MSC_VER
#pragma warning( disable: 4100 )
#endif

using namespace View;

ContainerContent::ContainerContent(MainHost *myHost, QAbstractItemModel *model, QGraphicsItem * parent, Qt::WindowFlags wFlags ) :
    QGraphicsWidget(parent,wFlags),
    model(model)
{
    config = &myHost->mainWindow->viewConfig;
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setGeometry(0,0,50,50);

    dropPos.setX(0);
    dropPos.setY(0);

    setAutoFillBackground(true);
    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Panel,Colors::Background) );
    setPalette( pal );

    connect( config, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
            this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)) );
}

void ContainerContent::SetModelIndex(QPersistentModelIndex index)
{
    objIndex = index;
}

QPointF ContainerContent::GetDropPos()
{
    QPointF p = dropPos;
    dropPos.rx()+=20;
    dropPos.ry()+=20;
    return p;
}

void ContainerContent::dragEnterEvent( QGraphicsSceneDragDropEvent *event)
{
    //accepts objects from parking
    if(event->source() == myParking ) {
        QGraphicsWidget::dragEnterEvent(event);
        HighlightStart();
        return;
    }

#ifdef VSTSDK
    //accept DLL files
    if (event->mimeData()->hasUrls()) {
        QString fName;
        QFileInfo info;

        foreach(QUrl url,event->mimeData()->urls()) {
            fName = url.toLocalFile();
            info.setFile( fName );
            if ( info.isFile() && info.isReadable() && info.suffix()=="dll" ) {
                event->setDropAction(Qt::CopyAction);
                event->accept();
                HighlightStart();
                return;
            }
        }
    }
#endif

    //accept Audio interface
    //accept Midi interface
    //accept Tools
    if(event->mimeData()->hasFormat("application/x-audiointerface") ||
       event->mimeData()->hasFormat("application/x-midiinterface") ||
       event->mimeData()->hasFormat("application/x-tools")) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
        HighlightStart();
        return;
    }
    event->ignore();
}

void ContainerContent::dragLeaveEvent( QGraphicsSceneDragDropEvent *event)
{
    HighlightStop();
}

void ContainerContent::dropEvent( QGraphicsSceneDragDropEvent *event)
{
    HighlightStop();
    QGraphicsWidget::dropEvent(event);
    dropPos = mapToScene(event->pos());
    event->setAccepted(model->dropMimeData(event->mimeData(), event->proposedAction(), 0, 0, objIndex));
}

void ContainerContent::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId!=ColorGroups::Panel)
        return;

    switch(colorId) {
        case Colors::Background : {
            QPalette pal(palette());
            pal.setColor(QPalette::Window,color);
            setPalette( pal );
            break;
        }
        default:
            break;
    }
}

void ContainerContent::HighlightStart()
{
    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Panel,Colors::HighlightBackground) );
    setPalette( pal );
}

void ContainerContent::HighlightStop()
{
    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Panel,Colors::Background) );
    setPalette( pal );
}
