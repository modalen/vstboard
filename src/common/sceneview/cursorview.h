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

#ifndef CURSORVIEW_H
#define CURSORVIEW_H

//#include "precomp.h"
#include "viewconfig.h"
#include "objectinfo.h"

namespace View {

    class CursorView : public QGraphicsWidget
    {
        Q_OBJECT
    public:
        explicit CursorView(MetaInfos::Enum type,QGraphicsItem *parent, ViewConfig *config);
        void SetValue(float newVal);
//        void SetModelIndex(const MetaInfo &info);
        inline float GetValue() const {return value;}
        void setPos ( const QPointF & pos );
        void setPos ( qreal x, qreal y );

    protected:
        void keyPressEvent ( QKeyEvent * event );
        void ValueChanged(float newVal);
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);
        void mousePressEvent ( QGraphicsSceneMouseEvent * event );
        void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
        void focusInEvent ( QFocusEvent * event );
        void focusOutEvent ( QFocusEvent * event );

        QGraphicsPolygonItem *cursor;
        MetaInfos::Enum type;
//        bool isMaxi;
//        bool upsideDown;
        bool drag;
        float value;
//        MetaInfo objInfo;
        QPointF offset;
        ViewConfig *config;

    signals:
        void valChanged(MetaInfos::Enum type,float val);

    public slots:
        void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
    };
}
#endif // CURSORVIEW_H
