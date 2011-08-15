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

#include "hostmodel.h"
#include "globals.h"
#include "connectables/objectfactory.h"
#include "mainhost.h"
#include "models/programsmodel.h"
#include "connectables/objects/vstplugin.h"
#include "commands/comaddobject.h"
#include "objectinfo.h"
#include "events.h"
#include "mainwindow.h"

HostModel::HostModel(MainHost *myHost, QObject *parent) :
    QObject(parent),
    myHost(myHost),
    delayedAction(0),
    LoadFileMapper(0)
{
    setObjectName("MainHostModel");

    LoadFileMapper = new QSignalMapper(this);
    delayedAction = new QTimer(this);
    delayedAction->setSingleShot(true);
    if(myHost) {
        connect(LoadFileMapper, SIGNAL(mapped(QString)), myHost, SLOT(LoadFile(QString)));
        connect(delayedAction, SIGNAL(timeout()), LoadFileMapper, SLOT(map()));
    }
}

void HostModel::valueChanged( const MetaInfo & senderInfo, int info, const QVariant &value)
{
    Events::valChanged *e = new Events::valChanged(MetaInfo(senderInfo), (MetaInfos::Enum)info, value);
    myHost->mainWindow->PostEvent(e);
}

bool HostModel::dropMime ( const QMimeData * data, MetaInfo & senderInfo, InsertionType::Enum insertType )
{
    QList<MetaInfo>listObjInfoToAdd;

    //objects from parking
    if(data->hasFormat("application/x-qstandarditemmodeldatalist")) {
        QStandardItemModel mod;
        mod.dropMimeData(data,Qt::MoveAction,0,0,QModelIndex());
        int a=mod.rowCount();
        for(int i=0;i<a;i++) {
            QStandardItem *it = mod.invisibleRootItem()->child(i);
            if(it->data(UserRoles::value).isValid()) {
                QSharedPointer<Connectables::Object> objPtr = myHost->objFactory->GetObjectFromId( it->data(UserRoles::value).toInt() );
                if(objPtr.isNull()) {
                    LOG("x-qstandarditemmodeldatalist object not found");
                    continue;
                }
                listObjInfoToAdd << objPtr->info();
            }
        }
    }


    //drop a file
    if (data->hasUrls()) {
        foreach(QUrl url,data->urls()) {
            QString fName = url.toLocalFile();
            QFileInfo info;
            info.setFile( fName );

            if ( info.isFile() && info.isReadable() ) {
#ifdef VSTSDK
                //vst plugin
                if ( info.suffix()=="dll" ) {

                    MetaInfo infoVst(MetaTypes::object);
                    infoVst.SetMeta(MetaInfos::ObjType, ObjTypes::VstPlugin);
                    infoVst.SetMeta(MetaInfos::Filename, fName);
                    listObjInfoToAdd << infoVst;
                }
#endif
                //setup file
                if ( info.suffix()==SETUP_FILE_EXTENSION ) {
                    if(myHost->programsModel->userWantsToUnloadSetup()) {
                        //load on the next loop : we have to get out of the container before loading files
                        LoadFileMapper->setMapping(delayedAction, fName);
                        delayedAction->start(0);
                        return true;
                    }
                }

                //project file
                if ( info.suffix()==PROJECT_FILE_EXTENSION ) {
                    if(myHost->programsModel->userWantsToUnloadProject()) {
                        //load on the next loop : we have to get out of the container before loading files
                        LoadFileMapper->setMapping(delayedAction, fName);
                        delayedAction->start(0);
                        return true;
                    }
                }

                //fxb file
                if( info.suffix() == VST_BANK_FILE_EXTENSION || info.suffix() == VST_PROGRAM_FILE_EXTENSION) {
                    QSharedPointer<Connectables::VstPlugin> senderObj = myHost->objFactory->GetObjectFromId(senderInfo.ObjId()).staticCast<Connectables::VstPlugin>();
                    if(!senderObj) {
                        LOG("fxb fxp target not found");
                        return false;
                    }

                    if( info.suffix() == VST_BANK_FILE_EXTENSION && senderObj->LoadBank(fName) ) {
                        senderInfo.SetMeta(MetaInfos::bankFile,fName);
                        return true;
                    }


                    if( info.suffix() == VST_PROGRAM_FILE_EXTENSION && senderObj->LoadProgram(fName) ) {
                        senderInfo.SetMeta(MetaInfos::programFile,fName);
                        return true;
                    }
                }
            }
        }
    }


    //audio interface
    if(data->hasFormat("application/x-audiointerface")) {
        QByteArray b = data->data("application/x-audiointerface");
        QDataStream stream(&b,QIODevice::ReadOnly);

        while(!stream.atEnd()) {
            MetaInfo info;
            stream >> info;

            if(info.Meta(MetaInfos::nbInputs).toInt()!=0) {
                info.SetMeta(MetaInfos::Direction,Directions::Input);
                listObjInfoToAdd << info;
            }

            if(info.Meta(MetaInfos::nbOutputs).toInt()!=0) {
                info.SetMeta(MetaInfos::Direction,Directions::Output);
                listObjInfoToAdd << info;
            }
        }
    }

    //midi interface
    if(data->hasFormat("application/x-midiinterface")) {
        QByteArray b = data->data("application/x-midiinterface");
        QDataStream stream(&b,QIODevice::ReadOnly);

        while(!stream.atEnd()) {
            MetaInfo info;
            stream >> info;
            listObjInfoToAdd << info;
        }
    }

    //tools
    if(data->hasFormat("application/x-tools")) {
        QByteArray b = data->data("application/x-tools");
        QDataStream stream(&b,QIODevice::ReadOnly);

        while(!stream.atEnd()) {
            MetaInfo info;
            stream >> info;
            listObjInfoToAdd << info;
        }
    }


    foreach(MetaInfo info, listObjInfoToAdd) {

        if(senderInfo.Type()==MetaTypes::container)
            info.SetContainerId(senderInfo.ObjId());
        else
            info.SetContainerId(senderInfo.ContainerId());

        if(senderInfo.Type()!=MetaTypes::object)
            senderInfo=MetaInfo();

        ComAddObject *com = new ComAddObject(myHost, info, senderInfo, insertType);
        Connectables::VstPlugin::shellSelectView->command=com;
        //emit UndoStackPush(com);
        Events::command *e = new Events::command(com);
        myHost->mainWindow->PostEvent(e);
    }

    return true;
}

