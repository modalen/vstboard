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
#include "connectables/objects/container.h"
#include "connectables/objectfactory.h"
#include "mainhost.h"
#include "vstpluginview.h"
#include "commands/comaddcable.h"
#include "commands/comdisconnectpin.h"
#include "commands/comremovepin.h"
#include "commands/comremoveobject.h"
#include "models/programsmodel.h"
#include "models/scenemodel.h"
#include "events.h"

using namespace View;

SceneView::SceneView(MainHost *myHost, MainGraphicsView *viewHost, MainGraphicsView *viewProject, MainGraphicsView *viewProgram, MainGraphicsView *viewGroup, QWidget *parent) :
    QObject(parent),
    viewHost(viewHost),
    viewProject(viewProject),
    viewProgram(viewProgram),
    viewGroup(viewGroup),
    model(model),
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
    myHost(myHost)
{
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
    connect(myHost->programsModel,SIGNAL(ProgDelete(int)),
            viewProgram, SLOT(RemoveViewProgram(int)));

    connect(myHost->programsModel,SIGNAL(GroupChanged(QModelIndex)),
            viewGroup, SLOT(SetViewProgram(QModelIndex)));
    connect(myHost->programsModel,SIGNAL(GroupDelete(int)),
            viewGroup, SLOT(RemoveViewProgram(int)));
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


void SceneView::UpdateObj(const MetaData &info)
{
//    if( !mapMetaDatas.contains(info.ObjId()) ) {
//        AddObj(info);
//        return;
//    }

    switch( info.Type() ) {
        case MetaType::object :
        case MetaType::container :
        {
            ObjectView *view = static_cast<ObjectView*>(mapViewItems.value(info.ObjId(),0));
            if(!view) {
                LOG("object not found"<<info.toString());
                return;
            }
            view->UpdateModelIndex(info);
            break;
        }
        case MetaType::pin :
        {
            PinView *view = static_cast<PinView*>(mapViewItems.value(info.ObjId(),0));
            if(!view) {
                LOG("pin not found"<<info.toString());
                return;
            }
            view->UpdateModelIndex(info);
            break;
        }
//        case MetaTypes::cursor :
//        {
//            PinView *view = static_cast<PinView*>(mapViewItems.value(info.ParentId(),0));
//            if(!view) {
//                LOG("pin not found"<<info.toString());
//                return;
//            }
//            static_cast<MinMaxPinView*>(view)->SetLimitModelIndex(info);
//            break;
//        }
        default:
            break;
    }
}

void SceneView::DelObj(quint32 objId)
{
    if(!mapMetaDatas.contains(objId))
        return;

    const MetaData info = mapMetaDatas.value(objId);

    switch(info.Type()) {
    case MetaType::object :
        {
            ObjectView *obj = static_cast<ObjectView*>(mapViewItems.value(objId,0));
            if(!obj) {
                LOG("obj not found"<<info.toString());
                return;
            }
            obj->hide();
            obj->scene()->removeItem(obj);
            obj->deleteLater();
            break;
        }
    case MetaType::container :
        {
            QGraphicsWidget *obj = static_cast<QGraphicsWidget*>( mapViewItems.value(info.ObjId(),0) );
            if(!obj) {
                LOG("container not found"<<info.toString());
                return;
            }
            obj->hide();
            obj->scene()->removeItem(obj);
            delete obj;
            break;
        }
    case MetaType::pin :
        {
            PinView *pin = static_cast<PinView*>(mapViewItems.value(info.ObjId(),0));
            if(!pin) {
                LOG("pin not found"<<info.toString());
                return;
            }

            ListPinsView *list = static_cast<ListPinsView*>(mapViewItems.value(info.ParentId(),0));
            pin->hide();
            list->layout->removeItem(pin);
            pin->scene()->removeItem(pin);
            pin->deleteLater();

            ObjectView *obj = static_cast<ObjectView*>( mapViewItems.value(info.ParentObjectId(),0) );
            if(obj) {
                obj->Shrink();
            }

//                mapConnectionInfo.remove(pin->GetObjectInfo());
            break;
        }
    case MetaType::cable :
        {
            int pinOutId = info.GetMetaData<int>(metaT::ObjIdOut);
            int pinInid = info.GetMetaData<int>(metaT::ObjIdIn);

            PinView* pinOut = static_cast<PinView*>(mapViewItems.value( pinOutId,0 ));
            PinView* pinIn = static_cast<PinView*>(mapViewItems.value( pinInid,0 ));

            CableView *cable = static_cast<CableView*>(mapViewItems.value(info.ObjId(),0));
            if(!cable) {
                LOG("cable not found"<<info.toString());
                return;
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
        LOG("nodetype not found"<<info.toString());
        return;
    }
    mapViewItems.remove(info.ObjId());
    mapMetaDatas.remove(info.ObjId());
}

void SceneView::AddObj(MetaData &info)
{
    if( mapMetaDatas.contains(info.ObjId()) ) {
        UpdateObj(info);
        return;
    }

    ObjectView *objView = 0;

    switch(info.Type()) {
        case MetaType::container :
        {
            if(info.ObjId() == FixedObjIds::hostContainer) {
                hostContainerView = new MainContainerView(info,model);
                objView=hostContainerView;
                hostContainerView->setParentItem(rootObjHost);
                connect(viewHost,SIGNAL(viewResized(QRectF)),
                        hostContainerView,SLOT(OnViewChanged(QRectF)));
                QTimer::singleShot(0, viewHost, SLOT(ForceResize()));
            }

            else if(info.ObjId() == FixedObjIds::projectContainer) {
                projectContainerView = new MainContainerView(info,model);
                objView=projectContainerView;
                projectContainerView->setParentItem(rootObjProject);
                connect(viewProject,SIGNAL(viewResized(QRectF)),
                        projectContainerView,SLOT(OnViewChanged(QRectF)));
                QTimer::singleShot(0, viewProject, SLOT(ForceResize()));
            }

            else if(info.ObjId() == FixedObjIds::programContainer) {
                programContainerView = new MainContainerView(info,model);
                objView=programContainerView;
                programContainerView->setParentItem(rootObjProgram);
                connect(viewProgram,SIGNAL(viewResized(QRectF)),
                        programContainerView,SLOT(OnViewChanged(QRectF)));
                QTimer::singleShot(0, viewProgram, SLOT(ForceResize()));
                programContainerView->SetParking(progParking);
            }

            else if(info.ObjId() == FixedObjIds::groupContainer) {
                groupContainerView = new MainContainerView(info,model);
                objView=groupContainerView;
                groupContainerView->setParentItem(rootObjInsert);
                connect(viewGroup,SIGNAL(viewResized(QRectF)),
                        groupContainerView,SLOT(OnViewChanged(QRectF)));
                QTimer::singleShot(0, viewGroup, SLOT(ForceResize()));
                groupContainerView->SetParking(groupParking);
            }

            if(!objView) {
                return;
            }
            objView->SetConfig(myHost->mainWindow->viewConfig);
            mapViewItems.insert( info.ObjId() , objView);
            mapMetaDatas.insert( info.ObjId(), MetaData(info));
            objView->setProperty("objId",info.ObjId());
            connect(objView,SIGNAL(destroyed(QObject*)),
                    this,SLOT(graphicObjectRemoved(QObject*)));
            break;
        }
        case MetaType::bridge :
        {

            ObjectView *parentView = static_cast<ObjectView*>(mapViewItems.value(info.ParentId(),0));
            if(!parentView) {
                LOG("parent not found"<<info.toString());
                return;
            }

            switch(info.GetMetaData<Directions::Enum>(metaT::Direction)) {
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
                    LOG("unknown listpin"<<info.toString());
                    return;
            }


            mapViewItems.insert( info.ObjId() , objView);
            mapMetaDatas.insert( info.ObjId(), MetaData(info));
            objView->setProperty("objId",info.ObjId());
            connect(objView,SIGNAL(destroyed(QObject*)),
                    this,SLOT(graphicObjectRemoved(QObject*)));
            break;
        }
        case MetaType::object :
        {
            MainContainerView *containerView = static_cast<MainContainerView*>(mapViewItems.value(info.ContainerId(),0));
            if(!containerView) {
                LOG("object container not found"<<info.toString());
                return;
            }

            if(info.GetMetaData<ObjTypes::Enum>(metaT::ObjType) == ObjTypes::VstPlugin) {
                objView = new VstPluginView(info, containerView, model);
            } else {
                objView = new ConnectableObjectView(info, containerView, model);
            }
            mapViewItems.insert( info.ObjId() , objView);
            mapMetaDatas.insert( info.ObjId(), MetaData(info));
//                objView->SetModelIndex(info);
//            QPointF pos = containerView->GetDropPos();
//            objView->setPos(pos);
//            info.SetMeta(metaT::Position, pos);
//            model()->setData(index,pos,UserRoles::position);
            QPointF pos = info.GetMetaData<QPointF>(metaT::Position);
             objView->setPos(pos);

            objView->SetConfig(myHost->mainWindow->viewConfig);

            //when adding item, the scene set focus to the last item
            if(objView->scene()->focusItem())
                objView->scene()->focusItem()->clearFocus();

            objView->setProperty("objId",info.ObjId());

            connect(objView,SIGNAL(destroyed(QObject*)),
                    this,SLOT(graphicObjectRemoved(QObject*)));
            connect(objView,SIGNAL(RemoveKeepCables(MetaData)),
                    this, SLOT(RemoveObjKeepCables(MetaData)));
            connect(objView,SIGNAL(RemoveWithCables(MetaData)),
                    this, SLOT(RemoveObjWithCables(MetaData)));
            break;
        }
        case MetaType::listPin :
        {
//                ObjectInfo infoParent = parent.data(UserRoles::objInfo).value<ObjectInfo>();
            ObjectView *objView = static_cast<ObjectView*>(mapViewItems.value(info.ParentId(),0));
            if(!objView) {
                LOG("listPin parent not found" << info.toString());
                return;
            }

            ListPinsView *v=0;

            int media = info.GetMetaData<MediaTypes::Enum>(metaT::Media);
            int direction = info.GetMetaData<Directions::Enum>(metaT::Direction);

            if(media==MediaTypes::Audio) {
                if(direction==Directions::Input)
                    v = objView->listAudioIn;
                else if(direction==Directions::Output)
                    v = objView->listAudioOut;
            } else if(media==MediaTypes::Midi) {
                if(direction==Directions::Input)
                    v = objView->listMidiIn;
                else if(direction==Directions::Output)
                    v = objView->listMidiOut;
             } else if(media==MediaTypes::Parameter) {
                if(direction==Directions::Input)
                    v = objView->listParametersIn;
                else if(direction==Directions::Output)
                    v = objView->listParametersOut;
            } else if(media==MediaTypes::Bridge) {
                v = objView->listBridge;
            }

            if(!v) {
//                LOG("unknown listpin type" << info.toString());
                return;
            }

            mapViewItems.insert(info.ObjId(), v);
            mapMetaDatas.insert( info.ObjId(), MetaData(info));
            v->setProperty("objId",info.ObjId());
            connect(v,SIGNAL(destroyed(QObject*)),
                    this,SLOT(graphicObjectRemoved(QObject*)));
            break;
        }
        case MetaType::pin :
        {
            ListPinsView *parentList = static_cast<ListPinsView*>(mapViewItems.value(info.ParentId(),0));
            if(!parentList) {
                LOG("MetaTypes::pin list not found"<<info.toString());
                return;
            }

            PinView *pinView;
            float angle=.0f;
            int media = info.GetMetaData<MediaTypes::Enum>(metaT::Media);

            if(media == MediaTypes::Bridge) {
                MetaData parentInfo = mapMetaDatas.value(info.ParentObjectId());
                int direction = parentinfo.GetMetaData<Directions::Enum>(metaT::Direction);
                if(direction==Directions::Input || direction==Directions::Output)
                    angle=1.570796f; //pi/2
                if(direction==Directions::Send || direction==Directions::Return)
                    angle=-1.570796f; //-pi/2

                pinView = static_cast<PinView*>( new BridgePinView(info, angle, parentList, myHost->mainWindow->viewConfig) );

            } else {
                int direction = info.GetMetaData<Directions::Enum>(metaT::Direction);
                if(direction==Directions::Input)
                    angle=3.141592f;
                if(direction==Directions::Output)
                    angle=.0f;

                if(media==MediaTypes::Parameter) {
                    pinView = static_cast<PinView*>( new MinMaxPinView(info, angle, parentList, myHost->mainWindow->viewConfig) );
                } else {
                    pinView = static_cast<PinView*>( new ConnectablePinView(info, angle, parentList, myHost->mainWindow->viewConfig) );
                }
            }

#ifndef QT_NO_DEBUG
            pinView->setToolTip(info.toString());
#endif

            connect(timerFalloff,SIGNAL(timeout()),
                    pinView,SLOT(updateVu()));

            mapViewItems.insert(info.ObjId(), pinView);
            mapMetaDatas.insert( info.ObjId(), MetaData(info));
            pinView->setProperty("objId",info.ObjId());
            connect(pinView,SIGNAL(destroyed(QObject*)),
                    this,SLOT(graphicObjectRemoved(QObject*)));

            int pinPlace = parentList->GetPinPosition(info.GetMetaData<int>(metaT::PinNumber));
            parentList->layout->insertItem(pinPlace, pinView);

            parentList->layout->setAlignment(pinView,Qt::AlignTop);
            connect(pinView, SIGNAL(ConnectPins(MetaData,MetaData)),
                    this, SLOT(ConnectPins(MetaData,MetaData)));
            connect(pinView,SIGNAL(RemoveCablesFromPin(MetaData)),
                    this,SLOT(RemoveCablesFromPin(MetaData)));
            connect(pinView,SIGNAL(RemovePin(MetaData)),
                    this,SLOT(RemovePin(MetaData)));
            break;
        }
//        case MetaTypes::cursor :
//        {
//            MinMaxPinView* pin = static_cast<MinMaxPinView*>(mapViewItems.value( info.ParentId(),0 ));
//            if(!pin) {
//                LOG("add pinLimit, pin not found"<<info.toString());
//                return;
//            }
//            pin->SetLimitModelIndex(info);
//            break;
//        }
        case MetaType::cable :
        {
            MainContainerView *cnt = static_cast<MainContainerView*>(mapViewItems.value(info.ContainerId(),0));
            if(!cnt) {
                LOG("add cable, container not found"<<info.toString());
                return;
            }
            int pinOutId = info.GetMetaData<int>(metaT::nbOutputs);
            int pinInid = info.GetMetaData<int>(metaT::nbInputs);

            PinView* pinOut = static_cast<PinView*>(mapViewItems.value( pinOutId,0 ));
            PinView* pinIn = static_cast<PinView*>(mapViewItems.value( pinInid,0 ));

            if(!pinOut || !pinIn) {
                LOG("addcable : pin not found"<<info.toString());
                return;
            }
            CableView *cable = new CableView(pinOutId,pinInid,cnt,myHost->mainWindow->viewConfig);
            pinOut->AddCable(cable);
            pinIn->AddCable(cable);
            mapViewItems.insert(info.ObjId(), cable);
            mapMetaDatas.insert( info.ObjId(), MetaData(info));
            cable->setProperty("objId",info.ObjId());
            connect(cable,SIGNAL(destroyed(QObject*)),
                    this,SLOT(graphicObjectRemoved(QObject*)));
            break;
        }
        default:
            LOG("unknown obj type"<<info.toString());
            break;

    }
}

void SceneView::graphicObjectRemoved ( QObject* obj )
{
    int id = obj->property("objId").toInt();
    mapViewItems.remove(id);
    mapMetaDatas.remove(id);
}

void SceneView::ConnectPins(const MetaData &pinOut, const MetaData &pinIn)
{
    Events::command *e = new Events::command(new ComAddCable(myHost,pinOut,pinIn));
    model->PostEvent(e);
}

void SceneView::RemoveCablesFromPin(const MetaData &pin)
{
    Events::command *e = new Events::command(new ComDisconnectPin(myHost,pin));
    model->PostEvent(e);
}

void SceneView::RemovePin(const MetaData &pin)
{
    Events::command *e = new Events::command(new ComRemovePin(myHost,pin));
    model->PostEvent(e);
}

void SceneView::RemoveObjWithCables(const MetaData &obj)
{
    Events::command *e = new Events::command(new ComRemoveObject(myHost, obj, RemoveType::RemoveWithCables));
    model->PostEvent(e);
}

void SceneView::RemoveObjKeepCables(const MetaData &obj)
{
    Events::command *e = new Events::command( new ComRemoveObject(myHost, obj, RemoveType::BridgeCables) );
    model->PostEvent(e);
}

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
