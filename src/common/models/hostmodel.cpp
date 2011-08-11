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

#include "hostmodel.h"
#include "globals.h"
#include "connectables/objectfactory.h"
#include "mainhost.h"
#include "models/programsmodel.h"
#include "objectinfo.h"
#include "connectables/vstplugin.h"
#include "commands/comaddobject.h"

HostModel::HostModel(MainHost *myHost, QObject *parent) :
        QStandardItemModel(parent),
    myHost(myHost),
    delayedAction(0),
    LoadFileMapper(0)
{
    LoadFileMapper = new QSignalMapper(this);
    delayedAction = new QTimer(this);
    delayedAction->setSingleShot(true);
    if(myHost) {
        connect(LoadFileMapper, SIGNAL(mapped(QString)), myHost, SLOT(LoadFile(QString)));
        connect(delayedAction, SIGNAL(timeout()), LoadFileMapper, SLOT(map()));
        connect(this, SIGNAL(UndoStackPush(QUndoCommand*)),
                myHost, SLOT(UndoStackPush(QUndoCommand*)));
    }
}

QMimeData * HostModel::mimeData ( const QModelIndexList & indexes ) const
{


    foreach(QModelIndex index, indexes) {
        ObjectInfo info = index.data(UserRoles::objInfo).value<ObjectInfo>();
        switch(info.Meta()) {
            case MetaTypes::pin :

                if(index.data(UserRoles::connectionInfo).isValid()) {
                    ConnectionInfo connectInfo = index.data(UserRoles::connectionInfo).value<ConnectionInfo>();

                    QByteArray bytes;
                    QDataStream stream(&bytes,QIODevice::WriteOnly);
                    stream << connectInfo;

                    QMimeData *mimeData = new QMimeData;
                    mimeData->setData("application/x-pin",bytes);
                    return mimeData;
                }

            default:
                return QStandardItemModel::mimeData(indexes);

        }
    }

    return QStandardItemModel::mimeData(indexes);
}

/*!
    Drop mime data on the model
    \param column autoconnect position : 1=before,2=replace,3=after
  */
bool HostModel::dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
    if(!parent.isValid()) {
        LOG("parent not valid");
        return false;
    }

    QModelIndex senderIndex = parent.sibling(parent.row(),0);
    if(!senderIndex.isValid()) {
        LOG("senderIndex not valid");
        return false;
    }
    ObjectInfo senderInfo = senderIndex.data(UserRoles::objInfo).value<ObjectInfo>();

    QSharedPointer<Connectables::Container> targetContainer;
    switch(senderInfo.Meta()) {
        case MetaTypes::container :
            targetContainer = myHost->objFactory->GetObj(senderIndex).staticCast<Connectables::Container>();
            break;
        case MetaTypes::object :
            targetContainer = myHost->objFactory->GetObj(senderIndex.parent()).staticCast<Connectables::Container>();
            break;
    }

    if(!targetContainer) {
        LOG("container not found");
        return false;
    }

