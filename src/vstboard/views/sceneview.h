/******************************************************************************
#    Copyright 2010 Rapha�l Fran�ois
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
******************************************************************************/

#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include "precomp.h"
#include "connectables/connectioninfo.h"
#include "maingraphicsview.h"

namespace View {
    class SceneView : public QAbstractItemView
    {
        Q_OBJECT
    public:
        explicit SceneView(MainGraphicsView *viewHost, MainGraphicsView *viewProject, MainGraphicsView *viewProgram,QWidget *parent = 0);

        QRect visualRect(const QModelIndex &index) const {return QRect();}
        void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) {}
        QModelIndex indexAt(const QPoint &point) const {return QModelIndex();}

        MainGraphicsView *viewHost;
        MainGraphicsView *viewProject;
        MainGraphicsView *viewProgram;

    protected:

        //we need parent objects to avoid a bug in qgraphicssene
        QGraphicsRectItem *rootObjHost;
        QGraphicsRectItem *rootObjProject;
        QGraphicsRectItem *rootObjProgram;

        QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) {return QModelIndex();}
        int horizontalOffset() const {return 0;}
        int verticalOffset() const {return 0;}
        bool isIndexHidden(const QModelIndex &index) const {return false;}
        void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) {}
        QRegion visualRegionForSelection(const QItemSelection &selection) const {return QRegion();}

        QModelIndex traverseTroughIndexes ( QModelIndex index );

        QHash<QPersistentModelIndex,QObject*>hashItems;
        QMap<ConnectionInfo,QPersistentModelIndex>mapConnectionInfo;

        QGraphicsScene *sceneHost;
        QGraphicsScene *sceneProject;
        QGraphicsScene *sceneProgram;
        QTimer *timerFalloff;

    signals:
        void hostShown(bool shown);
        void projectShown(bool shown);
        void programShown(bool shown);

    public slots:
        void ConnectPins(ConnectionInfo pinOut,ConnectionInfo pinIn);
        void RemoveCablesFromPin(ConnectionInfo pin);
        void ToggleHostView(bool show);
        void ToggleProjectView(bool show);
        void ToggleProgramView(bool show);

    protected slots:
        void dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight  );
        void rowsAboutToBeRemoved ( const QModelIndex & parent, int start, int end  );
        void rowsInserted ( const QModelIndex & parent, int start, int end  );
        void graphicObjectRemoved ( QObject* obj);
    };
}
#endif // SCENEVIEW_H