//bool HostModel::setData ( const QModelIndex & index, const QVariant & value, int role )
//{
//    MetaInfo info = index.data(UserRoles::objInfo).value<MetaInfo>();
//    switch(info.Type()) {
//        case MetaTypes::object :
//        //case MetaTypes::container :
//        {
//            int objId = index.data(UserRoles::value).toInt();
//            if(!objId) {
//                LOG("MetaTypes::object has no object Id");
//                return false;
//            }
//            QSharedPointer<Connectables::Object> objPtr = myHost->objFactory->GetObjectFromId(objId);
//            if(objPtr.isNull()) {
//                LOG("MetaTypes::object the object deleted"<<objId);
//                return false;
//            }

//            //save vst bank file
//            if(role == UserRoles::bankFile) {
//                objPtr.staticCast<Connectables::VstPlugin>()->SaveBank( value.toString() );
//                QStandardItem *item = itemFromIndex(index);
//                if(item)
//                    item->setData(value,UserRoles::bankFile);
//                return true;
//            }

//            //save vst program file
//            if(role == UserRoles::programFile) {
//                objPtr.staticCast<Connectables::VstPlugin>()->SaveProgram( value.toString() );
//                QStandardItem *item = itemFromIndex(index);
//                if(item)
//                    item->setData(value,UserRoles::programFile);
//                return true;
//            }
//            break;
//        }

//        case MetaTypes::pin :
//        {
//            MetaInfo pinInfo = index.data(UserRoles::objInfo).value<MetaInfo>();
//            if(pinInfo.Meta(MetaInfos::Media).toInt()==MediaTypes::Parameter) {
//                if(role==UserRoles::value) {
//                    bool ok=true;
//                    float newVal = value.toFloat(&ok);// item->data(Qt::DisplayRole).toFloat(&ok);
//                    if(!ok) {
//                        LOG("pin can't convert value to float");
//                        return false;
//                    }

//                    if(newVal>1.0f) newVal=1.0f;
//                    if(newVal<.0f) newVal=.0f;
//                    Connectables::ParameterPin* pin = static_cast<Connectables::ParameterPin*>(myHost->objFactory->GetPin(pinInfo));
//                    if(!pin) {
//                        return false;
//                    }
//                    pin->ChangeValue( newVal );

//                    QStandardItem *item = itemFromIndex(index);
//                    if(item)
//                        item->setData(newVal,role);
//                    return true;
//                }
//            }
//            break;
//        }
//        case MetaTypes::cursor :
//        {
//            MetaInfo pinInfo = index.parent().data(UserRoles::objInfo).value<MetaInfo>();
//            if(pinInfo.Meta(MetaInfos::Media).toInt()==MediaTypes::Parameter) {
//                if(role==UserRoles::value) {
//                    Connectables::ParameterPin* pin = static_cast<Connectables::ParameterPin*>(myHost->objFactory->GetPin(pinInfo));
//                    MetaInfo info = index.data(UserRoles::objInfo).value<MetaInfo>();
//                    Directions::Enum direction = (Directions::Enum)info.Meta(MetaInfos::Direction).toInt();
//                    LimitTypes::Enum limit = (LimitTypes::Enum)info.Meta(MetaInfos::LimitType).toInt();
//                    pin->SetLimit(direction,limit,value.toFloat());

//                    QStandardItem *item = itemFromIndex(index);
//                    if(item)
//                        item->setData(value,role);
//                }
//            }
//        }
//        default:
//            break;

//    }
//}
