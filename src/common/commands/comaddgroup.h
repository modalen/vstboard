#ifndef COMADDGROUP_H
#define COMADDGROUP_H

class ProgramsModel;
class ComAddGroup : public QUndoCommand
{
public:
    ComAddGroup(ProgramsModel *model,
                int row=-1,
                QByteArray *data=0,
                QUndoCommand *parent=0);
    void undo();
    void redo();

private:
    ProgramsModel *model;
    int row;
    QByteArray data;
    bool done;
};

#endif // COMADDGROUP_H
