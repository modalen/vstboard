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
    currentGroupId(0),
    currentProgId(0),
    currentMidiGroup(0),
    currentMidiProg(0),
    dirtyFlag(false)
{
    updateTimer.setInterval(20);
    updateTimer.setSingleShot(true);
    connect(&updateTimer, SIGNAL(timeout()),
            this, SLOT(UpdateView()));
}

ProgramManager::~ProgramManager()
{
    Clear();
}

void ProgramManager::Clear()
{
    nextGroupId=1;
    nextProgId=1;
    listGroups.clear();
}

void ProgramManager::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop["actionType"]=="fullUpdate") {

        QList<Group> oldListGroups = listGroups;
        listGroups.clear();

        foreach(const MsgObject &msgGrp, msg.children) {
            Group grp;
            if(msgGrp.prop.contains("id"))
                grp.id = msgGrp.prop["id"].toInt();
            else {
                orderChanged=true;
                updateTimer.start();
                grp.id = GetNextGroupId();
            }
            grp.name = msgGrp.prop["name"].toString();


            if(msgGrp.children.isEmpty()) {
                //create a default program in the group
                Program prg;
                prg.id = GetNextProgId();
                grp.listPrograms << prg;
                orderChanged=true;
                updateTimer.start();
            } else {
                foreach(const MsgObject &msgPrg, msgGrp.children) {
                    Program prg;
                    if(msgPrg.prop.contains("id"))
                        prg.id = msgPrg.prop["id"].toInt();
                    else {
                        orderChanged=true;
                        updateTimer.start();
                        prg.id = GetNextProgId();
                    }
                    prg.name = msgPrg.prop["name"].toString();
                    grp.listPrograms << prg;
                }
            }

            listGroups << grp;
        }

        oldListGroups.clear();

        //find the current prog
        if(FindCurrentProg())
            return;
    }

    if(msg.prop.contains("currentGroup")) {
        int grp = msg.prop["currentGroup"].toInt();
        UserChangeGroup(grp);
    }
    if(msg.prop.contains("currentProg")) {
        int prg = msg.prop["currentProg"].toInt();
        UserChangeProg(prg);
    }

//    if(msg.prop.contains("removeProg")) {
//        int prg = msg.prop["removeProg"].toInt();
//        int grp = msg.prop["group"].toInt();

//        if(listGroups[grp].listPrograms.count()<=1)
//            return;

//        if(listGroups.count()>grp && listGroups[grp].listPrograms.count()>prg) {
//            listGroups[grp].listPrograms.removeAt( prg );
//            msgCtrl->SendMsg(msg);

//            if(grp==currentMidiGroup && prg==currentMidiProg)
//                ValidateProgChange(grp,prg);
//        }
//    }
//    if(msg.prop.contains("removeGroup")) {
//        if(listGroups.count()<=1)
//            return;

//        int grp = msg.prop["removeGroup"].toInt();
//        if(listGroups.count()>grp) {
//            listGroups.removeAt(grp);
//            msgCtrl->SendMsg(msg);

//            if(grp==currentMidiGroup)
//                ValidateProgChange(grp,currentMidiProg);
//        }
//    }

//    if(msg.prop.contains("addProg")) {
//        Program newPrg;
//        newPrg.id=GetNextProgId();
//        newPrg.name=tr("new prog");
//        listGroups[currentMidiGroup].listPrograms.insert( msg.prop["addProg"].toInt(), newPrg );
//    }
//    if(msg.prop.contains("addGroup")) {
//        Group newGroup;
//        newGroup.id=GetNextGroupId();
//        newGroup.name=tr("new grp");
//        listGroups.insert( msg.prop["addGroup"].toInt(), newGroup );
//    }

//    if(msg.prop.contains("name")) {
//        if(msg.prop.contains("prog")) {
//            int grp = msg.prop["group"].toInt();
//            int prg = msg.prop["prog"].toInt();
//            listGroups[grp].listPrograms[prg].name = msg.prop["name"].toString();
//        } else {
//            int grp = msg.prop["group"].toInt();
//            listGroups[grp].name = msg.prop["name"].toString();
//        }
//        msgCtrl->SendMsg(msg);
//    }
}

void ProgramManager::UpdateView()
{
    MsgObject msg(-1,GetIndex());

    if(orderChanged) {
        orderChanged=false;
        msg.prop["actionType"]="fullUpdate";

        int cpt=0;
        foreach(const Group &grp, listGroups) {
            MsgObject msgGrp(GetIndex(),cpt);
            grp.GetInfos(msgGrp);
            msg.children << msgGrp;
            ++cpt;
        }
    }

    msg.prop["currentGroup"]=currentMidiGroup;
    msg.prop["currentProg"]=currentMidiProg;

    msgCtrl->SendMsg(msg);
}

void ProgramManager::BuildDefaultPrograms()
{
    Clear();

    for(unsigned int grpn=0; grpn<3; grpn++) {
        Group grp;
        grp.name = QString("Grp%1").arg(grpn);
        grp.id = GetNextGroupId();

        for(unsigned int prgn=0; prgn<5; prgn++) {
            Program prg;
            prg.name = QString("Prg%1").arg(prgn);
            prg.id = GetNextProgId();
            grp.listPrograms << prg;
        }
        listGroups << grp;
    }

    groupAutosaveState=Qt::Checked;
    progAutosaveState=Qt::Checked;

    currentMidiGroup=127;
    currentMidiProg=127;
    ChangeProgNow(0,0);
    SetDirty(false);

    orderChanged=true;
    updateTimer.start();
}

