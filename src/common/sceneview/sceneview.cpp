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

#include "sceneview.h"
#include "globals.h"
#include "connectables/objectfactory.h"
#include "objectview.h"
#include "connectableobjectview.h"
#include "maincontainerview.h"
#include "bridgeview.h"
#include "connectablepinview.h"
#include "minmaxpinview.h"
#include "bridgepinview.h"
#include "connectables/container.h"
#include "connectables/objectfactory.h"
#include "mainhost.h"
#include "vstpluginview.h"
#include <QSplitter>
#include "commands/comaddcable.h"
#include "commands/comdisconnectpin.h"
#include "commands/comremovepin.h"
#include "models/programsmodel.h"

using namespace View;

SceneView::SceneView(MainHost *myHost, MainWindow *mainWindow, MainGraphicsView *viewHost, MainGraphicsView *viewProject, MainGraphicsView *viewProgram, MainGraphicsView *viewGroup,QWidget *parent) :
    QAbstractItemView(parent),
    viewHost(viewHost),
    viewProject(viewProject),
    viewProgram(viewProgram),
    viewGroup(viewGroup),
    rootObjHost(0),
    rootObjProject(0),
    rootObjProgram(0),
    rootObjInsert(0),
    sceneHost(0),
    sceneProgram(0),
    sceneGroup(0),
    hostContainerView(0),
    projectContainerView(0),
    programContainerView(0),
    groupContainerView(0),
    progParking(0),
    groupParking(0),
    timerFalloff(0),
    myHost(myHost),
    mainWindow(mainWindow)
{
    setHidden(true);
    timerFalloff = new QTimer(this);
    timerFalloff->start(50);

    sceneHost = new QGraphicsScene(this);
    sceneProject = new QGraphicsScene(this);
    sceneProgram = new QGraphicsScene(this);
    sceneGroup = new QGraphicsScene(this);

    viewHost->SetViewConfig(mainWindow->viewConfig);
    viewProject->SetViewConfig(mainWindow->viewConfig);
    viewProgram->SetViewConfig(mainWindow->viewConfig);
    viewGroup->SetViewConfig(mainWindow->viewConfig);

    //we need a root object to avoid a bug when the scene is empty
    rootObjHost = new QGraphicsRectItem(0, sceneHost);
    rootObjProject = new QGraphicsRectItem(0, sceneProject);
    rootObjProgram = new QGraphicsRectItem(0, sceneProgram);
    rootObjInsert = new QGraphicsRectItem(0, sceneGroup);

    viewHost->setScene(sceneHost);
    viewProject->setScene(sceneProject);
    viewProgram->setScene(sceneProgram);
    viewGroup->setScene(sceneGroup);

    connect(myHost->programManager,SIGNAL(ProgChanged(quint32)),
            viewProgram, SLOT(SetViewProgram(quint32)));
    connect(myHost->programManager,SIGNAL(ProgDelete(quint32)),
            viewProgram, SLOT(RemoveViewProgram(quint32)));

    connect(myHost->programManager,SIGNAL(GroupChanged(quint32)),
            viewGroup, SLOT(SetViewProgram(quint32)));
    connect(myHost->programManager,SIGNAL(GroupDelete(quint32)),
            viewGroup, SLOT(RemoveViewProgram(quint32)));
}

void SceneView::SetParkings(QWidget *progPark, QWidget *groupPark)
{
    progParking = progPark;
    groupParking = groupPark;

    if(programContainerView)
        programContainerView->SetParking(progParking);
    if(groupContainerView)
        groupContainerView->SetParking(groupParking);
}

