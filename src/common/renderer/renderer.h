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

#ifndef RENDERER_H
#define RENDERER_H

//#include "precomp.h"
#include "connectables/objects/object.h"
#include "renderthread.h"
#include "optimizer.h"
#include "debugsemaphore.h"
#include "debugreadwritelock.h"

class MainHost;
class RendererNode;
class Renderer : public QObject
{
    Q_OBJECT

public:
    Renderer(MainHost *myHost);
    ~Renderer();
    void SetNbThreads(int nbThreads);
    void SetEnabled(bool enabled) {stop=!enabled;}
    void OnNewRenderingOrder(const QList<SolverNode*> & listNodes);
    QStandardItemModel * GetModel();
    Optimizer * GetOptimizer() { return &optimizer; }

    void LoadNodes(const QList<RendererNode*> & listNodes);
    QList<RendererNode*> SaveNodes();

protected:
    void InitThreads();
    void GetStepsFromOptimizer();
    void Clear();

    int maxNumberOfThreads;
    int numberOfThreads;
    int numberOfSteps;
    bool stop;

    DMutex mutexNodes;
    bool newNodes;
    DMutex mutexOptimize;
    bool needOptimize;
    int nextOptimize;
    QList<RenderThread*>listOfThreads;
    DReadWriteLock rwlock;
    DSemaphore sem;
    MainHost *myHost;
    Optimizer optimizer;


    bool needBuildModel;
    QTimer updateViewTimer;
    QStandardItemModel model;

public slots:
    void StartRender();
    void Optimize();

    void UpdateView();


friend class RenderThread;
};

#endif // RENDERER_H
