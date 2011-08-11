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

SceneView::SceneView(MainHost *myHost,Connectables::ObjectFactory *objFactory, MainGraphicsView *viewHost, MainGraphicsView *viewProject, MainGraphicsView *viewProgram, MainGraphicsView *viewGroup,QWidget *parent) :
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
    objFactory(objFactory),
    myHost(myHost)
{
    connect(this, SIGNAL(UndoStackPush(QUndoCommand*)),
            myHost, SLOT(UndoStackPush(QUndoCommand*)));

    setHidden(true);
    timerFalloff = new QTimer(this);
    timerFalloff->start(50);

    sceneHost = new QGraphicsScene(this);
    sceneProject = new QGraphicsScene(this);
    sceneProgram = new QGraphicsScene(this);
    sceneGroup = new QGraphicsScene(this);

    //we need a root object to avoid a bug when the scene is empty
    rootObjHost = new QGraphicsRectItem(0, sceneHost);
    rootObjProject = new QGraphicsRectItem(0, sceneProject);
    rootObjProgram = new QGraphicsRectItem(0, sceneProgram);
    rootObjInsert = new QGraphicsRectItem(0, sceneGroup);

    viewHost->setScene(sceneHost);
    viewProject->setScene(sceneProject);
    viewProgram->setScene(sceneProgram);
    viewGroup->setScene(sceneGroup);

    connect(myHost->programsModel,SIGNAL(ProgChanged(QModelIndex)),
            viewProgram, SLOT(SetViewProgram(QModelIndex)));
    connect(myHost->programsModel,SIGNAL(ProgDelete(QModelIndex)),
            viewProgram, SLOT(RemoveViewProgram(QModelIndex)));

    connect(myHost->programsModel,SIGNAL(GroupChanged(QModelIndex)),
            viewGroup, SLOT(SetViewProgram(QModelIndex)));
    connect(myHost->programsModel,SIGNAL(GroupDelete(QModelIndex)),
            viewGroup, SLOT(RemoveViewProgram(QModelIndex)));
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
    QAbstractItemView::dataChanged(topLeft, bottomRight);

    QModelIndex tmpIndex = topLeft;
    do {
        if(!tmpIndex.isValid() || !tmpIndex.data(UserRoles::objInfo).isValid()) {
            LOG("invalid index");
        } else {
            ObjectInfo info = tmpIndex.data(UserRoles::objInfo).value<ObjectInfo>();

            switch( info.Meta() ) {
            case MetaTypes::object :
            case MetaTypes::container :
            {
                ObjectView *view = static_cast<ObjectView*>(hashItems.value(tmpIndex,0));
                if(!view) {
                    LOG("object not found");
                    return;
                }
                view->UpdateModelIndex();
                break;
            }
            case MetaTypes::pin :
            {
                PinView *view = static_cast<PinView*>(hashItems.value(tmpIndex,0));
                if(!view) {
                    LOG("pin not found");
                    return;
                }
                view->UpdateModelIndex(tmpIndex);
                break;
            }
                //        case MetaTypes::editor :
                //            {
                //                ObjectView *view = static_cast<ObjectView*>(hashItems.value(tmpIndex.parent(),0));
                //                if(!view) {
                //                    debug("SceneView::dataChanged editor not found")
                //                            return;
                //                }
                //                view->SetEditorIndex(tmpIndex);
                //                break;
                //            }
                //        case MetaTypes::learning :
                //            {
                //                ObjectView *view = static_cast<ObjectView*>(hashItems.value(tmpIndex.parent(),0));
                //                if(!view) {
                //                    debug("SceneView::dataChanged editor not found")
                //                            return;
                //                }
                //                view->SetLearningIndex(tmpIndex);
                //                break;
                //            }
            case MetaTypes::cursor :
            {
                PinView *view = static_cast<PinView*>(hashItems.value(tmpIndex.parent(),0));
                if(!view) {
                    LOG("pin not found");
                    return;
                }
                static_cast<MinMaxPinView*>(view)->UpdateLimitModelIndex(tmpIndex.parent());
                break;
            }
            default:
                break;
            }
        }

        if ( tmpIndex == bottomRight )
            break;
        tmpIndex = traverseTroughIndexes ( tmpIndex );
    } while ( tmpIndex.isValid() );
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
        switch(info.Meta()) {
        case MetaTypes::object :
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
        case MetaTypes::container :
            {
                QGraphicsWidget *obj = static_cast<QGraphicsWidget*>( hashItems.value(index,0) );
                if(!obj) {
                    LOG("container not found");
                    continue;
                }
                obj->hide();
                obj->scene()->removeItem(obj);
                delete obj;
                break;
            }
        case MetaTypes::pin :
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

                mapConnectionInfo.remove(pin->GetObjectInfo());
                break;
            }
        case MetaTypes::cable :
            {
                ObjectInfo infoOut = index.data(UserRoles::value).value<ObjectInfo>();
                ObjectInfo infoIn = index.data(UserRoles::connectionInfo).value<ObjectInfo>();

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
                delete cable;
                break;
            }
        default:
            LOG("nodetype not found"<<index.data().toString());
            {
                QModelIndex p = parent;
                while(p.isValid()) {
                    ObjectInfo pinfo = p.data(UserRoles::objInfo).value<ObjectInfo>();
                    LOG(p.data().toString()<<pinfo.Meta());
                    p = p.parent();
                }
            }
            break;
        }
        hashItems.remove(index);
    }
    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}

