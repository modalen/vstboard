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

#include "projectfile.h"
#include "mainhost.h"
#include "mainwindow.h"
#include "models/programsmodel.h"
#include "fileversion.h"
#include "objectinfo.h"

bool ProjectFile::SaveToProjectFile(MainHost *myHost,QString filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly)) {
        myHost->mainWindow->DisplayMessage(QMessageBox::Critical,tr("Unable to open %1").arg(filePath));
        return false;
    }
    QDataStream out(&file);
    return ToStream(myHost,out,PROJECT_FILE_KEY);
}

bool ProjectFile::SaveToSetupFile(MainHost *myHost,QString filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly)) {
        myHost->mainWindow->DisplayMessage(QMessageBox::Critical,tr("Unable to open %1").arg(filePath));
        return false;
    }
    QDataStream out(&file);
    return ToStream(myHost,out,SETUP_FILE_KEY);
}

bool ProjectFile::LoadFromFile(MainHost *myHost,QString filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)) {
        myHost->mainWindow->DisplayMessage(QMessageBox::Critical,tr("Unable to open %1").arg(filePath));
        return false;
    }

    QDataStream in(&file);
    bool ret=FromStream(myHost,in);
    return ret;
}

bool ProjectFile::ToStream(MainHost *myHost,QDataStream &out, quint32 fileKey )
{
    out.setVersion(QDataStream::Qt_4_7);

    MainHost::currentFileVersion=PROJECT_AND_SETUP_FILE_VERSION;

    out << (quint32)fileKey;
    out << MainHost::currentFileVersion;

    myHost->EnableSolverUpdate(false);

    if(fileKey == SETUP_FILE_KEY || fileKey == SETUPANDPROJECT_FILE_KEY) {

        {
            //save HostContainer
            QByteArray tmpBa;
            QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);
            myHost->hostContainer->SaveProgram();
            myHost->hostContainer->toStream(tmpStream);
            SaveChunk( "HostContainer", tmpBa, out);
        }

        //save HostView
        if(myHost->mainWindow) {
            QByteArray tmpBa;
            QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);
            myHost->mainWindow->mySceneView->viewHost->SaveProgram();
            tmpStream << *myHost->mainWindow->mySceneView->viewHost;
            SaveChunk("HostView", tmpBa, out);
        }

        {
            //save Colors
            QByteArray tmpBa;
            QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);
            myHost->mainWindow->viewConfig->SaveToFile( tmpStream );
            SaveChunk("Colors", tmpBa, out);
        }
    }

    if(fileKey == PROJECT_FILE_KEY || fileKey == SETUPANDPROJECT_FILE_KEY) {

        {
            //save ProjectContainer
            QByteArray tmpBa;
            QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);
            myHost->projectContainer->SaveProgram();
            myHost->projectContainer->toStream(tmpStream);
            SaveChunk( "ProjectContainer", tmpBa, out);
        }

        {
            //save ProgramContainer
            QByteArray tmpBa;
            QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);
            myHost->programContainer->SaveProgram();
            myHost->programContainer->toStream(tmpStream);
            SaveChunk( "ProgramContainer", tmpBa, out);
        }

        {
            //save GroupContainer
            QByteArray tmpBa;
            QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);
            myHost->groupContainer->SaveProgram();
            myHost->groupContainer->toStream(tmpStream);
            SaveChunk( "GroupContainer", tmpBa, out);
        }

        {
            //save ProgramList
            QByteArray tmpBa;
            QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);
            tmpStream << *myHost->programsModel;
            SaveChunk( "ProgramList", tmpBa, out);
        }

        //save ProjectViews
        if(myHost->mainWindow) {
            myHost->mainWindow->mySceneView->viewProject->SaveProgram();
            myHost->mainWindow->mySceneView->viewProgram->SaveProgram();
            myHost->mainWindow->mySceneView->viewGroup->SaveProgram();

            QByteArray tmpBa;
            QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);
            tmpStream << *myHost->mainWindow->mySceneView->viewProject;
            tmpStream << *myHost->mainWindow->mySceneView->viewProgram;
            tmpStream << *myHost->mainWindow->mySceneView->viewGroup;
            SaveChunk("ProjectViews", tmpBa, out);
        }
    }

    myHost->EnableSolverUpdate(true);
    return true;
}

