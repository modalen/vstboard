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

#include "renderer.h"
#include "pathsolver.h"
#include "connectables/objects/object.h"
#include "solvernode.h"
#include "mainhost.h"
#include "views/configdialog.h"
#include "renderernode.h"

Renderer::Renderer(MainHost *myHost)
    : QObject(),
      numberOfThreads(0),
      numberOfSteps(0),
      stop(false),
      newNodes(false),
      needOptimize(false),
      nextOptimize(0),
//      needBuildModel(false),
      sem(0),
      myHost(myHost),
      model(0)
{
    SET_MUTEX_NAME(mutexNodes,"mutexNodes renderer");
    SET_MUTEX_NAME(mutexOptimize,"mutexOptimize renderer");
    SET_READWRITELOCK_NAME(rwlock,"rwlock renderer");
    SET_SEMAPHORE_NAME(sem,"sem renderer");

    maxNumberOfThreads = ConfigDialog::defaultNumberOfThreads(myHost);
    InitThreads();
}

Renderer::~Renderer()
{
    Clear();
}

void Renderer::UpdateView()
{
    if(!model)
        return;

    if(!rwlock.tryLockForRead())
        return;

    optimizer.BuildModel( model );
    optimizer.UpdateView( model );

    int i = 0;
    foreach(RenderThread *th, listOfThreads) {
        model->setHorizontalHeaderItem(i, new QStandardItem( QString("%1 (cpu:%2)").arg(i).arg(th->currentCpu) ));
        ++i;
    }
    rwlock.unlock();
}

void Renderer::Clear()
{
    rwlock.lockForWrite();
    stop=true;
    numberOfThreads=0;
    numberOfSteps=0;
    qDeleteAll(listOfThreads);
    listOfThreads.clear();
    rwlock.unlock();
}

void Renderer::SetNbThreads(int nbThreads)
{
    if(nbThreads<=0) nbThreads = 2;

    rwlock.lockForWrite();
    qDeleteAll(listOfThreads);
    listOfThreads.clear();
    maxNumberOfThreads = nbThreads;
    InitThreads();
    rwlock.unlock();
}

void Renderer::LoadNodes(const QList<RendererNode*> & listNodes)
{
    if(listNodes.isEmpty()) {
        myHost->SetSolverUpdateNeeded();
        return;
    }

    mutexNodes.lock();
    //copy nodes
    QList<RendererNode*> tmpListOfNodes;
    foreach(RendererNode *n, listNodes) {
        tmpListOfNodes << new RendererNode(*n);
    }
    optimizer.NewListOfNodes( tmpListOfNodes );
    newNodes=true;
    mutexNodes.unlock();

    mutexOptimize.lock();
    nextOptimize=0;
    needOptimize=true;
    mutexOptimize.unlock();
}

const QList<RendererNode*> Renderer::SaveNodes() const
{
    QMutexLocker l(&mutexNodes);
    return optimizer.GetListOfNodes();
}

void Renderer::OnNewRenderingOrder(const QList<SolverNode*> & listNodes)
{
    mutexNodes.lock();

    //copy nodes
    QList<RendererNode*> tmpListOfNodes;
    foreach(SolverNode *n, listNodes) {
        tmpListOfNodes << new RendererNode(*n);
    }

    optimizer.SetNbThreads(maxNumberOfThreads);
    optimizer.NewListOfNodes( tmpListOfNodes );

    newNodes=true;
    mutexNodes.unlock();

    if(maxNumberOfThreads>1) {
        mutexOptimize.lock();
        nextOptimize=50;
        QTimer::singleShot(nextOptimize, this, SLOT(Optimize()));
        mutexOptimize.unlock();
    }
}

