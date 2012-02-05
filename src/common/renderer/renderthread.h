#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

//#include "precomp.h"

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
    void RenderStep(int step);

    QMap<int, RendererNode* > listOfSteps;
    Renderer *renderer;
    QReadWriteLock mutex;

    QSemaphore sem;
    int step;
    bool stop;

    RendererNode* currentNode;

    QMutex mutexRender;


#ifdef WIN32
    HMODULE DllAvRt;
    HANDLE hMmTask;
    typedef enum _PA_AVRT_PRIORITY
    {
        PA_AVRT_PRIORITY_LOW = -1,
        PA_AVRT_PRIORITY_NORMAL,
        PA_AVRT_PRIORITY_HIGH,
        PA_AVRT_PRIORITY_CRITICAL
    } PA_AVRT_PRIORITY, *PPA_AVRT_PRIORITY;
    typedef HANDLE WINAPI AVSETMMTHREADCHARACTERISTICS(LPCSTR, LPDWORD TaskIndex);
    typedef BOOL WINAPI AVREVERTMMTHREADCHARACTERISTICS(HANDLE);
    typedef BOOL WINAPI AVSETMMTHREADPRIORITY(HANDLE, PA_AVRT_PRIORITY);
    AVSETMMTHREADCHARACTERISTICS* FunctionAvSetMmThreadCharacteristics;
    AVREVERTMMTHREADCHARACTERISTICS* FunctionAvRevertMmThreadCharacteristics;
    AVSETMMTHREADPRIORITY* FunctionAvSetMmThreadPriority;
#endif
};

#endif // RENDERTHREAD_H
