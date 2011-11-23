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

#include "renderthread.h"
#include "renderer.h"
#include "renderernode.h"

RenderThread::RenderThread(Renderer *renderer, int cpu, const QString &name)
    : QThread(renderer),
      renderer(renderer),
      sem(0),
      stop(false),
      currentCpu(cpu),
      currentNode(0)
{
    setObjectName(name);

#ifdef WIN32
    DllAvRt = NULL;
    FunctionAvSetMmThreadCharacteristics = NULL;
    FunctionAvRevertMmThreadCharacteristics = NULL;
    FunctionAvSetMmThreadPriority = NULL;

    DllAvRt = LoadLibrary(TEXT("avrt.dll"));
    if (DllAvRt != NULL)
    {
        FunctionAvSetMmThreadCharacteristics = (AVSETMMTHREADCHARACTERISTICS*)GetProcAddress(DllAvRt,"AvSetMmThreadCharacteristicsA");
        FunctionAvRevertMmThreadCharacteristics = (AVREVERTMMTHREADCHARACTERISTICS*)GetProcAddress(DllAvRt, "AvRevertMmThreadCharacteristics");
        FunctionAvSetMmThreadPriority = (AVSETMMTHREADPRIORITY*)GetProcAddress(DllAvRt, "AvSetMmThreadPriority");
    }
#endif
}

RenderThread::~RenderThread()
{
    ResetSteps();
    stop=true;
    sem.release();

    wait(1000);
}


void RenderThread::run()
{
//    SetThreadIdealProcessor( GetCurrentThread(), currentCpu );
//not available on XP
//    currentCpu = GetCurrentProcessorNumber();

#ifdef WIN32
    /* If we have access to AVRT.DLL (Vista and later), use it */
    if (FunctionAvSetMmThreadCharacteristics != NULL) {
        DWORD dwTask = 0;
        HANDLE hMmTask = FunctionAvSetMmThreadCharacteristics(QString("vstboard%1").arg(currentCpu).toStdString().c_str(), &dwTask);
        if (hMmTask != NULL && hMmTask != INVALID_HANDLE_VALUE) {
            BOOL bret = FunctionAvSetMmThreadPriority(hMmTask, PA_AVRT_PRIORITY_CRITICAL);
            if (!bret) {
                LOG("can't set msc priority");
            }
        }
        else {
            LOG("can't set msc priority, avrt.dll not loaded");
        }
    }
#endif

    while(!stop) {
        sem.acquire();
        RenderStep(step);
    }

#ifdef WIN32
    if (hMmTask != NULL) {
        FunctionAvRevertMmThreadCharacteristics(hMmTask);
    }
    if(DllAvRt)
        FreeLibrary(DllAvRt);
#endif
}

void RenderThread::RenderStep(int step)
{
    //new loop : reset the nodes
    if(step==-1) {
        mutex.lockForRead();

        //reset counters
        QMap<int, RendererNode* >::iterator i = listOfSteps.begin();
        while (i != listOfSteps.end()) {
            RendererNode *node = i.value();
            if(node) {
                node->NewRenderLoop();
            }
            ++i;
        }
        mutex.unlock();
        renderer->sem.release();
        return;
    }

    //a spanned node is rendering
    if(currentNode && currentNode->maxRenderOrder == step) {
        //this is the last step, we have to wait for the node to be rendered
        mutexRender.lock();
        currentNode=0;
        mutexRender.unlock();
        renderer->sem.release();
        return;
    }

    //there's a node to render at this step
    RendererNode *n = listOfSteps.value(step,0);
    if(n!=0) {
        currentNode = n;

        if(currentNode->maxRenderOrder == step) {
            //not a spanned node, we have to render it now
            currentNode->Render();
            currentNode=0;

            //release the step when done
            renderer->sem.release();
            return;
        } else {
            //this step is spanned, release the step now
            renderer->sem.release();

            //and render when we can
            mutexRender.lock();
            currentNode->Render();
            mutexRender.unlock();
            return;
        }
    }



    //nothing to do
    renderer->sem.release();
    return;
}

void RenderThread::ResetSteps()
{
    foreach( RendererNode *node, listOfSteps ) {
        delete node;
    }
    listOfSteps.clear();
}

void RenderThread::SetListOfSteps( const QMap<int, RendererNode* > &lst )
{
    mutex.lockForWrite();
    ResetSteps();
    listOfSteps = lst;
    mutex.unlock();
}

void RenderThread::StartRenderStep( int s )
{
    step=s;
    sem.release();
}

QList<RendererNode*> RenderThread::GetListOfNodes()
{
    mutex.lockForRead();
    QList<RendererNode*> tmpList;

    foreach(RendererNode *node, listOfSteps) {
        if(node) {
            RendererNode *newNode = new RendererNode(*node);
            tmpList << newNode;
            tmpList << newNode->GetListOfMergedNodes();
            newNode->ClearMergedNodes();
        }
    }
    mutex.unlock();
    return tmpList;
}