void SceneView::dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight  )
{
//    QAbstractItemView::dataChanged(topLeft, bottomRight);

//    QModelIndex tmpIndex = topLeft;
//    do {
//        ObjectInfo info = tmpIndex.data(UserRoles::objInfo).value<ObjectInfo>();

//        switch( info.nodeType ) {
//        case NodeType::object :
//        case NodeType::container :
//            {
//                ObjectView *view = static_cast<ObjectView*>(hashItems.value(tmpIndex,0));
//                if(!view) {
//                    LOG("object not found");
//                    return;
//                }
//                view->UpdateModelIndex();
//                break;
//            }
//        case NodeType::pin :
//            {
//                PinView *view = static_cast<PinView*>(hashItems.value(tmpIndex,0));
//                if(!view) {
//                    LOG("pin not found");
//                    return;
//                }
//                view->UpdateModelIndex(tmpIndex);
//                break;
//            }
//        case NodeType::cursor :
//            {
//                PinView *view = static_cast<PinView*>(hashItems.value(tmpIndex.parent(),0));
//                if(!view) {
//                    LOG("pin not found");
//                    return;
//                }
//                static_cast<MinMaxPinView*>(view)->UpdateLimitModelIndex(tmpIndex.parent());
//                break;
//            }
//        case NodeType::cable :
//            {
//                CableView *cable = static_cast<CableView*>(hashItems.value(tmpIndex,0));
//                if(!cable) {
//                    LOG("cable not found");
//                    continue;
//                }
//                cable->UpdateModelIndex(tmpIndex);
//                break;
//            }
//        default:
//            break;
//        }

//        if ( tmpIndex == bottomRight )
//            break;
//        tmpIndex = traverseTroughIndexes ( tmpIndex );
//    } while ( tmpIndex.isValid() );
}

QModelIndex SceneView::traverseTroughIndexes ( QModelIndex index ) {
    // 1. dive deep into the structure until we found the bottom (not bottomRight)
    QModelIndex childIndex = model()->index ( 0,0,index );

    if ( childIndex.isValid() )
        return childIndex;

    // 2. now traverse all elements in the lowest hierarchy
    QModelIndex tmpIndex = model()->index ( index.row() +1,0,model()->parent ( index ) );//index.sibling(index.row()+1,0);
    if ( tmpIndex.isValid() )
        return tmpIndex;

    // 3. if no more childs are found, return QModelIndex()
    return QModelIndex();
}

void SceneView::rowsAboutToBeRemoved ( const QModelIndex & parent, int start, int end  )
{
    for ( int i = start; i <= end; ++i ) {
        const QPersistentModelIndex index = parent.child(i,0);

        ObjectInfo info = index.data(UserRoles::objInfo).value<ObjectInfo>();
        switch(info.nodeType) {
        case NodeType::object :
            {
                ObjectView *obj = static_cast<ObjectView*>(hashItems.value(index,0));
                if(!obj) {
                    LOG("obj not found");
                    continue;
                }
                obj->hide();
                obj->scene()->removeItem(obj);
                obj->deleteLater();
                break;
            }
        case NodeType::container :
            {
                QGraphicsWidget *obj = static_cast<QGraphicsWidget*>( hashItems.value(index,0) );
                if(!obj) {
                    LOG("container not found");
                    continue;
                }
                obj->hide();
                obj->scene()->removeItem(obj);
                obj->deleteLater();
                break;
            }
        case NodeType::pin :
            {
                PinView *pin = static_cast<PinView*>(hashItems.value(index,0));
                if(!pin) {
                    LOG("pin not found");
                    continue;
                }

                ListPinsView *list = static_cast<ListPinsView*>(hashItems.value(parent,0));
                pin->hide();
                list->layout->removeItem(pin);
                pin->scene()->removeItem(pin);
                pin->deleteLater();

                ObjectView *obj = static_cast<ObjectView*>( hashItems.value(index.parent().parent(),0) );
                if(obj) {
                    obj->Shrink();
                }

                mapConnectionInfo.remove(pin->GetConnectionInfo());
                break;
            }
        case NodeType::cable :
            {
                ConnectionInfo infoOut = index.data(UserRoles::value).value<ConnectionInfo>();
                ConnectionInfo infoIn = index.data(UserRoles::connectionInfo).value<ConnectionInfo>();

                QPersistentModelIndex ixOut =  mapConnectionInfo.value(infoOut);
                QPersistentModelIndex ixIn =  mapConnectionInfo.value(infoIn);
                PinView* pinOut = static_cast<PinView*>(hashItems.value( ixOut,0 ));
                PinView* pinIn = static_cast<PinView*>(hashItems.value( ixIn,0 ));

                CableView *cable = static_cast<CableView*>(hashItems.value(index,0));
                if(!cable) {
                    LOG("cable not found");
                    continue;
                }

                if(pinOut)
                    pinOut->RemoveCable(cable);
                if(pinIn)
                    pinIn->RemoveCable(cable);

                cable->hide();
                cable->scene()->removeItem(cable);
                cable->deleteLater();
                break;
            }
        default:
            break;
        }
        hashItems.remove(index);
    }
    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}

