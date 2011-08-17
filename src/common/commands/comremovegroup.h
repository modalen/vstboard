#ifndef COMREMOVEGROUP_H
#define COMREMOVEGROUP_H

class ProgramsModel;
class ComRemoveGroup : public QUndoCommand
{
public:
    ComRemoveGroup(ProgramsModel *model,
                   int row,
                   QUndoCommand *parent=0);
    void undo();
    void redo();

private:
    ProgramsModel *model;
    int row;
    QByteArray data;
    bool done;
};

#endif // COMREMOVEGROUP_H
