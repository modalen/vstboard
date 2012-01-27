#include "programmanager.h"
#include "mainhost.h"
#include "commands/comdiscardchanges.h"
#include "commands/comchangeprogram.h"

ProgramManager::ProgramManager(MainHost *myHost) :
    QObject(myHost),
    MsgHandler(myHost,FixedObjId::programsManager),
    myHost(myHost),
    nextGroupId(1),
    nextProgId(1),
    dirtyFlag(false)
{

}

ProgramManager::~ProgramManager()
{
    clear();
}

void ProgramManager::clear()
{
    listGroups.clear();
}

void ProgramManager::UserChangeGroup(quint16 grp)
{
    if(listGroups.count()<=grp)
        return;

    quint16 newPrg=currentMidiProg;
    if(listGroups.at(grp).listPrograms.count()<=currentMidiProg)
        newPrg=0;

    UserChangeProg(newPrg,grp);
}

void ProgramManager::UserChangeProg(quint16 prog, quint16 grp)
{
    if(prog==currentMidiProg && grp==currentMidiGroup)
        return;

    if(listGroups.count()<=grp || listGroups.at(grp).listPrograms.count()<=prog)
        return;

//    if(progAutosaveState == Qt::Unchecked && myHost->programContainer->IsDirty()) {
//        QUndoCommand *discardCom = new QUndoCommand( tr("Discard changes") );
//        if(grp != currentMidiGroup && groupAutosaveState == Qt::Unchecked && myHost->groupContainer->IsDirty())
//            new ComDiscardChanges(this, currentMidiGroup, -1, discardCom);
//        new ComDiscardChanges(this, currentMidiProg, currentMidiGroup, discardCom);
//        new ComChangeProgram(this, currentMidiGroup, currentMidiProg, grp, prog, discardCom);
//        myHost->undoStack.push( discardCom );
//    } else {
//        if(myHost->undoProgramChanges())
//            myHost->undoStack.push( new ComChangeProgram(this, currentMidiGroup, currentMidiProg, grp, prog) );
//    }

    ValidateProgChange(grp,prog);
}

void ProgramManager::UserChangeProg(quint16 prog)
{
    UserChangeProg(prog,currentMidiGroup);
}

bool ProgramManager::ChangeProgNow(int midiGroupNum, int midiProgNum)
{
    //if the program has not been changed, just return
    if( !ValidateProgChange( midiGroupNum, midiProgNum ) )
        return false;

    //if program changed, force the host to update
    myHost->UpdateSolverNow();
    return true;
}

bool ProgramManager::ValidateProgChange(int midiGroupNum, int midiProgNum)
{
    if(midiGroupNum==currentMidiGroup && midiProgNum==currentMidiProg)
        return false;

    if(listGroups.count()<=midiGroupNum)
        return false;

//    if(!userWantsToUnloadProgram()) {
//        emit ProgChanged( currentPrg );
//        return false;
//    }


    if(midiGroupNum!=currentMidiGroup) {

//        if(!userWantsToUnloadGroup()) {
//            emit ProgChanged( currentPrg );
//            return false;
//        }

        currentMidiGroup = midiGroupNum;
        emit GroupChanged( listGroups.at(currentMidiGroup).id );
        emit MidiGroupChanged( currentMidiGroup );
    }

    currentMidiProg = midiProgNum;
    emit ProgChanged( listGroups.at(currentMidiGroup).listPrograms.at(currentMidiProg).id );
    emit MidiProgChanged( currentMidiProg );

    SetDirty();
    return true;
}

QDataStream & ProgramManager::toStream (QDataStream &out)
{
    out << (quint16)listGroups.count();
    foreach(const Group &grp, listGroups) {
        out << grp.name;
        out << grp.id;


        out << (quint16)grp.listPrograms.count();
        foreach(const Program &prg, grp.listPrograms) {
            out << prg.name;
            out << prg.id;
        }
    }

    out << (quint16)currentMidiGroup;
    out << (quint16)currentMidiProg;
    out << groupAutosaveState;
    out << progAutosaveState;

    SetDirty(false);
    return out;
}

QDataStream & ProgramManager::fromStream (QDataStream &in)
{
    clear();

    quint16 nbgrp;
    in >> nbgrp;

    for(unsigned int i=0; i<nbgrp; i++) {

        Group grp;

        in >> grp.name;
        in >> grp.id;
        if(grp.id>=nextGroupId)
            nextGroupId=grp.id+1;

        quint16 nbprog;
        in >> nbprog;
        for(unsigned int j=0; j<nbprog; j++) {

            Program prg;
            in >> prg.name;
            in >> prg.id;
            if(prg.id>=nextProgId)
                nextProgId=prg.id+1;

            grp.listPrograms << prg;
        }

        listGroups << grp;
    }

    currentMidiGroup=127;
    currentMidiProg=127;
    quint16 grp;
    quint16 prg;
    in >> grp;
    in >> prg;
    ChangeProgNow(grp,prg);

    in >> (quint8&)groupAutosaveState;
    in >> (quint8&)progAutosaveState;

    SetDirty(false);
    return in;
}

QDataStream & operator<< (QDataStream& out, ProgramManager& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream& in, ProgramManager& value)
{
    return value.fromStream(in);
}
