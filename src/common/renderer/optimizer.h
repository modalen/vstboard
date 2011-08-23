#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "optimizerstep.h"
#include "debugmutex.h"

class Optimizer
{
public:
    Optimizer();
    ~Optimizer();
    void SetNbThreads(int nb);
    void NewListOfNodes(const QList<RendererNode*> & listNodes);
    void Optimize();
    const OptimizerStep* GetStep(int step) const;
    void SetStep(int step, OptimizerStep* s);
    const QMap<int, RendererNode* > GetThreadNodes(int thread) const;
    const QList<RendererNode*> GetListOfNodes() const;

    void BuildModel( QStandardItemModel *model );
    void UpdateView( QStandardItemModel *model );

protected:
    void Clear();
    QMap<int,OptimizerStep*>listOfSteps;
    int nbThreads;
    mutable DMutex mutex;
};

#endif // OPTIMIZER_H
