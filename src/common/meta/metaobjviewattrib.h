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

#ifndef METAOBJVIEWATTRIB_H
#define METAOBJVIEWATTRIB_H

#include "metadata.h"

class MetaObjViewAttrib : public MetaData
{
public:
    MetaObjViewAttrib() :
        MetaData(MetaType::objViewAttrib)
    {}

    void SetPosition(const QPointF &pos) {SetMeta<QPointF>(metaT::Position,pos);}
    QPointF Position() const {return GetMetaData<QPointF>(metaT::Position);}

    void SetEditorVisible(const bool visible) {SetMeta<bool>(metaT::EditorVisible,visible);}
    bool EditorVisible() const {return GetMetaData<bool>(metaT::EditorVisible);}

    void SetEditorSize(const QSize &size) {SetMeta<QSize>(metaT::EditorSize,size);}
    QSize EditorSize() const {return GetMetaData<QSize>(metaT::EditorSize);}

    void SetEditorPosition(const QPoint &pos) {SetMeta<QPointF>(metaT::EditorPosition,pos);}
    QPoint EditorPosition() const {return GetMetaData<QPoint>(metaT::EditorPosition);}

    void SetEditorVScroll(const int &v) {SetMeta<int>(metaT::EditorVScroll,v);}
    int EditorVScroll() const {return GetMetaData<int>(metaT::EditorVScroll);}

    void SetEditorHScroll(const int &h) {SetMeta<int>(metaT::EditorHScroll,h);}
    int EditorHScroll() const {return GetMetaData<int>(metaT::EditorHScroll);}

    bool operator!=(const MetaObjViewAttrib &c) {
        return (Position()!=c.Position()
                || EditorVisible()!=c.EditorVisible()
                || EditorSize()!=c.EditorSize()
                || EditorPosition()!=c.EditorPosition()
                || EditorVScroll()!=c.EditorVScroll()
                || EditorHScroll()!=c.EditorHScroll()
                );
    }

};


#endif // METAOBJVIEWATTRIB_H