void Renderer::StartRender()
{
    if(!rwlock.tryLockForRead(5)) {
        LOG("can't lock renderer");
        return;
    }

    if(stop) {
        rwlock.unlock();
        return;
    }

    mutexNodes.lock();
    if(newNodes) {
        newNodes=false;
        mutexNodes.unlock();
        GetStepsFromOptimizer();
    } else {
        mutexNodes.unlock();
    }

    mutexOptimize.lock();
    if(needOptimize) {
        needOptimize=false;
        mutexOptimize.unlock();

        QList<RendererNode*> tmpListOfNodes;
        foreach(RenderThread *th, listOfThreads) {
            tmpListOfNodes << th->GetListOfNodes();
        }
        optimizer.NewListOfNodes( tmpListOfNodes );
        optimizer.Optimize();
        GetStepsFromOptimizer();
    } else {
        mutexOptimize.unlock();
    }

    if(numberOfThreads<=0 || numberOfSteps<=0) {
            rwlock.unlock();
            return;
        }

        int maxRenderingTime = myHost->GetSampleRate()/myHost->GetBufferSize();
        int renderTimeout = 2000;

        if(sem.available()!=0) {
            LOG("semaphore available before rendering !"<<sem.available());
            sem.acquire(sem.available());
        }

        sem.release(maxNumberOfThreads);
        for(int currentStep=-1; currentStep<numberOfSteps; currentStep++) {

            if(sem.tryAcquire(maxNumberOfThreads,maxRenderingTime)) {
                foreach( RenderThread *th, listOfThreads) {
                    th->StartRenderStep( currentStep );
                }
            } else if( sem.tryAcquire(maxNumberOfThreads,renderTimeout) ) {
                LOG("renderer step"<<currentStep-1<<" took more than the total available time ("<<maxRenderingTime<<"ms) , finished threads:"<< sem.available() << "/" << maxNumberOfThreads);
                foreach( RenderThread *th, listOfThreads) {
                    th->StartRenderStep( currentStep );
                }
            } else {
                LOG("renderer step"<<currentStep-1<<"timeout, finished threads:"<< sem.available() << "/" << maxNumberOfThreads);
                sem.acquire(sem.available());
                rwlock.unlock();
                return;
            }
        }

        //wait for the last step to finish
        if(sem.tryAcquire(maxNumberOfThreads,maxRenderingTime)) {
            rwlock.unlock();
            return;
        } else if(sem.tryAcquire(maxNumberOfThreads,renderTimeout) ) {
            LOG("renderer last step took more than the total available time ("<<maxRenderingTime<<"ms) , finished threads:"<< sem.available() << "/" << maxNumberOfThreads);
        } else {
            LOG("renderer last step timeout, finished threads:"<< sem.available() << "/" << maxNumberOfThreads);
            sem.acquire(sem.available());
        }


        if(sem.available()!=0) {
            LOG("semaphore available after rendering !"<<sem.available());
            sem.acquire(sem.available());
        }

        rwlock.unlock();
}

void Renderer::Optimize()
{
    mutexOptimize.lock();
    needOptimize=true;

    switch(nextOptimize) {
        case 50:
            nextOptimize=500;
            break;
        case 500:
            nextOptimize=5000;
            break;
        default:
            nextOptimize=0;
            break;
    }

    if(nextOptimize!=0)
        QTimer::singleShot(nextOptimize, this, SLOT(Optimize()));

    mutexOptimize.unlock();
}

void Renderer::GetStepsFromOptimizer()
{
    numberOfSteps = -1;
    numberOfThreads = maxNumberOfThreads;

    for(int th=0; th<maxNumberOfThreads; th++) {
        const QMap<int, RendererNode* >lst = optimizer.GetThreadNodes(th);

        RenderThread *thread = listOfThreads.value(th);
        thread->SetListOfSteps( lst );

        if(!lst.isEmpty()) {
            int lastStep = lst.uniqueKeys().last();
            if( lastStep >= numberOfSteps) {
                numberOfSteps = lastStep+1;
            }
        }
    }

    emit ModelUpdated();
}

void Renderer::InitThreads()
{
    for(int i=0; i<maxNumberOfThreads; i++) {
        RenderThread *th = new RenderThread(this, i, QString("RenderThread %1").arg(i));
        listOfThreads << th;
        th->start(QThread::TimeCriticalPriority);
    }
}

