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

#ifndef MINMAXPINVIEW_H
#define MINMAXPINVIEW_H

#include "connectablepinview.h"
#include "cursorview.h"

namespace View {
    class MinMaxPinView : public ConnectablePinView
    {
        Q_OBJECT
    public:
        MinMaxPinView(const MetaInfo &info, float angle, QGraphicsItem * parent, ViewConfig *config);
        void UpdateModelIndex(const MetaInfo &info);

    protected:
        void CreateCursors();
        void UpdateScaleView();
        CursorView *inMin;
        CursorView *inMax;
        CursorView *outMin;
        CursorView *outMax;
        QGraphicsPolygonItem *scaledView;
        bool cursorCreated;
    public slots:
        void LimitChanged(MetaInfos::Enum type,float val);
    };
}
#endif // MINMAXPINVIEW_H