bool ProjectFile::FromStream(MainHost *myHost,QDataStream &in)
{
    in.setVersion(QDataStream::Qt_4_7);

    quint32 magic;
    in >> magic;
    if(magic != PROJECT_FILE_KEY && magic != SETUP_FILE_KEY && magic != SETUPANDPROJECT_FILE_KEY) {
        myHost->mainWindow->DisplayMessage(QMessageBox::Critical,tr("Unknown file format. This file can't be loaded"));
        return false;
    }

    in >> MainHost::currentFileVersion;
    if(MainHost::currentFileVersion != PROJECT_AND_SETUP_FILE_VERSION) {
        myHost->mainWindow->DisplayMessage(QMessageBox::Critical,tr("File format v%1 can't be converted to the current file format v%2").arg(MainHost::currentFileVersion).arg(PROJECT_AND_SETUP_FILE_VERSION));
        return false;
    }

    myHost->EnableSolverUpdate(false);
    myHost->renderer->SetEnabled(false);

    QString chunkName;
    QByteArray tmpBa;

    while( !in.atEnd() ) {
        QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);
        LoadNextChunk( chunkName, tmpBa, in );

        if(chunkName=="HostContainer") {
            myHost->SetupHostContainer();
            if(!myHost->hostContainer->fromStream( tmpStream ))
                myHost->SetupHostContainer();
        }

        else if(chunkName=="ProjectContainer") {
            myHost->SetupProjectContainer();
            if(!myHost->projectContainer->fromStream(tmpStream))
                myHost->SetupProjectContainer();
        }

        else if(chunkName=="ProgramContainer") {
            myHost->SetupProgramContainer();
            if(!myHost->programContainer->fromStream(tmpStream))
                myHost->SetupProgramContainer();
        }

        else if(chunkName=="GroupContainer") {
            myHost->SetupGroupContainer();
            if(!myHost->groupContainer->fromStream(tmpStream))
                myHost->SetupGroupContainer();
        }

        else if(chunkName=="ProgramList") {
            tmpStream >> *myHost->programsModel;
        }

        else if(chunkName=="HostView") {
            if(myHost->mainWindow) {
                tmpStream >> *myHost->mainWindow->mySceneView->viewHost;
                myHost->mainWindow->mySceneView->viewHost->SetViewProgram( myHost->hostContainer->GetProgramToSet() );
            }
        }

        else if(chunkName=="ProjectViews") {
            if(myHost->mainWindow) {
                tmpStream >> *myHost->mainWindow->mySceneView->viewProject;
                tmpStream >> *myHost->mainWindow->mySceneView->viewProgram;
                tmpStream >> *myHost->mainWindow->mySceneView->viewGroup;
                myHost->mainWindow->mySceneView->viewProject->SetViewProgram( myHost->projectContainer->GetProgramToSet() );
                myHost->mainWindow->mySceneView->viewProgram->SetViewProgram( myHost->programContainer->GetProgramToSet() );
                myHost->mainWindow->mySceneView->viewGroup->SetViewProgram( myHost->groupContainer->GetProgramToSet() );
            }
        }

        else if(chunkName=="Colors") {
            myHost->mainWindow->viewConfig->LoadFromFile( tmpStream );
        }

        //unknown chunk
        else {
            in.setStatus(QDataStream::ReadCorruptData);
            LOG("unknown file section"<<chunkName);
        }

        if(!tmpStream.atEnd()) {
            in.setStatus(QDataStream::ReadCorruptData);
#ifndef QT_NO_DEBUG
            LOG("stream not at end, drop remaining data");
            while(!tmpStream.atEnd()) {
                char c[1000];
                int nb=tmpStream.readRawData(c,1000);
                LOG(nb << QByteArray::fromRawData(c,nb).toHex());
            }
#endif
        }

        if(tmpStream.status()==QDataStream::ReadCorruptData) {
            in.setStatus(QDataStream::ReadCorruptData);
        }

        if(tmpStream.status()==QDataStream::ReadPastEnd) {
            LOG("err"<<tmpStream.status());
            myHost->renderer->SetEnabled(true);
            myHost->EnableSolverUpdate(true);
            myHost->mainWindow->DisplayMessage(QMessageBox::Critical,tr("The file is corrupted and cannot be loaded"));
            return false;
        }
    }

    myHost->renderer->SetEnabled(true);
    myHost->EnableSolverUpdate(true);

    if(in.status()!=QDataStream::Ok)
        myHost->mainWindow->DisplayMessage(QMessageBox::Warning,tr("The file is corrupted and cannot be fully loaded"));

    return true;
}

bool ProjectFile::SaveChunk(const QString &chunkName, QByteArray &ba, QDataStream &out)
{
    out << chunkName;
    out << ba;
    return true;
}

bool ProjectFile::LoadNextChunk( QString &chunkName, QByteArray &ba, QDataStream &in)
{
    if(in.atEnd())
        return false;

    in >> chunkName;
    in >> ba;
    return true;
}
