#ifndef PROGRAMMANAGER_H
#define PROGRAMMANAGER_H

#include "precomp.h"
#include "msghandler.h"

class Program
{
public:
    ~Program() {
        //delete prog in container
    }

    quint32 id;
    QString name;

    void GetInfos(MsgObject &msg) const {
        msg.prop["name"]=name;
        msg.prop["id"]=id;
    }
};

class Group
{
public:
    ~Group() {
        listPrograms.clear();

        //delete prog in container
    }

    void GetInfos(MsgObject &msg) const {
        msg.prop["name"]=name;
        msg.prop["id"]=id;

        int cpt=0;
        foreach(const Program &prg, listPrograms) {
            MsgObject msgProg(-1,cpt);
            prg.GetInfos(msgProg);
            msg.children << msgProg;
            ++cpt;
        }
    }

    quint32 id;
    QString name;
    QList<Program> listPrograms;
};

class MainHost;
class ProgramManager : public QObject, public MsgHandler
{
Q_OBJECT
public:

    ProgramManager(MainHost *myHost);
    virtual ~ProgramManager();

    friend QDataStream & operator<< (QDataStream&, ProgramManager&);
    friend QDataStream & operator>> (QDataStream&, ProgramManager&);

    inline bool IsDirty() { return dirtyFlag; }
    inline void SetDirty(bool dirty=true) { dirtyFlag=dirty; }

    inline quint16 GetNextProgId() { return nextProgId++; }
    inline quint16 GetNextGroupId() { return nextGroupId++; }

    inline quint16 GetCurrentMidiGroup() const { return currentMidiGroup; }
    inline quint16 GetCurrentMidiProg() const { return currentMidiProg; }

    bool ChangeProgNow(int midiGroupNum, int midiProgNum);
    bool ValidateProgChange(int midiGroupNum, int midiProgNum);

    bool userWantsToUnloadGroup();
    bool userWantsToUnloadProgram();
    bool userWantsToUnloadProject();
    bool userWantsToUnloadSetup();

    void BuildDefaultPrograms();

    void ReceiveMsg(const MsgObject &msg);

private:
    void Clear();
    void UserChangeProg(quint16 prog, quint16 grp);
    void ValidateMidiChange(quint16 prog, quint16 grp);
    bool FindCurrentProg();

    QTimer updateTimer;
    bool orderChanged;

    QDataStream & toStream (QDataStream &out);
    QDataStream & fromStream (QDataStream &in);

    MainHost *myHost;

    QList<Group> listGroups;

    quint16 nextGroupId;
    quint16 nextProgId;
    bool dirtyFlag;
    quint8 groupAutosaveState;
    quint8 progAutosaveState;

    quint16 currentMidiGroup;
    quint16 currentMidiProg;
    quint32 currentGroupId;
    quint32 currentProgId;

signals:
    void ProgChanged(quint32 progId);
    void GroupChanged(quint32 grpId);
    void ProgDelete(quint32 progId);
    void GroupDelete(quint32 grpId);
    void MidiProgChanged(quint16 prog);
    void MidiGroupChanged(quint16 grp);

public slots:
    void UserChangeGroup(quint16 grp);
    void UserChangeProg(quint16 prog);

private slots:
    void UpdateView();
};

QDataStream & operator<< (QDataStream& out, ProgramManager& value);
QDataStream & operator>> (QDataStream& in, ProgramManager& value);

#endif // PROGRAMMANAGER_H