bool ProgramManager::userWantsToUnloadGroup()
{
    //drop changes
    if(groupAutosaveState == Qt::Unchecked)
        return true;

    //no changes
    if(!myHost->groupContainer->IsDirty())
        return true;

    //auto save
    if(groupAutosaveState == Qt::Checked) {
        myHost->groupContainer->SaveProgram();
        SetDirty();
        return true;
    }

    //prompt user

    return true;
}

bool ProgramManager::userWantsToUnloadProgram()
{
    //drop changes
    if(progAutosaveState == Qt::Unchecked)
        return true;

    //no changes
    if(!myHost->programContainer->IsDirty())
        return true;

    //auto save
    if(progAutosaveState == Qt::Checked) {
        myHost->programContainer->SaveProgram();
        SetDirty();
        return true;
    }

    //prompt user

    return true;
}

bool ProgramManager::userWantsToUnloadProject()
{
    Qt::CheckState onUnsaved = (Qt::CheckState)myHost->settings->GetSetting("onUnsavedProject",Qt::PartiallyChecked).toInt();

    //drop changes
    if(onUnsaved == Qt::Unchecked)
        return true;

    //unload current program
    if(!userWantsToUnloadProgram())
        return false;

    //unload current group
    if(!userWantsToUnloadGroup())
        return false;

    //no changes
    if(!IsDirty())
        return true;

    //auto save
    if(onUnsaved == Qt::Checked) {
        myHost->SaveProjectFile();
        return true;
    }

    //prompt user

    return true;
}

bool ProgramManager::userWantsToUnloadSetup()
{
    Qt::CheckState onUnsaved = (Qt::CheckState)myHost->settings->GetSetting("onUnsavedSetup",Qt::PartiallyChecked).toInt();

    //drop changes
    if(onUnsaved == Qt::Unchecked)
        return true;

    //no changes
    if(!myHost->hostContainer->IsDirty())
        return true;

    //auto save
    if(onUnsaved == Qt::Checked) {
        myHost->SaveSetupFile();
        return true;
    }

    //prompt user


    return true;
}

void ProgramManager::UserChangeGroup(quint16 grp)
{
    if(listGroups.count()<=grp)
        grp=listGroups.count()-1;

    quint16 newPrg=currentMidiProg;
    quint16 nbPrg = listGroups[grp].listPrograms.count();
    if(newPrg>=nbPrg)
        newPrg=nbPrg-1;

    UserChangeProg(newPrg,grp);
}

void ProgramManager::UserChangeProg(quint16 prog, quint16 grp)
{
//    if(prog==currentMidiProg && grp==currentMidiGroup)
//        return;

//    if(listGroups.count()<=grp || listGroups[grp].listPrograms.count()<=prog)
//        return;

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
    quint16 nbPrg = listGroups[currentMidiGroup].listPrograms.count();
    if(prog>=nbPrg)
        prog=nbPrg-1;

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
    if(listGroups.count()<=midiGroupNum)
        midiGroupNum=0;

    if(listGroups.count()==0)
        return false;

    if(listGroups[midiGroupNum].listPrograms.count()<=midiProgNum)
        midiProgNum=0;

    if(listGroups[midiGroupNum].listPrograms.count()==0)
        return false;

    if(listGroups[midiGroupNum].id==currentGroupId && listGroups[midiGroupNum].listPrograms[midiProgNum].id==currentProgId)
        return false;

//    if(!userWantsToUnloadProgram()) {
//        emit ProgChanged( currentPrg );
//        return false;
//    }

    if(listGroups[midiGroupNum].id!=currentGroupId) {

//        if(!userWantsToUnloadGroup()) {
//            emit ProgChanged( currentPrg );
//            return false;
//        }

        currentGroupId = listGroups[midiGroupNum].id;
        emit GroupChanged( currentGroupId );
    }

    if(listGroups[midiGroupNum].listPrograms[midiProgNum].id!=currentProgId) {
        currentProgId = listGroups[midiGroupNum].listPrograms[midiProgNum].id;
        emit ProgChanged( currentProgId );

        SetDirty();
        ValidateMidiChange(midiProgNum,midiGroupNum);
    }

    return true;
}

void ProgramManager::ValidateMidiChange(quint16 prog, quint16 grp)
{
    if(grp!=currentMidiGroup) {
        currentMidiGroup=grp;
        emit MidiGroupChanged(currentMidiGroup);
    }
    if(prog!=currentMidiProg) {
        currentMidiProg=prog;
        emit MidiProgChanged(currentMidiProg);
    }

    updateTimer.start();

//    MsgObject msg(-1,GetIndex());
//    msg.prop["currentGroup"]=currentMidiGroup;
//    msg.prop["currentProg"]=currentMidiProg;
//    msgCtrl->SendMsg(msg);
}

bool ProgramManager::FindCurrentProg()
{
    int grpCpt=0;
    foreach(const Group &grp, listGroups) {
        int prgCpt=0;
        foreach(const Program &prg, grp.listPrograms) {
            if(prg.id==currentProgId) {
                ValidateMidiChange(prgCpt,grpCpt);
                return true;
            }
            prgCpt++;
        }
        ++grpCpt;
    }
    return false;
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
    Clear();

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

    orderChanged=true;
    updateTimer.start();

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