void SceneView::rowsInserted ( const QModelIndex & parent, int start, int end  )
{
    if(parent.row()==-1) {
        QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
        return;
    }

    if(!parent.isValid()) {
        LOG("parent not valid");
        return;
    }


    for ( int i = start; i <= end; ++i ) {
        QPersistentModelIndex index( parent.child(i,0) ); // model()->index( i, 0, parent );

        ObjectView *objView = 0;
        ObjectInfo info = index.data(UserRoles::objInfo).value<ObjectInfo>();

        switch(info.Meta()) {
            case MetaTypes::container :
            {
                int objId = index.data(UserRoles::value).toInt();

                if(objId == FixedObjIds::hostContainer) {
                    hostContainerView = new MainContainerView(myHost, model());
                    objView=hostContainerView;
                    hostContainerView->setParentItem(rootObjHost);
                    connect(viewHost,SIGNAL(viewResized(QRectF)),
                            hostContainerView,SLOT(OnViewChanged(QRectF)));
                    QTimer::singleShot(0, viewHost, SLOT(ForceResize()));
                }

                if(objId == FixedObjIds::projectContainer) {
                    projectContainerView = new MainContainerView(myHost, model());
                    objView=projectContainerView;
                    projectContainerView->setParentItem(rootObjProject);
                    connect(viewProject,SIGNAL(viewResized(QRectF)),
                            projectContainerView,SLOT(OnViewChanged(QRectF)));
                    QTimer::singleShot(0, viewProject, SLOT(ForceResize()));
                }

                if(objId == FixedObjIds::programContainer) {
                    programContainerView = new MainContainerView(myHost, model());
                    objView=programContainerView;
                    programContainerView->setParentItem(rootObjProgram);
                    connect(viewProgram,SIGNAL(viewResized(QRectF)),
                            programContainerView,SLOT(OnViewChanged(QRectF)));
                    QTimer::singleShot(0, viewProgram, SLOT(ForceResize()));
                    programContainerView->SetParking(progParking);
                }

                if(objId == FixedObjIds::groupContainer) {
                    groupContainerView = new MainContainerView(myHost, model());
                    objView=groupContainerView;
                    groupContainerView->setParentItem(rootObjInsert);
                    connect(viewGroup,SIGNAL(viewResized(QRectF)),
                            groupContainerView,SLOT(OnViewChanged(QRectF)));
                    QTimer::singleShot(0, viewGroup, SLOT(ForceResize()));
                    groupContainerView->SetParking(groupParking);
                }

                hashItems.insert( index , objView);
                connect(objView,SIGNAL(destroyed(QObject*)),
                        this,SLOT(graphicObjectRemoved(QObject*)));

                objView->SetModelIndex(index);

                break;
            }
            case MetaTypes::bridge :
            {

                ObjectView *parentView = static_cast<ObjectView*>(hashItems.value(parent,0));
                if(!parentView) {
                    LOG("parent not found");
                    continue;
                }

                switch(info.Meta(MetaInfos::Direction).toInt()) {
                    case Directions::Input :
                        objView = static_cast<MainContainerView*>(parentView)->bridgeIn;
                        break;
                    case Directions::Output :
                        objView = static_cast<MainContainerView*>(parentView)->bridgeOut;
                        break;
                    case Directions::Send :
                        objView = static_cast<MainContainerView*>(parentView)->bridgeSend;
                        break;
                    case Directions::Return :
                        objView = static_cast<MainContainerView*>(parentView)->bridgeReturn;
                        break;
                    default:
                        LOG("unknown listpin");
                        continue;
                }


                hashItems.insert( index , objView);
                connect(objView,SIGNAL(destroyed(QObject*)),
                        this,SLOT(graphicObjectRemoved(QObject*)));
                objView->SetModelIndex(index);
               // viewHost->ForceResize();
                break;
            }
            case MetaTypes::object :
            {
                MainContainerView *parentView = static_cast<MainContainerView*>(hashItems.value(parent,0));
                if(!parentView) {
                    LOG("object parent not found");
                    continue;
                }

                if(info.Meta(MetaInfos::ObjType).toInt() == ObjTypes::VstPlugin) {
                    objView = new VstPluginView(myHost, model(),parentView);
                } else {
                    objView = new ConnectableObjectView(myHost, model(),parentView);
                }
                hashItems.insert( index , objView);
                objView->SetModelIndex(index);
                QPointF pos = parentView->GetDropPos();
                objView->setPos(pos);
                model()->setData(index,pos,UserRoles::position);

                //when adding item, the scene set focus to the last item
                if(objView->scene()->focusItem())
                    objView->scene()->focusItem()->clearFocus();

                connect(objView,SIGNAL(destroyed(QObject*)),
                        this,SLOT(graphicObjectRemoved(QObject*)));
                break;
            }
            case MetaTypes::listPin :
            {
//                ObjectInfo infoParent = parent.data(UserRoles::objInfo).value<ObjectInfo>();
                ObjectView *parentView = static_cast<ObjectView*>(hashItems.value(parent,0));
                if(!parentView) {
                    LOG("listAudioIn parent not found");
                    continue;
                }

                ListPinsView *v=0;

                int media = info.Meta(MetaInfos::Media).toInt();
                int direction = info.Meta(MetaInfos::Direction).toInt();
                if(media==MediaTypes::Audio) {
                    if(direction==Directions::Input)
                        v = parentView->listAudioIn;
                    else if(direction==Directions::Output)
                        v = parentView->listAudioOut;
                } else if(media==MediaTypes::Midi) {
                    if(direction==Directions::Input)
                        v = parentView->listMidiIn;
                    else if(direction==Directions::Output)
                        v = parentView->listMidiOut;
                 } else if(media==MediaTypes::Parameter) {
                    if(direction==Directions::Input)
                        v = parentView->listParametersIn;
                    else if(direction==Directions::Output)
                        v = parentView->listParametersOut;
                } else if(media==MediaTypes::Bridge) {
                    v = parentView->listBridge;
                }

                if(!v) {
                        LOG("unknown pin type");
                        continue;
                }

                hashItems.insert(index, v);
                connect(v,SIGNAL(destroyed(QObject*)),
                        this,SLOT(graphicObjectRemoved(QObject*)));
                break;
            }
            case MetaTypes::pin :
            {
                ListPinsView *parentList = static_cast<ListPinsView*>(hashItems.value(parent,0));
                if(!parentList) {
                    LOG("MetaTypes::pin list not found");
                    continue;
                }
                ObjectInfo pinInfo = index.data(UserRoles::objInfo).value<ObjectInfo>();
                Connectables::Pin *pin = objFactory->GetPin(pinInfo);
                if(!pin) {
                    LOG("MetaTypes::pin pin not found");
                    continue;
                }

                PinView *pinView;
                ObjectInfo parentInfo = parent.parent().data(UserRoles::objInfo).value<ObjectInfo>();
                float angle=.0f;

                int media = info.Meta(MetaInfos::Media).toInt();
                int direction = info.Meta(MetaInfos::Direction).toInt();

                if(media == MetaTypes::bridge) {
                    if(direction==Directions::Input || direction==Directions::Output)
                        angle=1.570796f; //pi/2
                    if(direction==Directions::Send || direction==Directions::Return)
                        angle=-1.570796f; //-pi/2

                    pinView = static_cast<PinView*>( new BridgePinView(angle, model(), parentList, pin->info(),myHost->mainWindow->viewConfig) );
                    connect(timerFalloff,SIGNAL(timeout()),
                            pinView,SLOT(updateVu()));
                } else {
                    if(direction==Directions::Input)
                        angle=3.141592f;
                    if(direction==Directions::Output)
                        angle=.0f;


                    if(media==MediaTypes::Parameter) {
                        MinMaxPinView *p = new MinMaxPinView(angle,model(),parentList,pin->info(),myHost->mainWindow->viewConfig);
                        connect(timerFalloff,SIGNAL(timeout()),
                                p,SLOT(updateVu()));
                        pinView = static_cast<PinView*>(p);
                    } else {
                        ConnectablePinView *p = new ConnectablePinView(angle, model(), parentList, pin->info(),myHost->mainWindow->viewConfig);
                        connect(timerFalloff,SIGNAL(timeout()),
                                p,SLOT(updateVu()));
                        pinView = static_cast<PinView*>(p);
                    }

                }
                pinView->SetPinModelIndex(index);

                hashItems.insert(index, pinView);
                connect(pinView,SIGNAL(destroyed(QObject*)),
                        this,SLOT(graphicObjectRemoved(QObject*)));
                mapConnectionInfo.insert(pinInfo,index);

                int pinPlace = parentList->GetPinPosition(pinInfo.Meta(MetaInfos::PinNumber).toInt());
                parentList->layout->insertItem(pinPlace, pinView);

                parentList->layout->setAlignment(pinView,Qt::AlignTop);
                connect(pinView, SIGNAL(ConnectPins(ObjectInfo,ObjectInfo)),
                        this, SLOT(ConnectPins(ObjectInfo,ObjectInfo)));
                connect(pinView,SIGNAL(RemoveCablesFromPin(ObjectInfo)),
                        this,SLOT(RemoveCablesFromPin(ObjectInfo)));
                connect(pinView,SIGNAL(RemovePin(ObjectInfo)),
                        this,SLOT(RemovePin(ObjectInfo)));

                pinView->UpdateModelIndex(index);
                break;
            }
            case MetaTypes::cursor :
            {
                MinMaxPinView* pin = static_cast<MinMaxPinView*>(hashItems.value( index.parent(),0 ));
                if(!pin) {
                    LOG("add pinLimit, pin not found");
                    continue;
                }
                pin->SetLimitModelIndex(index);
                break;
            }
            case MetaTypes::cable :
            {
                MainContainerView *cnt = static_cast<MainContainerView*>(hashItems.value(parent.parent(),0));
                if(!cnt) {
                    LOG("add cable, container not found");
                    continue;
                }
                ObjectInfo infoOut = index.data(UserRoles::value).value<ObjectInfo>();
                ObjectInfo infoIn = index.data(UserRoles::connectionInfo).value<ObjectInfo>();

                if(!mapConnectionInfo.contains(infoOut) || !mapConnectionInfo.contains(infoIn)) {
                    LOG("add cable, connectioninfo not found");
                    continue;
                }

                QPersistentModelIndex ixOut =  mapConnectionInfo.value(infoOut);
                QPersistentModelIndex ixIn =  mapConnectionInfo.value(infoIn);

                PinView* pinOut = static_cast<PinView*>(hashItems.value( ixOut,0 ));
                PinView* pinIn = static_cast<PinView*>(hashItems.value( ixIn,0 ));

                if(!pinOut || !pinIn) {
                    LOG("addcable : pin not found");
                    continue;
                }
                CableView *cable = new CableView(infoOut,infoIn,cnt,myHost->mainWindow->viewConfig);
                pinOut->AddCable(cable);
                pinIn->AddCable(cable);
                hashItems.insert(index, cable);
                connect(cable,SIGNAL(destroyed(QObject*)),
                        this,SLOT(graphicObjectRemoved(QObject*)));
                break;
            }
            default:
                if(index.data().toString()!="cables") {
                    LOG("nodetype not found");
                    QModelIndex p = parent;
                    while(p.isValid()) {
                        ObjectInfo pinfo = p.data(UserRoles::objInfo).value<ObjectInfo>();
                        LOG(p.data().toString()<<pinfo.Meta());
                        p = p.parent();
                    }
                }
                break;

        }
    }

    QAbstractItemView::rowsInserted(parent, start, end);
}

