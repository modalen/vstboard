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

#ifndef CONNECTABLEOBJECTVIEW_H
#define CONNECTABLEOBJECTVIEW_H

#include "objectview.h"
#include "cableview.h"
#include "objectdropzone.h"
#include "maincontainerview.h"

class SceneModel;
namespace View {

    class ConnectableObjectView : public ObjectView
    {
    Q_OBJECT
    public:
        ConnectableObjectView(const MetaInfo &info,MainContainerView * parent, SceneModel *model );
        virtual void SetConfig(ViewConfig *config);

    protected:
        void resizeEvent ( QGraphicsSceneResizeEvent * event );
        QList<CableView*>listCables;
        ObjectDropZone *dropReplace;
        ObjectDropZone *dropAttachLeft;
        ObjectDropZone *dropAttachRight;

    private:
        SceneModel *model;

    public slots:
        void ObjectDropped(QGraphicsSceneDragDropEvent *event);
        virtual void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
    };

}

#endif // CONNECTABLEOBJECTVIEW_H