void SceneView::AddObject(const MsgObject &msg)
{
    ObjectView *objView = 0;

    //update existing object
    if(msg.prop["actionType"]=="update") {
        if(!mainWindow->listObj.contains(msg.objIndex)) {
//            LOG("obj not found"<<msg.objIndex)
            return;
        }
        mainWindow->listObj.value(msg.objIndex)->ReceiveMsg(msg);
        return;
    }

    if(msg.prop["actionType"]=="remove") {
        if(!mainWindow->listObj.contains(msg.objIndex)) {
//            LOG("obj not found"<<msg.objIndex)
            return;
        }
        MsgHandler *obj =  mainWindow->listObj.take(msg.objIndex);
        delete obj;
        return;
    }

    if(msg.prop["actionType"]=="tempObj") {
        MainContainerView *parentView = static_cast<MainContainerView*>(mainWindow->listObj.value(msg.parentIndex,0));
        if(!parentView) {
            LOG("object parent not found");
            return;
        }

        objView = new ConnectableObjectView(myHost, mainWindow, msg.objIndex, parentView);
        objView->ReceiveMsg(msg);

        QPointF pos = parentView->GetDropPos();
        objView->setPos(pos);

        //when adding item, the scene set focus to the last item
        if(objView->scene()->focusItem())
            objView->scene()->focusItem()->clearFocus();

        return;
    }

    //create new object
    if(msg.prop["actionType"]=="add") {

        switch(msg.prop.value("nodeType",0).toInt()) {
            case NodeType::container :
            {
                if(msg.objIndex == FixedObjId::hostContainer) {
                    if(hostContainerView)
                        delete hostContainerView;
                    hostContainerView = new MainContainerView(myHost, mainWindow, msg.objIndex);
                    objView=hostContainerView;
                    hostContainerView->setParentItem(rootObjHost);
                    connect(viewHost,SIGNAL(viewResized(QRectF)),
                            hostContainerView,SLOT(OnViewChanged(QRectF)));
                    QTimer::singleShot(0, viewHost, SLOT(ForceResize()));
                }

                if(msg.objIndex == FixedObjId::projectContainer) {
                    if(projectContainerView)
                        delete projectContainerView;
                    projectContainerView = new MainContainerView(myHost, mainWindow, msg.objIndex);
                    objView=projectContainerView;
                    projectContainerView->setParentItem(rootObjProject);
                    connect(viewProject,SIGNAL(viewResized(QRectF)),
                            projectContainerView,SLOT(OnViewChanged(QRectF)));
                    QTimer::singleShot(0, viewProject, SLOT(ForceResize()));
                }

                if(msg.objIndex == FixedObjId::programContainer) {
                    if(programContainerView)
                        delete programContainerView;
                    programContainerView = new MainContainerView(myHost, mainWindow, msg.objIndex);
                    objView=programContainerView;
                    programContainerView->setParentItem(rootObjProgram);
                    connect(viewProgram,SIGNAL(viewResized(QRectF)),
                            programContainerView,SLOT(OnViewChanged(QRectF)));
                    QTimer::singleShot(0, viewProgram, SLOT(ForceResize()));
                    programContainerView->SetParking(progParking);
                }

                if(msg.objIndex == FixedObjId::groupContainer) {
                    if(groupContainerView)
                        delete groupContainerView;
                    groupContainerView = new MainContainerView(myHost, mainWindow, msg.objIndex);
                    objView=groupContainerView;
                    groupContainerView->setParentItem(rootObjInsert);
                    connect(viewGroup,SIGNAL(viewResized(QRectF)),
                            groupContainerView,SLOT(OnViewChanged(QRectF)));
                    QTimer::singleShot(0, viewGroup, SLOT(ForceResize()));
                    groupContainerView->SetParking(groupParking);
                }

                objView->ReceiveMsg(msg);
                break;
            }
            case NodeType::bridge :
            {

                ObjectView *parentView = static_cast<ObjectView*>(mainWindow->listObj.value(msg.parentIndex,0));
                if(!parentView) {
                    LOG("parent not found");
                    return;
                }

                MainContainerView *containerView = static_cast<MainContainerView*>(parentView);
                switch(msg.prop.value("objType",0).toInt()) {
                    case ObjType::BridgeIn :
                        objView = containerView->bridgeIn;
                        break;
                    case ObjType::BridgeOut :
                        objView = containerView->bridgeOut;
                        break;
                    case ObjType::BridgeSend :
                        objView = containerView->bridgeSend;
                        break;
                    case ObjType::BridgeReturn :
                        objView = containerView->bridgeReturn;
                        break;
                    default:
                        LOG("unknown listpin");
                        return;
                }

                objView->SetIndex(msg.objIndex);
                objView->ReceiveMsg(msg);
                break;
            }
            case NodeType::object :
            {
                MainContainerView *parentView = static_cast<MainContainerView*>(mainWindow->listObj.value(msg.parentIndex,0));
                if(!parentView) {
                    LOG("object parent not found");
                    return;
                }
                QPointF pos = parentView->GetDropPos();

                //a temp obj exists
                if(mainWindow->listObj.contains(msg.objIndex)) {
                    ConnectableObjectView *obj = static_cast<ConnectableObjectView*>( mainWindow->listObj.value(msg.objIndex) );
                    pos=obj->pos();
                    delete obj;
                }

                if(msg.prop.value("objType",0).toInt() == ObjType::VstPlugin) {
                    objView = new VstPluginView(myHost, mainWindow, msg.objIndex, parentView);
                } else {
                    objView = new ConnectableObjectView(myHost, mainWindow, msg.objIndex, parentView);
                }
                objView->ReceiveMsg(msg);


                objView->setPos(pos);
                //model()->setData(index,pos,UserRoles::position);

                //when adding item, the scene set focus to the last item
                if(objView->scene()->focusItem())
                    objView->scene()->focusItem()->clearFocus();


                break;
            }
            case NodeType::listPin :
            {
                ObjectView *parentView = static_cast<ObjectView*>(mainWindow->listObj.value(msg.parentIndex,0));
                if(!parentView) {
                    LOG("listAudioIn parent not found"<<msg.prop);
                    return;
                }

                ListPinsView *v=0;

                switch(msg.prop.value("objType",0).toInt()) {
                    case ObjType::listAudioIn :
                        v = parentView->listAudioIn;
                        break;
                    case ObjType::listAudioOut :
                        v = parentView->listAudioOut;
                        break;
                    case ObjType::listMidiIn :
                        v = parentView->listMidiIn;
                        break;
                    case ObjType::listMidiOut :
                        v = parentView->listMidiOut;
                        break;
                    case ObjType::listParamIn :
                        v = parentView->listParametersIn;
                        break;
                    case ObjType::listParamOut :
                        v = parentView->listParametersOut;
                        break;
                    case ObjType::listBridgeIn :
                    case ObjType::listBridgeOut :
                        v = parentView->listBridge;
                        break;
                    default:
                        LOG("unknown pin type");
                        return;
                }

                if(!v) {
                    //debug("SceneView::rowsInserted listpin not found")
                    return;
                }
                v->SetIndex(msg.objIndex);
                break;
            }
            case NodeType::pin :
            {
                ListPinsView *parentList = static_cast<ListPinsView*>(mainWindow->listObj.value(msg.parentIndex,0));
                if(!parentList) {
//                    LOG("NodeType::pin list not found"<<msg.parentIndex<<msg.prop);
                    return;
                }
                ConnectionInfo pinInfo = msg.prop.value("connectionInfo").value<ConnectionInfo>();

                PinView *pinView;
                float angle=.0f;

                if(msg.prop.value("parentNodeType",-1).toInt() == NodeType::bridge) {
                    ObjType::Enum t = (ObjType::Enum)msg.prop.value("parentObjType",-1).toInt();
                    if(t==ObjType::BridgeIn || t==ObjType::BridgeOut)
                        angle=1.570796f; //pi/2
                    if(t==ObjType::BridgeSend || t==ObjType::BridgeReturn)
                        angle=-1.570796f; //-pi/2

                    pinView = static_cast<PinView*>( new BridgePinView(angle, mainWindow, msg.objIndex, parentList, pinInfo,mainWindow->viewConfig) );
                    connect(timerFalloff,SIGNAL(timeout()),
                            pinView,SLOT(updateVu()));
                } else {
                    if(pinInfo.direction==PinDirection::Input)
                        angle=3.141592f;
                    if(pinInfo.direction==PinDirection::Output)
                        angle=.0f;


                    if(pinInfo.type==PinType::Parameter) {
                        MinMaxPinView *p = new MinMaxPinView(angle,mainWindow,msg.objIndex,parentList,pinInfo,mainWindow->viewConfig);
                        connect(timerFalloff,SIGNAL(timeout()),
                                p,SLOT(updateVu()));
                        pinView = static_cast<PinView*>(p);
                    } else {
                        ConnectablePinView *p = new ConnectablePinView(angle, mainWindow, msg.objIndex, parentList, pinInfo,mainWindow->viewConfig);
                        connect(timerFalloff,SIGNAL(timeout()),
                                p,SLOT(updateVu()));
                        pinView = static_cast<PinView*>(p);
                    }

                }
    //            mapConnectionInfo.insert(pinInfo,index);

                int pinPlace = parentList->GetPinPosition(pinInfo.pinNumber);
                parentList->layout->insertItem(pinPlace, pinView);

                parentList->layout->setAlignment(pinView,Qt::AlignTop);
//                connect(pinView, SIGNAL(ConnectPins(ConnectionInfo,ConnectionInfo)),
//                        this, SLOT(ConnectPins(ConnectionInfo,ConnectionInfo)));
//                connect(pinView,SIGNAL(RemoveCablesFromPin(ConnectionInfo)),
//                        this,SLOT(RemoveCablesFromPin(ConnectionInfo)));
//                connect(pinView,SIGNAL(RemovePin(ConnectionInfo)),
//                        this,SLOT(RemovePin(ConnectionInfo)));

                pinView->ReceiveMsg(msg);
                break;
            }
            case NodeType::cursor :
            {
                MinMaxPinView* pinView = static_cast<MinMaxPinView*>(mainWindow->listObj.value(msg.parentIndex,0));
                if(!pinView) {
//                    LOG("add pinLimit, pin not found");
                    return;
                }
                pinView->ReceiveMsg(msg);
                break;
            }
            case NodeType::cable :
            {
                MainContainerView *cnt = static_cast<MainContainerView*>(mainWindow->listObj.value(msg.parentIndex,0));
                if(!cnt) {
                    LOG("add cable, container not found");
                    return;
                }

                PinView* pinOut = static_cast<PinView*>(mainWindow->listObj.value(msg.prop["pinOut"].toInt(),0));
                PinView* pinIn = static_cast<PinView*>(mainWindow->listObj.value(msg.prop["pinIn"].toInt(),0));

                if(!pinOut || !pinIn) {
                    LOG("addcable : pin not found"<<msg.prop);
                    return;
                }
                CableView *cable = new CableView(mainWindow, msg.objIndex, pinOut, pinIn, cnt, mainWindow->viewConfig);
                cable->ReceiveMsg(msg);
//                cable->UpdateModelIndex(index);
                pinOut->AddCable(cable);
                pinIn->AddCable(cable);
                cable->ReceiveMsg(msg);
                break;
            }
            default :
            {
                LOG("unkown nodeType"<< msg.objIndex << msg.prop)
                return;
            }
        }
        return;
    }

    LOG("unhandled action"<<msg.prop);
}

