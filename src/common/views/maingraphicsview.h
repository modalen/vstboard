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

#ifndef MAINGRAPHICSVIEW_H
#define MAINGRAPHICSVIEW_H

#include "precomp.h"

//namespace View {

    class MainGraphicsView : public QGraphicsView
    {
    Q_OBJECT
    public:
        MainGraphicsView(QWidget * parent = 0);
        MainGraphicsView(QGraphicsScene * scene, QWidget * parent = 0);
//        bool event(QEvent *event);

        QDataStream & toStream (QDataStream &) const;
        QDataStream & fromStream (QDataStream &);

        void SaveProgram();

        void ProgramToStream (int progId, QDataStream &out);
        void ProgramFromStream (int progId, QDataStream &in);

        void SetViewProgram(int progId);

    protected:
        void wheelEvent(QWheelEvent * event);
        void mousePressEvent ( QMouseEvent * event );
        void resizeEvent ( QResizeEvent * event );
        void scrollContentsBy ( int dx, int dy );
        void paintEvent ( QPaintEvent * event );

        struct SceneProg {
            qreal scale;
            qint16 scrollx;
            qint16 scrolly;
        };
        QMap<qint16,SceneProg>listPrograms;

        qint16 currentProgId;

    signals:
        void viewResized(QRectF trans);

    public slots:
        void zoomIn();
        void zoomOut();
        void zoomReset();
        void ForceResize();
        void SetViewProgram(const QModelIndex &idx);
        void RemoveViewProgram(int prg);
        void ClearViewPrograms();
    };

//}

QDataStream & operator<< (QDataStream & out, const MainGraphicsView& value);
QDataStream & operator>> (QDataStream & in, MainGraphicsView& value);

#endif // MAINGRAPHICSVIEW_H
