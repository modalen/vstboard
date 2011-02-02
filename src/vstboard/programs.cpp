/**************************************************************************
#    Copyright 2010-2011 Rapha�l Fran�ois
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "programs.h"
#include "globals.h"
#include "mainhost.h"
#include "mainwindow.h"

Programs::Programs(MainHost *parent) :
    QObject(parent),
    model(0),
    nextGroupId(0),
    nextProgId(0),
    myHost(parent),
    progAutosaveState(Autosave::save),
    groupAutosaveState(Autosave::save),
    projectDirty(false),
    mainWindow(0)
{
    model=new ProgramsModel(parent);
}

void Programs::BuildModel()
{
    if(model)
        model->clear();

    groupAutosaveState=Autosave::save;
    progAutosaveState=Autosave::save;

    for(unsigned int grp=0; grp<3; grp++) {
        QStandardItem *grpItem = new QStandardItem(QString("Grp%1").arg(grp));
        grpItem->setData(NodeType::programGroup,UserRoles::nodeType);
        grpItem->setData(nextGroupId,UserRoles::value);
        nextGroupId++;
        grpItem->setDragEnabled(true);
        grpItem->setDropEnabled(false);
        grpItem->setEditable(true);

        QStandardItem *prgList = new QStandardItem();
        prgList->setDragEnabled(false);
        prgList->setDropEnabled(true);
        prgList->setEditable(false);

        for(unsigned int prg=0; prg<5; prg++) {
            QStandardItem *prgItem = new QStandardItem(QString("Prg%1").arg(prg));
            prgItem->setData(NodeType::program,UserRoles::nodeType);
            prgItem->setData(nextProgId,UserRoles::value);
//            prgItem->setData(nextProgId,Qt::ToolTipRole);
            nextProgId++;
            prgItem->setDragEnabled(true);
            prgItem->setDropEnabled(false);
            prgItem->setEditable(true);

            prgList->appendRow(prgItem);
        }
        grpItem->appendRow(prgList);
        model->invisibleRootItem()->appendRow(grpItem);
    }
    currentGrp = model->item(0)->index();
    currentPrg = model->item(0)->child(0)->child(0)->index();

    model->item(0)->setBackground(Qt::green);
    model->item(0)->child(0)->child(0)->setBackground(Qt::green);


    emit GroupChanged( currentGrp );
    emit ProgChanged( currentPrg );

    emit GroupAutosaveChanged(groupAutosaveState);
    emit ProgAutosaveChanged(progAutosaveState);

    projectDirty=false;
}

int Programs::GetCurrentMidiGroup() const
{
    if(!currentGrp.isValid())
        return 0;
    return currentGrp.row();
}

int Programs::GetCurrentMidiProg() const
{
    if(!currentPrg.isValid())
        return 0;
    return currentPrg.row();
}

bool Programs::userWantsToUnloadGroup()
{
    if(mainWindow->openedPrompt)
        return false;

    if(groupAutosaveState == Autosave::discard)
        return true;

    if(!myHost->groupContainer->IsDirty())
        return true;

    if(groupAutosaveState == Autosave::save) {
        myHost->groupContainer->SaveProgram();
        projectDirty=true;
        return true;
    }

    //prompt
    QMessageBox msgBox;
    mainWindow->openedPrompt=true;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("The group has been modified."));
    msgBox.setInformativeText(tr("Do you want to save your changes?"));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);

    int res = msgBox.exec();
    mainWindow->openedPrompt=false;

    switch(res) {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Save:
            myHost->groupContainer->SaveProgram();
            projectDirty=true;
            return true;
    }

    return true;
}

bool Programs::userWantsToUnloadProgram()
{
    if(mainWindow->openedPrompt)
        return false;

    if(progAutosaveState == Autosave::discard)
        return true;

    if(!myHost->programContainer->IsDirty())
        return true;

    if(progAutosaveState == Autosave::save) {
        myHost->programContainer->SaveProgram();
        projectDirty=true;
        return true;
    }

    //prompt
    QMessageBox msgBox;
    mainWindow->openedPrompt=true;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("The progarm has been modified."));
    msgBox.setInformativeText(tr("Do you want to save your changes?"));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);

    int res = msgBox.exec();
    mainWindow->openedPrompt=false;

    switch(res) {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Save:
            myHost->programContainer->SaveProgram();
            projectDirty=true;
            return true;
    }

    return true;
}

QStandardItem *Programs::CopyProgram(QStandardItem *progOri)
{
    int oriId = progOri->data(UserRoles::value).toInt();
    emit ProgCopy(oriId,nextProgId);

    QStandardItem *prgItem = new QStandardItem(progOri->text());
    prgItem->setData(NodeType::program,UserRoles::nodeType);
    prgItem->setData(nextProgId,UserRoles::value);
    nextProgId++;
    prgItem->setDragEnabled(true);
    prgItem->setDropEnabled(false);
    prgItem->setEditable(true);

    projectDirty=true;

    return prgItem;
}

QStandardItem *Programs::CopyGroup(QStandardItem *grpOri)
{
    int oriId = grpOri->data(UserRoles::value).toInt();
    emit GroupCopy(oriId,nextGroupId);

    QStandardItem *grpItem = new QStandardItem(grpOri->text());
    grpItem->setData(NodeType::programGroup,UserRoles::nodeType);
    grpItem->setData(nextGroupId,UserRoles::value);
    nextGroupId++;
    grpItem->setDragEnabled(true);
    grpItem->setDropEnabled(false);
    grpItem->setEditable(true);

    QStandardItem *prgList = new QStandardItem();
    prgList->setDragEnabled(false);
    prgList->setDropEnabled(true);
    prgList->setEditable(false);

    QStandardItem *prgListOri = grpOri->child(0);
    for(int prg=0; prg<prgListOri->rowCount(); prg++) {
        prgList->appendRow( CopyProgram( prgListOri->child(prg) ) );
    }

    grpItem->appendRow(prgList);

    projectDirty=true;

    return grpItem;
}

bool Programs::RemoveIndex(const QModelIndex &index)
{
    if(mainWindow->openedPrompt)
        return false;

    if(index.data(UserRoles::nodeType).toInt() == NodeType::program) {

        //keep one program
        if(model->rowCount(index.parent()) == 1)
            return false;

        //move to another program
        if(index==currentPrg) {
            if(currentPrg.row()==0) {
                ChangeProg(1);
            }  else {
                ChangeProg(0);
            }
        }

        //delete program
        int prgId = index.data(UserRoles::value).toInt();
        emit ProgDelete(prgId);

        projectDirty=true;
        return true;
    }

    if(index.data(UserRoles::nodeType).toInt() == NodeType::programGroup) {

        //keep one group
        if(model->rowCount()==1)
            return false;

        //move to another group
        if(index==currentGrp) {
            if(currentGrp.row()==0) {
                ChangeGroup(1);
            }  else {
                ChangeGroup(0);
            }
        }

        //delete all programs from group
        QStandardItem *lstPrg = model->itemFromIndex(index.child(0,0));
        for(int i=0; i< lstPrg->rowCount(); i++) {
            QStandardItem *prg = lstPrg->child(i);
            int prgId = prg->data(UserRoles::value).toInt();
            emit ProgDelete(prgId);
        }

        //delete group
        int grpId = index.data(UserRoles::value).toInt();
        emit GroupDelete(grpId);

        projectDirty=true;
        return true;
    }

    return true;
}

void Programs::ChangeProg(QStandardItem *newPrg)
{
    if(!newPrg || newPrg->index()==currentPrg)
        return;

    if(!userWantsToUnloadProgram()) {
        emit ProgChanged( currentPrg );
        return;
    }

    QModelIndex newgrp = newPrg->parent()->parent()->index();
    if(newgrp!=currentGrp) {
        if(!userWantsToUnloadGroup()) {
            emit ProgChanged( currentPrg );
            return;
        }

        QStandardItem *oldGrpItem = model->itemFromIndex( currentGrp );
        if(oldGrpItem)
            oldGrpItem->setBackground(Qt::transparent);

        currentGrp=newgrp;

        QStandardItem *newGrpItem = model->itemFromIndex(currentGrp);
        if(newGrpItem) {
            newGrpItem->setBackground(Qt::green);
            for(int i=0; i<newGrpItem->child(0,0)->rowCount(); i++)
                newGrpItem->child(0,0)->child(i,0)->setBackground(Qt::transparent);
        }

        emit GroupChanged( currentGrp );
    }

    model->itemFromIndex(currentPrg)->setBackground(Qt::transparent);
    newPrg->setBackground(Qt::green);

    currentPrg = newPrg->index();
    emit ProgChanged( currentPrg );

    projectDirty=true;
}

void Programs::ChangeProg(const QModelIndex &prgIndex)
{
    QStandardItem *newPrg = model->itemFromIndex( prgIndex );
    ChangeProg(newPrg);
}

void Programs::ChangeProg(int midiProgNum) {
    if(!currentPrg.isValid())
        return;
    ChangeProg(currentPrg.parent().child(midiProgNum,0));
}

void Programs::ChangeGroup(int grpNum)
{
    QModelIndex newGrp = model->item(grpNum)->index();
    if(!newGrp.isValid())
        return;

    if(newGrp==currentGrp)
        return;

    QStandardItem *newPrg = model->itemFromIndex( newGrp.child(0,0).child(0,0) );

    if(currentPrg.isValid()) {
        int prg = currentPrg.row();
        newPrg = model->itemFromIndex( newGrp.child(0,0).child( prg,0 ) );
    }

    if(!newPrg) {
        debug("Programs::ChangeGroup prog not found")
        newPrg = model->itemFromIndex( newGrp.child(0,0).child(0,0) );
    }

    if(!newPrg) {
        debug("Programs::ChangeGroup prog 0 not found")
        return;
    }

    ChangeProg(newPrg);
}

void Programs::SetProgAutosave(const Autosave::Enum state)
{
    progAutosaveState=state;
    projectDirty=true;
}

void Programs::SetGroupAutosave(const Autosave::Enum state)
{
    groupAutosaveState=state;
    projectDirty=true;
}

QDataStream & Programs::toStream (QDataStream &out)
{
    QStandardItem *root = model->invisibleRootItem();
    out << (quint16)root->rowCount();
    for(int i=0; i<root->rowCount(); i++) {
        QStandardItem *grpItem = root->child(i);
        out << grpItem->text();
        out << (quint32)grpItem->data(UserRoles::value).toInt();

        QStandardItem *prgList = grpItem->child(0);
        out << (quint16)prgList->rowCount();

        for(int j=0; j<prgList->rowCount(); j++) {
            QStandardItem *prgItem = prgList->child(j);
            out << prgItem->text();
            out << (quint32)prgItem->data(UserRoles::value).toInt();
        }
    }

    quint16 grp = currentPrg.parent().parent().row();
    out << grp;
    quint16 prg = currentPrg.row();
    out << prg;

    out << (quint8)groupAutosaveState;
    out << (quint8)progAutosaveState;

    projectDirty=false;
    return out;
}

QDataStream & Programs::fromStream (QDataStream &in)
{
    model->clear();
    QStandardItem *root = model->invisibleRootItem();
    quint16 nbgrp;
    in >> nbgrp;
    for(unsigned int i=0; i<nbgrp; i++) {
        QStandardItem *grpItem = new QStandardItem();
        grpItem->setData(NodeType::programGroup,UserRoles::nodeType);
        grpItem->setDragEnabled(true);
        grpItem->setDropEnabled(false);
        grpItem->setEditable(true);
        QString str;
        in >> str;
        grpItem->setText(str);
        quint32 prgId;
        in >> prgId;
        grpItem->setData(prgId,UserRoles::value);

        QStandardItem *prgList = new QStandardItem();
        prgList->setDragEnabled(false);
        prgList->setDropEnabled(true);
        prgList->setEditable(false);

        quint16 nbprog;
        in >> nbprog;
        for(unsigned int j=0; j<nbprog; j++) {
            QStandardItem *prgItem = new QStandardItem();
            prgItem->setData(NodeType::program,UserRoles::nodeType);
            prgItem->setDragEnabled(true);
            prgItem->setDropEnabled(false);
            prgItem->setEditable(true);
            QString prgStr;
            in >> prgStr;
            prgItem->setText(prgStr);
            quint32 prgId;
            in >> prgId;
            prgItem->setData(prgId,UserRoles::value);

            if(prgId>=nextProgId)
                nextProgId=prgId+1;

            prgList->appendRow(prgItem);
        }
        grpItem->appendRow(prgList);
        root->appendRow(grpItem);
    }

    quint16 grp;
    in >> grp;
    quint16 prg;
    in >> prg;

    currentGrp=QModelIndex();
    currentPrg=QModelIndex();
    ChangeGroup(grp);
    ChangeProg(prg);


    in >> (quint8&)groupAutosaveState;
    emit GroupAutosaveChanged(groupAutosaveState);

    in >> (quint8&)progAutosaveState;
    emit ProgAutosaveChanged(progAutosaveState);

    projectDirty=false;
    return in;
}

QDataStream & operator<< (QDataStream& out, Programs& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream& in, Programs& value)
{
    return value.fromStream(in);
}