void SceneView::rowsInserted ( const QModelIndex & parent, int start, int end  )
{
//    if(parent.row()==-1) {
//        QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
//        return;
//    }

//    if(!parent.isValid()) {
//        LOG("parent not valid");
//        return;
//    }


//    for ( int i = start; i <= end; ++i ) {
//        QPersistentModelIndex index( parent.child(i,0) ); // model()->index( i, 0, parent );

//        ObjectView *objView = 0;
//        ObjectInfo info = index.data(UserRoles::objInfo).value<ObjectInfo>();

//        switch(info.nodeType) {
//            case NodeType::container :
//            {
//                int objId = index.data(UserRoles::value).toInt();

//                if(objId == FixedObjId::hostContainer) {
//                    hostContainerView = new MainContainerView(myHost, model());
//                    objView=hostContainerView;
//                    hostContainerView->setParentItem(rootObjHost);
//                    connect(viewHost,SIGNAL(viewResized(QRectF)),
//                            hostContainerView,SLOT(OnViewChanged(QRectF)));
//                    QTimer::singleShot(0, viewHost, SLOT(ForceResize()));
//                }

//                if(objId == FixedObjId::projectContainer) {
//                    projectContainerView = new MainContainerView(myHost, model());
//                    objView=projectContainerView;
//                    projectContainerView->setParentItem(rootObjProject);
//                    connect(viewProject,SIGNAL(viewResized(QRectF)),
//                            projectContainerView,SLOT(OnViewChanged(QRectF)));
//                    QTimer::singleShot(0, viewProject, SLOT(ForceResize()));
//                }

//                if(objId == FixedObjId::programContainer) {
//                    programContainerView = new MainContainerView(myHost, model());
//                    objView=programContainerView;
//                    programContainerView->setParentItem(rootObjProgram);
//                    connect(viewProgram,SIGNAL(viewResized(QRectF)),
//                            programContainerView,SLOT(OnViewChanged(QRectF)));
//                    QTimer::singleShot(0, viewProgram, SLOT(ForceResize()));
//                    programContainerView->SetParking(progParking);
//                }

//                if(objId == FixedObjId::groupContainer) {
//                    groupContainerView = new MainContainerView(myHost, model());
//                    objView=groupContainerView;
//                    groupContainerView->setParentItem(rootObjInsert);
//                    connect(viewGroup,SIGNAL(viewResized(QRectF)),
//                            groupContainerView,SLOT(OnViewChanged(QRectF)));
//                    QTimer::singleShot(0, viewGroup, SLOT(ForceResize()));
//                    groupContainerView->SetParking(groupParking);
//                }

//                hashItems.insert( index , objView);
//                connect(objView,SIGNAL(destroyed(QObject*)),
//                        this,SLOT(graphicObjectRemoved(QObject*)));

////                objView->SetModelIndex(index);

//                break;
//            }
//            case NodeType::bridge :
//            {

//                ObjectView *parentView = static_cast<ObjectView*>(hashItems.value(parent,0));
//                if(!parentView) {
//                    LOG("parent not found");
//                    continue;
//                }

//                switch(info.objType) {
//                    case ObjType::BridgeIn :
//                        objView = static_cast<MainContainerView*>(parentView)->bridgeIn;
//                        break;
//                    case ObjType::BridgeOut :
//                        objView = static_cast<MainContainerView*>(parentView)->bridgeOut;
//                        break;
//                    case ObjType::BridgeSend :
//                        objView = static_cast<MainContainerView*>(parentView)->bridgeSend;
//                        break;
//                    case ObjType::BridgeReturn :
//                        objView = static_cast<MainContainerView*>(parentView)->bridgeReturn;
//                        break;
//                    default:
//                        LOG("unknown listpin");
//                        continue;
//                }


//                hashItems.insert( index , objView);
//                connect(objView,SIGNAL(destroyed(QObject*)),
//                        this,SLOT(graphicObjectRemoved(QObject*)));
////                objView->SetModelIndex(index);
//                break;
//            }
//            case NodeType::object :
//            {
//                MainContainerView *parentView = static_cast<MainContainerView*>(hashItems.value(parent,0));
//                if(!parentView) {
//                    LOG("object parent not found");
//                    continue;
//                }

//                if(info.objType == ObjType::VstPlugin) {
//                    objView = new VstPluginView(myHost, model(),parentView);
//                } else {
//                    objView = new ConnectableObjectView(myHost, model(),parentView);
//                }
//                hashItems.insert( index , objView);
////                objView->SetModelIndex(index);
//                QPointF pos = parentView->GetDropPos();
//                objView->setPos(pos);
//                model()->setData(index,pos,UserRoles::position);

//                //when adding item, the scene set focus to the last item
//                if(objView->scene()->focusItem())
//                    objView->scene()->focusItem()->clearFocus();

//                connect(objView,SIGNAL(destroyed(QObject*)),
//                        this,SLOT(graphicObjectRemoved(QObject*)));
//                break;
//            }
//            case NodeType::listPin :
//            {
////                ObjectInfo infoParent = parent.data(UserRoles::objInfo).value<ObjectInfo>();
//                ObjectView *parentView = static_cast<ObjectView*>(hashItems.value(parent,0));
//                if(!parentView) {
//                    LOG("listAudioIn parent not found");
//                    continue;
//                }

//                ListPinsView *v=0;

//                switch(info.objType) {
//                    case ObjType::listAudioIn :
//                        v = parentView->listAudioIn;
//                        break;
//                    case ObjType::listAudioOut :
//                        v = parentView->listAudioOut;
//                        break;
//                    case ObjType::listMidiIn :
//                        v = parentView->listMidiIn;
//                        break;
//                    case ObjType::listMidiOut :
//                        v = parentView->listMidiOut;
//                        break;
//                    case ObjType::listParamIn :
//                        v = parentView->listParametersIn;
//                        break;
//                    case ObjType::listParamOut :
//                        v = parentView->listParametersOut;
//                        break;
//                    case ObjType::listBridgeIn :
//                    case ObjType::listBridgeOut :
//                        v = parentView->listBridge;
//                        break;
//                    default:
//                        LOG("unknown pin type");
//                        continue;
//                }

//                if(!v) {
//                    //debug("SceneView::rowsInserted listpin not found")
//                    continue;
//                }
//                hashItems.insert(index, v);
//                connect(v,SIGNAL(destroyed(QObject*)),
//                        this,SLOT(graphicObjectRemoved(QObject*)));
//                break;
//            }
//            case NodeType::pin :
//            {
//                ListPinsView *parentList = static_cast<ListPinsView*>(hashItems.value(parent,0));
//                if(!parentList) {
//                    LOG("NodeType::pin list not found");
//                    continue;
//                }
//                ConnectionInfo pinInfo = index.data(UserRoles::connectionInfo).value<ConnectionInfo>();

//                PinView *pinView;
//                ObjectInfo parentInfo = parent.parent().data(UserRoles::objInfo).value<ObjectInfo>();
//                float angle=.0f;

//                if(parentInfo.nodeType == NodeType::bridge) {
//                    if(parentInfo.objType==ObjType::BridgeIn || parentInfo.objType==ObjType::BridgeOut)
//                        angle=1.570796f; //pi/2
//                    if(parentInfo.objType==ObjType::BridgeSend || parentInfo.objType==ObjType::BridgeReturn)
//                        angle=-1.570796f; //-pi/2

//                    pinView = static_cast<PinView*>( new BridgePinView(angle, model(), parentList, pinInfo,mainWindow->viewConfig) );
//                    connect(timerFalloff,SIGNAL(timeout()),
//                            pinView,SLOT(updateVu()));
//                } else {
//                    if(pinInfo.direction==PinDirection::Input)
//                        angle=3.141592f;
//                    if(pinInfo.direction==PinDirection::Output)
//                        angle=.0f;


//                    if(pinInfo.type==PinType::Parameter) {
//                        MinMaxPinView *p = new MinMaxPinView(angle,model(),parentList,pinInfo,mainWindow->viewConfig);
//                        connect(timerFalloff,SIGNAL(timeout()),
//                                p,SLOT(updateVu()));
//                        pinView = static_cast<PinView*>(p);
//                    } else {
//                        ConnectablePinView *p = new ConnectablePinView(angle, model(), parentList, pinInfo,mainWindow->viewConfig);
//                        connect(timerFalloff,SIGNAL(timeout()),
//                                p,SLOT(updateVu()));
//                        pinView = static_cast<PinView*>(p);
//                    }

//                }
//                pinView->SetPinModelIndex(index);

//                hashItems.insert(index, pinView);
//                connect(pinView,SIGNAL(destroyed(QObject*)),
//                        this,SLOT(graphicObjectRemoved(QObject*)));
//                mapConnectionInfo.insert(pinInfo,index);

//                int pinPlace = parentList->GetPinPosition(pinInfo.pinNumber);
//                parentList->layout->insertItem(pinPlace, pinView);

//                parentList->layout->setAlignment(pinView,Qt::AlignTop);
//                connect(pinView, SIGNAL(ConnectPins(ConnectionInfo,ConnectionInfo)),
//                        this, SLOT(ConnectPins(ConnectionInfo,ConnectionInfo)));
//                connect(pinView,SIGNAL(RemoveCablesFromPin(ConnectionInfo)),
//                        this,SLOT(RemoveCablesFromPin(ConnectionInfo)));
//                connect(pinView,SIGNAL(RemovePin(ConnectionInfo)),
//                        this,SLOT(RemovePin(ConnectionInfo)));

//                pinView->UpdateModelIndex(index);
//                break;
//            }
//            case NodeType::cursor :
//            {
//                MinMaxPinView* pin = static_cast<MinMaxPinView*>(hashItems.value( index.parent(),0 ));
//                if(!pin) {
//                    LOG("add pinLimit, pin not found");
//                    continue;
//                }
//                ObjectInfo info = index.data(UserRoles::objInfo).value<ObjectInfo>();
//                pin->SetLimitModelIndex(info.objType, index);
//                break;
//            }
//            case NodeType::cable :
//            {
//                MainContainerView *cnt = static_cast<MainContainerView*>(hashItems.value(parent.parent(),0));
//                if(!cnt) {
//                    LOG("add cable, container not found");
//                    continue;
//                }
//                ConnectionInfo infoOut = index.data(UserRoles::value).value<ConnectionInfo>();
//                ConnectionInfo infoIn = index.data(UserRoles::connectionInfo).value<ConnectionInfo>();

//                if(!mapConnectionInfo.contains(infoOut) || !mapConnectionInfo.contains(infoIn)) {
//                    LOG("add cable, connectioninfo not found");
//                    continue;
//                }

//                QPersistentModelIndex ixOut =  mapConnectionInfo.value(infoOut);
//                QPersistentModelIndex ixIn =  mapConnectionInfo.value(infoIn);

//                PinView* pinOut = static_cast<PinView*>(hashItems.value( ixOut,0 ));
//                PinView* pinIn = static_cast<PinView*>(hashItems.value( ixIn,0 ));

//                if(!pinOut || !pinIn) {
//                    LOG("addcable : pin not found");
//                    continue;
//                }
//                CableView *cable = new CableView(infoOut,infoIn,cnt,mainWindow->viewConfig);
//                cable->UpdateModelIndex(index);
//                pinOut->AddCable(cable);
//                pinIn->AddCable(cable);
//                hashItems.insert(index, cable);
//                connect(cable,SIGNAL(destroyed(QObject*)),
//                        this,SLOT(graphicObjectRemoved(QObject*)));
//                break;
//            }
//            default:
//                if(index.data().toString()!="cables") {
//                    LOG("nodetype not found");
//                }
//                break;

//        }
//    }

    QAbstractItemView::rowsInserted(parent, start, end);
}

//void SceneView::graphicObjectRemoved ( QObject* obj)
//{
//    hashItems.remove( hashItems.key(obj) );
//}

//void SceneView::ConnectPins(const ConnectionInfo &pinOut, const ConnectionInfo &pinIn)
//{
//    myHost->undoStack.push( new ComAddCable(myHost,pinOut,pinIn) );
//}

//void SceneView::RemoveCablesFromPin(const ConnectionInfo &pin)
//{
//    myHost->undoStack.push( new ComDisconnectPin(myHost,pin) );
//}

//void SceneView::RemovePin(const ConnectionInfo &pin)
//{
//    myHost->undoStack.push( new ComRemovePin(myHost,pin) );
//}

void SceneView::ToggleHostView(bool show)
{
    viewHost->setVisible(show);
}

void SceneView::ToggleProjectView(bool show)
{
    viewProject->setVisible(show);
}

void SceneView::ToggleProgramView(bool show)
{
    viewProgram->setVisible(show);
}

void SceneView::ToggleInsertView(bool show)
{
    viewGroup->setVisible(show);
}
