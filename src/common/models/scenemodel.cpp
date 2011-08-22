/**************************************************************************
#    Copyright 2010-2011 Raphaël François
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

#include "scenemodel.h"
#include "globals.h"
#include "connectables/objectfactory.h"
#include "mainhost.h"
#include "models/programsmodel.h"
#include "connectables/objects/vstplugin.h"
#include "commands/comaddobject.h"
#include "events.h"
#include "mainwindow.h"
#include "sceneview/sceneview.h"
#include "meta/metaobjengine.h"

SceneModel::SceneModel(MainHost *myHost, View::SceneView *view, QObject *parent) :
    QObject(parent),
    MetaTransporter(),
    myHost(myHost),
    mySceneView(view),
    delayedAction(0),
    LoadFileMapper(0)
{
    setObjectName("HostModel");

    MetaTransporter::autoUpdate=true;

    LoadFileMapper = new QSignalMapper(this);
    delayedAction = new QTimer(this);
    delayedAction->setSingleShot(true);
    if(myHost) {
        connect(LoadFileMapper, SIGNAL(mapped(QString)), myHost, SLOT(LoadFile(QString)));
        connect(delayedAction, SIGNAL(timeout()), LoadFileMapper, SLOT(map()));
    }
}

bool SceneModel::event(QEvent *event)
{
    switch(event->type()) {
        case Events::typeNewObj : {
            Events::sendObj *e = static_cast<Events::sendObj*>(event);
            e->objInfo.SetTransporter(this);
//            LOG("add" << e->objInfo.toString());
            mySceneView->AddObj(e->objInfo);
            return true;
        }
        case Events::typeDelObj : {
            Events::delObj *e = static_cast<Events::delObj*>(event);
//            LOG("del" << e->objId);
            mySceneView->DelObj(e->objId);
            return true;
        }
        case Events::typeUpdateObj : {
            Events::sendObj *e = static_cast<Events::sendObj*>(event);
            e->objInfo.SetTransporter(this);
//            LOG("update" << e->objInfo.toString());
            mySceneView->UpdateObj(e->objInfo);
            return true;
        }
    }

    return QObject::event(event);
}

bool SceneModel::dropFile( const QString &fName, MetaObjEngine &info, MetaObjEngine &target)
{
    QFileInfo fileInfo;
    fileInfo.setFile( fName );

    if ( !fileInfo.isFile() || !fileInfo.isReadable() )
        return false;

    QString fileType(fileInfo.suffix().toLower());


#ifdef VSTSDK
    //vst plugin ?
    if ( fileType=="dll" ) {
        info.SetType(MetaType::object);
        info.SetObjType(ObjTypes::VstPlugin);
        info.SetMeta(metaT::Filename, fName);
        return true;
    }
#endif

    //fxb file
    if( fileType == VST_BANK_FILE_EXTENSION ) {
        if(target.GetMetaData<ObjTypes::Enum>(metaT::ObjType) != ObjTypes::VstPlugin) {
            LOG("fxb bad target");
            return false;
        }

//        if( senderObj->LoadBank(fName) ) {
            target.SetMeta(metaT::bankFile,fName);
            return true;
//        }
    }

    if( fileType == VST_PROGRAM_FILE_EXTENSION ) {
        if(target.GetMetaData<ObjTypes::Enum>(metaT::ObjType) != ObjTypes::VstPlugin) {
            LOG("fxp bad target");
            return false;
        }

//        if( senderObj->LoadProgram(fName) ) {
            target.SetMeta(metaT::programFile,fName);
            return true;
//        }
    }

    //setup file
    if ( fileType==SETUP_FILE_EXTENSION ) {
        if(myHost->programsModel->userWantsToUnloadSetup()) {
            //load on the next loop : we have to get out of the container before loading files
            LoadFileMapper->setMapping(delayedAction, fName);
            delayedAction->start(0);
            return true;
        }
    }

    //project file
    if ( fileType==PROJECT_FILE_EXTENSION ) {
        if(myHost->programsModel->userWantsToUnloadProject()) {
            //load on the next loop : we have to get out of the container before loading files
            LoadFileMapper->setMapping(delayedAction, fName);
            delayedAction->start(0);
            return true;
        }
    }

    return false;
}

bool SceneModel::dropMime ( const QMimeData * data, MetaObjEngine & senderInfo, QPointF &pos, InsertionType::Enum insertType )
{
    QList<MetaData>listObjInfoToAdd;


    if(data->hasFormat(MIMETYPE_METAINFO)) {
        QByteArray b = data->data(MIMETYPE_METAINFO);
        QDataStream stream(&b,QIODevice::ReadOnly);

        while(!stream.atEnd()) {
            MetaData info;
            stream >> info;
            listObjInfoToAdd << info;
        }
    }

    //drop a file
    if (data->hasUrls()) {
        foreach(QUrl url,data->urls()) {
            QString fName = url.toLocalFile();
            MetaObjEngine info;
            if(!dropFile(fName, info, senderInfo)) {
                QMessageBox msg(QMessageBox::Information,
                            tr("File not accepted"),
                            tr("The object %1 cannot load %2").arg(senderInfo.ObjName()).arg(fName)
                            );
                msg.exec();
                continue;
            }

            if(info.Type()!=MetaType::ND)
                listObjInfoToAdd << info;
        }
    }


//    //audio interface
//    if(data->hasFormat(MIMETYPE_AUDIODEVICE)) {
//        QByteArray b = data->data(MIMETYPE_AUDIODEVICE);
//        QDataStream stream(&b,QIODevice::ReadOnly);

//        while(!stream.atEnd()) {
//            MetaData info;
//            stream >> info;
//            if(info.Meta(metaT::ObjType).toInt() == ObjTypes::AudioInterface) {
//                if(info.Meta(metaT::nbInputs).toInt()!=0) {
//                    info.SetMeta(metaT::Direction,Directions::Input);
//                    listObjInfoToAdd << info;
//                 }

//                if(info.Meta(metaT::nbOutputs).toInt()!=0) {

//                    //move the second device
//                    if(info.Meta(metaT::nbInputs).toInt()!=0) {
//                        QPointF pt = info.Meta(metaT::Position).toPointF();
//                        pt.rx()+=110;
//                        info.SetMeta(metaT::Position,pt);
//                    }

//                    info.SetMeta(metaT::Direction,Directions::Output);
//                    listObjInfoToAdd << info;
//                }
//            }
//        }
//    }



    foreach(MetaObjEngine info, listObjInfoToAdd) {

        if(senderInfo.Type()==MetaType::container)
            info.SetContainerId(senderInfo.ObjId());
        else
            info.SetContainerId(senderInfo.ContainerId());

        info.SetMeta(metaT::Position,pos);
        pos+=QPointF(15,15);

        MetaData targetInfo;
        if(senderInfo.Type()==MetaType::object)
            targetInfo=senderInfo;

        ComAddObject *com = new ComAddObject(myHost, info, targetInfo, insertType);
        Connectables::VstPlugin::shellSelectView->command=com;
        //emit UndoStackPush(com);
        Events::command *e = new Events::command(com);
        PostEvent(e);
    }

    return true;
}