void SceneView::graphicObjectRemoved ( QObject* obj)
{
    hashItems.remove( hashItems.key(obj) );
}

void SceneView::ConnectPins(const ObjectInfo &pinOut, const ObjectInfo &pinIn)
{
//    QPersistentModelIndex ixOut = mapConnectionInfo.value(pinOut);
//    QPersistentModelIndex ixIn = mapConnectionInfo.value(pinIn);

//    //                      pin  . list   . object . container
//    QModelIndex parentOut = ixOut.parent().parent().parent();
//    QModelIndex parentIn = ixIn.parent().parent().parent();

//    if(pinOut.bridge) parentOut = parentOut.parent();
//    if(pinIn.bridge) parentIn = parentIn.parent();

//    QSharedPointer<Connectables::Object> cntPtr = objFactory->GetObjectFromId(parentOut.data(UserRoles::value).toInt());

//    if(pinOut.direction==Directions::Output) {
//        static_cast<Connectables::Container*>(cntPtr.data())->UserAddCable(pinOut,pinIn);
//    } else {
//        static_cast<Connectables::Container*>(cntPtr.data())->UserAddCable(pinIn,pinOut);
//    }

    emit UndoStackPush( new ComAddCable(myHost,pinOut,pinIn) );
}

void SceneView::RemoveCablesFromPin(const ObjectInfo &pin)
{
//    QPersistentModelIndex ix = mapConnectionInfo.value(pin);
//    QModelIndex parent = ix.parent().parent().parent();
//    if(pin.bridge) parent = parent.parent();
//    QSharedPointer<Connectables::Object> cntPtr = objFactory->GetObjectFromId(parent.data(UserRoles::value).toInt());
//    static_cast<Connectables::Container*>(cntPtr.data())->UserRemoveCableFromPin(pin);
    emit UndoStackPush( new ComDisconnectPin(myHost,pin) );
}

void SceneView::RemovePin(const ObjectInfo &pin)
{
//    QPersistentModelIndex ix = mapConnectionInfo.value(pin);
//    QSharedPointer<Connectables::Object> objPtr = objFactory->GetObjectFromId(pin.objId);
//    objPtr->UserRemovePin(pin);
    emit UndoStackPush( new ComRemovePin(myHost,pin) );
}

void SceneView::ToggleHostView(bool show)
{
    viewHost->setVisible(show);
//    emit hostShown(show);
}

void SceneView::ToggleProjectView(bool show)
{
    viewProject->setVisible(show);
//    emit projectShown(show);
}

void SceneView::ToggleProgramView(bool show)
{
    viewProgram->setVisible(show);
//    emit programShown(show);
}

void SceneView::ToggleInsertView(bool show)
{
    viewGroup->setVisible(show);
//    emit insertShown(show);
}
