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

#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif

//#include "precomp.h"

#include "views/maingraphicsview.h"

class MainHost;
class MetaInfo;
namespace View {
    class MainContainerView;

    class SceneView : QObject
    {
        Q_OBJECT
    public:
        explicit SceneView(MainHost *myHost, MainGraphicsView *viewHost, MainGraphicsView *viewProject, MainGraphicsView *viewProgram, MainGraphicsView *viewGroup,QWidget *parent = 0);

        void SetParkings(QWidget *progPark, QWidget *groupPark);

        void AddObj(const MetaInfo &info);
        void DelObj(quint32 objId);
        void UpdateObj(const MetaInfo &info);

        MainGraphicsView *viewHost;
        MainGraphicsView *viewProject;
        MainGraphicsView *viewProgram;
        MainGraphicsView *viewGroup;

    protected:

        //we need parent objects to avoid a bug in qgraphicssene
        QGraphicsRectItem *rootObjHost;
        QGraphicsRectItem *rootObjProject;
        QGraphicsRectItem *rootObjProgram;
        QGraphicsRectItem *rootObjInsert;

        QMap<quint32,QObject*>mapViewItems;
        QMap<quint32,MetaInfo>mapMetaInfos;

        QGraphicsScene *sceneHost;
        QGraphicsScene *sceneProject;
        QGraphicsScene *sceneProgram;
        QGraphicsScene *sceneGroup;

        MainContainerView *hostContainerView;
        MainContainerView *projectContainerView;
        MainContainerView *programContainerView;
        MainContainerView *groupContainerView;

        QWidget *progParking;
        QWidget *groupParking;

        QTimer *timerFalloff;
        MainHost *myHost;

    public slots:
        void ConnectPins(const MetaInfo &pinOut, const MetaInfo &pinIn);
        void RemoveCablesFromPin(const MetaInfo &pin);
        void RemovePin(const MetaInfo &pin);
        void RemoveObjWithCables(const MetaInfo &obj);
        void RemoveObjKeepCables(const MetaInfo &obj);

        void ToggleHostView(bool show);
        void ToggleProjectView(bool show);
        void ToggleProgramView(bool show);
        void ToggleInsertView(bool show);

    protected slots:
        void graphicObjectRemoved ( QObject* obj);
    };
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // SCENEVIEW_H