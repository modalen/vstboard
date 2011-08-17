#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include "mutexdebug.h"

class Renderer;
class RendererNode;
class RenderThread : public QThread
{
    Q_OBJECT

public:
    RenderThread(Renderer *renderer, int cpu, const QString &name);
    ~RenderThread();

    void run();
    void SetListOfSteps( const QMap<int, RendererNode* > &lst );
    void StartRenderStep( int s );
    QList<RendererNode*> GetListOfNodes();

    int currentCpu;

protected:
    void ResetSteps();
    void Stop();
    void RenderStep(int step);

    QMap<int, RendererNode* > listOfSteps;
    Renderer *renderer;
    QReadWriteLock mutex;

    QSemaphore sem;
    int step;
    bool stop;

    RendererNode* currentNode;

    DMutex mutexRender;
};

#endif // RENDERTHREAD_H
