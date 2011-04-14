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

#define PROJECT_FILE_KEY 0x757b0a5d

#include "projectfile.h"
#include "../mainhost.h"
#include "../mainwindow.h"
#include "fileversion.h"

void ProjectFile::Clear(MainHost *myHost)
{
    myHost->EnableSolverUpdate(false);
    myHost->SetupProjectContainer();
    myHost->SetupProgramContainer();
    myHost->SetupGroupContainer();
    myHost->EnableSolverUpdate(true);

    if(myHost->mainWindow) {
        myHost->mainWindow->mySceneView->viewProject->ClearPrograms();
        myHost->mainWindow->mySceneView->viewProgram->ClearPrograms();
        myHost->mainWindow->mySceneView->viewGroup->ClearPrograms();
    }
    myHost->programList->BuildModel();
}

bool ProjectFile::SaveToFile(MainHost *myHost,QString filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setText(tr("Unable to open %1").arg(filePath));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return false;
    }
    QDataStream out(&file);
    return ToStream(myHost,out);
}

bool ProjectFile::LoadFromFile(MainHost *myHost,QString filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox msgBox;
        msgBox.setText(tr("Unable to open %1").arg(filePath));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return false;
    }

    QDataStream in(&file);
    return FromStream(myHost,in);
}

bool ProjectFile::ToStream(MainHost *myHost,QDataStream &out)
{
    out.setVersion(QDataStream::Qt_4_7);

    myHost->currentFileVersion=PROJECT_AND_SETUP_FILE_VERSION;

    out << (quint32)PROJECT_FILE_KEY;
    out << myHost->currentFileVersion;

    myHost->EnableSolverUpdate(false);

    myHost->groupContainer->SaveProgram();
    myHost->programContainer->SaveProgram();
    myHost->projectContainer->SaveProgram();

    for(myHost->filePass=0; myHost->filePass<LOADSAVE_STAGES ; myHost->filePass++) {
        out << *myHost->projectContainer;
        out << *myHost->programContainer;
        out << *myHost->groupContainer;
    }

    out << *myHost->programList;

    if(myHost->mainWindow) {
        out << (quint8)1;
        myHost->mainWindow->mySceneView->viewProject->SaveProgram();
        myHost->mainWindow->mySceneView->viewProgram->SaveProgram();
        myHost->mainWindow->mySceneView->viewGroup->SaveProgram();

        out << *myHost->mainWindow->mySceneView->viewProject;
        out << *myHost->mainWindow->mySceneView->viewProgram;
        out << *myHost->mainWindow->mySceneView->viewGroup;
    } else {
        out << (quint8)0;
    }

    myHost->EnableSolverUpdate(true);

    return true;
}

bool ProjectFile::FromStream(MainHost *myHost,QDataStream &in)
{
    in.setVersion(QDataStream::Qt_4_7);

    quint32 magic;
    in >> magic;
    if(magic != PROJECT_FILE_KEY) {
        QMessageBox msgBox;
        //msgBox.setWindowTitle(filePath);
        msgBox.setText( tr("Unknown file format.") );
        msgBox.setInformativeText( tr("Not a VstBoard project file") );
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return false;
    }

    in >> myHost->currentFileVersion;
    if(myHost->currentFileVersion < 11) {
        QMessageBox msgBox;
//        msgBox.setWindowTitle(filePath);
        msgBox.setText( tr("Wrong file version.") );
        msgBox.setInformativeText( tr("Project file format v%1 can't be converted to the current file format v%2").arg(myHost->currentFileVersion).arg(PROJECT_AND_SETUP_FILE_VERSION) );
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return false;
    }

    myHost->EnableSolverUpdate(false);
    myHost->renderer->SetEnabled(false);

    myHost->SetupProjectContainer();
    myHost->SetupGroupContainer();
    myHost->SetupProgramContainer();

    for(myHost->filePass=0; myHost->filePass<LOADSAVE_STAGES ; myHost->filePass++) {
        in >> *myHost->projectContainer;
        in >> *myHost->programContainer;
        in >> *myHost->groupContainer;
    }

    myHost->objFactory->ResetSavedId();

    in >> *myHost->programList;

    quint8 gui;
    in >> gui;
    if(gui && myHost->mainWindow) {
        myHost->mainWindow->mySceneView->viewProject->SetProgram(EMPTY_PROGRAM);
        myHost->mainWindow->mySceneView->viewProgram->SetProgram(EMPTY_PROGRAM);
        myHost->mainWindow->mySceneView->viewGroup->SetProgram(EMPTY_PROGRAM);
        in >> *myHost->mainWindow->mySceneView->viewProject;
        myHost->mainWindow->mySceneView->viewProject->SetProgram(0);
        in >> *myHost->mainWindow->mySceneView->viewProgram;
        in >> *myHost->mainWindow->mySceneView->viewGroup;
    }

    myHost->renderer->SetEnabled(true);
    myHost->EnableSolverUpdate(true);

    return true;
}