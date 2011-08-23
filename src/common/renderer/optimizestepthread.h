#ifndef OPTIMIZESTEPTHREAD_H
#define OPTIMIZESTEPTHREAD_H

class RendererNode;
class OptimizeStepThread
{
public:
    OptimizeStepThread();
    OptimizeStepThread(const OptimizeStepThread &th);
    bool GetMergedNode(RendererNode **node) const;
    void AddToModel(QStandardItemModel *model, int row, int col) const;
    void UpdateView( QStandardItemModel *model ) const;

    QList<RendererNode*>listOfNodes;
    long cpuTime;
    RendererNode* spanFor;
};

#endif // OPTIMIZESTEPTHREAD_H