//    QList< QSharedPointer<Connectables::Object> > listObjToAdd;
    QList<ObjectInfo>listObjInfoToAdd;

    //objects from parking
    if(data->hasFormat("application/x-qstandarditemmodeldatalist")) {
        QStandardItemModel mod;
        mod.dropMimeData(data,action,0,0,QModelIndex());
        int a=mod.rowCount();
        for(int i=0;i<a;i++) {
            QStandardItem *it = mod.invisibleRootItem()->child(i);
            if(it->data(UserRoles::value).isValid()) {
                QSharedPointer<Connectables::Object> objPtr = myHost->objFactory->GetObjectFromId( it->data(UserRoles::value).toInt() );
                if(objPtr.isNull()) {
                    LOG("x-qstandarditemmodeldatalist object not found");
                    continue;
                }
//                listObjToAdd << objPtr;
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

                    ObjectInfo infoVst(MetaTypes::object);
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
                    QSharedPointer<Connectables::VstPlugin> senderObj = myHost->objFactory->GetObj(senderIndex).staticCast<Connectables::VstPlugin>();
                    if(!senderObj) {
                        LOG("fxb fxp target not found");
                        return false;
                    }

                    if( info.suffix() == VST_BANK_FILE_EXTENSION && senderObj->LoadBank(fName) ) {
                        QStandardItem *item = itemFromIndex(senderIndex);
                        if(item)
                            item->setData(fName,UserRoles::bankFile);
                        return true;
                    }


                    if( info.suffix() == VST_PROGRAM_FILE_EXTENSION && senderObj->LoadProgram(fName) ) {
                        QStandardItem *item = itemFromIndex(senderIndex);
                        if(item)
                            item->setData(fName,UserRoles::programFile);
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
            ObjectInfo info;
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
            ObjectInfo info;
            stream >> info;
            listObjInfoToAdd << info;
        }
    }

    //tools
    if(data->hasFormat("application/x-tools")) {
        QByteArray b = data->data("application/x-tools");
        QDataStream stream(&b,QIODevice::ReadOnly);

        while(!stream.atEnd()) {
            ObjectInfo info;
            stream >> info;
            listObjInfoToAdd << info;
        }
    }


    foreach(ObjectInfo info, listObjInfoToAdd) {

        QSharedPointer<Connectables::Object> senderObj = myHost->objFactory->GetObj(senderIndex);

        InsertionType::Enum insertType = InsertionType::NoInsertion;
        switch(column) {
            case 0:
                insertType = InsertionType::NoInsertion;
                break;
            case 1:
                if(action==Qt::MoveAction)
                    insertType = InsertionType::InsertBefore;
                else
                    insertType = InsertionType::AddBefore;
                break;
            case 2:
                insertType = InsertionType::Replace;
                break;
            case 3:
                if(action==Qt::MoveAction)
                    insertType = InsertionType::InsertAfter;
                else
                    insertType = InsertionType::AddAfter;
                break;
        }

        ComAddObject *com = new ComAddObject(myHost, info, targetContainer, senderObj, insertType);
        Connectables::VstPlugin::shellSelectView->command=com;
        emit UndoStackPush(com);
    }

    return true;
}

bool HostModel::setData ( const QModelIndex & index, const QVariant & value, int role )
{
    ObjectInfo info = index.data(UserRoles::objInfo).value<ObjectInfo>();
    switch(info.Meta()) {
        case MetaTypes::object :
        //case MetaTypes::container :
        {
            int objId = index.data(UserRoles::value).toInt();
            if(!objId) {
                LOG("MetaTypes::object has no object Id");
                return false;
            }
            QSharedPointer<Connectables::Object> objPtr = myHost->objFactory->GetObjectFromId(objId);
            if(objPtr.isNull()) {
                LOG("MetaTypes::object the object deleted"<<objId);
                return false;
            }

            //save vst bank file
            if(role == UserRoles::bankFile) {
                objPtr.staticCast<Connectables::VstPlugin>()->SaveBank( value.toString() );
                QStandardItem *item = itemFromIndex(index);
                if(item)
                    item->setData(value,UserRoles::bankFile);
                return true;
            }

            //save vst program file
            if(role == UserRoles::programFile) {
                objPtr.staticCast<Connectables::VstPlugin>()->SaveProgram( value.toString() );
                QStandardItem *item = itemFromIndex(index);
                if(item)
                    item->setData(value,UserRoles::programFile);
                return true;
            }
            break;
        }

        case MetaTypes::pin :
        {
            ObjectInfo pinInfo = index.data(UserRoles::objInfo).value<ObjectInfo>();
            if(pinInfo.Meta(MetaInfos::Media).toInt()==MediaTypes::Parameter) {
                if(role==UserRoles::value) {
                    bool ok=true;
                    float newVal = value.toFloat(&ok);// item->data(Qt::DisplayRole).toFloat(&ok);
                    if(!ok) {
                        LOG("pin can't convert value to float");
                        return false;
                    }

                    if(newVal>1.0f) newVal=1.0f;
                    if(newVal<.0f) newVal=.0f;
                    Connectables::ParameterPin* pin = static_cast<Connectables::ParameterPin*>(myHost->objFactory->GetPin(pinInfo));
                    if(!pin) {
                        return false;
                    }
                    pin->ChangeValue( newVal );

                    QStandardItem *item = itemFromIndex(index);
                    if(item)
                        item->setData(newVal,role);
                    return true;
                }
            }
            break;
        }
        case MetaTypes::cursor :
        {
            ObjectInfo pinInfo = index.parent().data(UserRoles::objInfo).value<ObjectInfo>();
            if(pinInfo.Meta(MetaInfos::Media).toInt()==MediaTypes::Parameter) {
                if(role==UserRoles::value) {
                    Connectables::ParameterPin* pin = static_cast<Connectables::ParameterPin*>(myHost->objFactory->GetPin(pinInfo));
                    ObjectInfo info = index.data(UserRoles::objInfo).value<ObjectInfo>();
                    Directions::Enum direction = (Directions::Enum)info.Meta(MetaInfos::Direction).toInt();
                    LimitTypes::Enum limit = (LimitTypes::Enum)info.Meta(MetaInfos::LimitType).toInt();
                    pin->SetLimit(direction,limit,value.toFloat());

                    QStandardItem *item = itemFromIndex(index);
                    if(item)
                        item->setData(value,role);
                }
            }
        }
        default:
            break;

    }

    return QStandardItemModel::setData(index,value,role);
}
