#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include "solvernode.h"
#include <QSemaphore>
#include <QReadWriteLock>

class Renderer;
class RenderThread : public QThread
{
    Q_OBJECT

public:
    RenderThread(Renderer *renderer, const QString &name);
    ~RenderThread();

    bool SetStep(SolverNode *node);
    void ResetSteps();
    int NeededModificationsToInsertNode(SolverNode *node, bool apply=false);
    bool MergeNodeInStep(SolverNode *node);

    void run();

    enum insertType {
        ND,
        postponeNodeInPlace,
        shortenNodeInPlace,
        postponeNewNode
    };

protected:
    void Stop();
    void RenderStep(int step);
    int FirstUsedStepInRange(int startStep, int endStep);
    SolverNode* FindNodeUsingStep(int step);
    bool PostponeNode(SolverNode *node, int minStep);
    bool ShortenNode(SolverNode *node, int maxStep);

    QMap<int, SolverNode* > listOfSteps;
    Renderer *renderer;
    QReadWriteLock mutex;

    QSemaphore sem;
    int step;
    bool stop;
    int lastStepForRendering;

friend class Renderer;
};

#endif // RENDERTHREAD_H
