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

#ifndef PINVIEW_H
#define PINVIEW_H

//#include "precomp.h"
#include "globals.h"
#include "objectinfo.h"
#include "viewconfig.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif

namespace Connectables {
    class Pin;
}

namespace View {
    class CableView;
    class PinView : public QGraphicsWidget, public MetaInfo
    {
    Q_OBJECT

    public:

        explicit PinView(const MetaInfo &info, float angle, QGraphicsItem * parent, ViewConfig *config);
        void AddCable(CableView *cable);
        void RemoveCable(CableView *cable);
        void UpdateCablesPosition();

        /*!
          Called when the model changed
          \param index the model index of the pin
          \todo does the parameter can be removed since the index is stored anyway ?
          */
        virtual void UpdateModelIndex(const MetaInfo &info) {}

        /*!
          Set the model index and update the view
          \param index the new model index
          */
//        virtual void SetPinModelIndex(const MetaInfo &info) {pinInfo = info; UpdateModelIndex(info);}

        virtual const QPointF pinPos() const;

        /// temporary cable for drag&drop
        static CableView *currentLine;

    protected:
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        QVariant itemChange ( GraphicsItemChange change, const QVariant & value );
        virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
        virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent  * event );
        virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent  * event );
        void dragLeaveEvent( QGraphicsSceneDragDropEvent  * event );
        void dragEnterEvent ( QGraphicsSceneDragDropEvent * event );
        void dragMoveEvent ( QGraphicsSceneDragDropEvent * event );
        void dropEvent ( QGraphicsSceneDragDropEvent  * event );

        /// outline
        QGraphicsRectItem *outline;

        /// highlight
        QGraphicsRectItem *highlight;

        /// list of connected cables
        QList<CableView *> connectedCables;

        /// pin angle in rad
        float pinAngle;

        ViewConfig *config;

        QAction *actDel;
        QAction *actUnplug;

        Qt::CursorShape defaultCursor;

    signals:
        /*!
          emitted when a pin is drag&droped over another one
          \param outputPin
          \param inputPin
          */
        void ConnectPins(const MetaInfo &outputPin, const MetaInfo &inputPin);

        /*!
          emitted on double click to remove all the connected cables
          */
        void RemoveCablesFromPin(const MetaInfo & pin);

        void RemovePin(const MetaInfo &pin);

    public slots:
        /// update the vu-meter, called by a timer
        virtual void updateVu() {}
        virtual void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color) {}
        void RemovePin();
        void Unplug();

    friend class Cable;
    };
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // PINVIEW_H
