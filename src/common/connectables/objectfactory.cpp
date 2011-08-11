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


#include "objectfactory.h"
#include "midisender.h"
#include "miditoautomation.h"
#include "hostcontroller.h"
#include "container.h"
#include "bridge.h"
#include "mainhost.h"

#ifdef SCRIPTENGINE
    #include "script.h"
#endif

#ifdef VSTSDK
    #include "vstplugin.h"
    #include "../vst/cvsthost.h"
#endif

/*!
  \namespace Connectables
  \brief classes used by the engine
*/
using namespace Connectables;

ObjectFactory::ObjectFactory(MainHost *myHost) :
    QObject(myHost),
    cptListObjects(50),
    myHost(myHost)
{
    setObjectName("ObjectFactory");

#ifdef SCRIPTENGINE
    QScriptValue scriptObj = myHost->scriptEngine->newQObject(this);
    myHost->scriptEngine->globalObject().setProperty("ObjectFactory", scriptObj);
#endif
}

ObjectFactory::~ObjectFactory()
{
    listObjects.clear();
}

void ObjectFactory::ResetSavedId()
{
    hashObjects::iterator i = listObjects.begin();
    while(i != listObjects.end()) {
        QSharedPointer<Object> objPtr = i.value().toStrongRef();
        if(objPtr.isNull()) {
            i=listObjects.erase(i);
        } else {
            //don't reset forcced ids
            if(i.key()>=50) {
                objPtr->ResetSavedIndex();
            }
            ++i;
        }
    }
}

int ObjectFactory::IdFromSavedId(int savedId)
{
    hashObjects::const_iterator i = listObjects.constBegin();
    while(i != listObjects.constEnd()) {
        QSharedPointer<Object> objPtr = i.value().toStrongRef();
        if(objPtr && objPtr->GetSavedIndex()==savedId) {
            return i.key();
        }
        ++i;
    }
    LOG("id not found"<<savedId);
    return -1;
}

Pin *ObjectFactory::GetPin(const ObjectInfo &pinInfo)
{
    if(!listObjects.contains(pinInfo.ObjId())) {
        LOG("obj not found"<<pinInfo.ObjId());
        return 0;
    }

    QSharedPointer<Object> objPtr = listObjects.value(pinInfo.ObjId()).toStrongRef();
    if(objPtr)
        return objPtr->GetPin(pinInfo);

    return 0;
}

QSharedPointer<Object> ObjectFactory::GetObj(const QModelIndex & index)
{
    //the object is not created, do it
    if(!index.data(UserRoles::value).isValid()) {
        return NewObject( index.data(UserRoles::objInfo).value<ObjectInfo>() );
    }

    //or return the existing object
    return GetObjectFromId(index.data(UserRoles::value).toInt());
}

QSharedPointer<Object> ObjectFactory::NewObject( ObjectInfo &info)
{
    int forcedObjId = 0;//cptListObjects;
    if(info.ObjId()!=0) {
        forcedObjId = info.ObjId();
        if(listObjects.contains(forcedObjId)) {
            LOG("forcedId already exists"<<forcedObjId);
        }
    } else {
        info.SetObjId( GetNewId() );
    }

    Object *obj=0;

    obj=CreateOtherObjects(info);

    if(!obj) {
        switch(info.Meta()) {

            case MetaTypes::container :
                obj = new Container(myHost, info);
                break;

            case MetaTypes::bridge :
                obj = new Bridge(myHost, info);
                break;

            case MetaTypes::object :

                switch(info.Meta(MetaInfos::ObjType).toInt()) {
#ifdef SCRIPTENGINE
                    case ObjTypes::Script:
                        obj = new Script(myHost, info);
                        break;
#endif
                    case ObjTypes::MidiSender:
                        obj = new MidiSender(myHost, info);
                        break;

                    case ObjTypes::MidiToAutomation:
                        obj = new MidiToAutomation(myHost, info);
                        break;

                    case ObjTypes::HostController:
                        obj = new HostController(myHost, info);
                        break;

            #ifdef VSTSDK
                    case ObjTypes::VstPlugin:
                        obj = new VstPlugin(myHost, info);
                        break;
            #endif

                    case ObjTypes::Dummy :
                        info.SetMeta(MetaInfos::errorMessage,"Dummy object");
                        obj = new Object(myHost, info);
                        break;

                    default:
                        LOG("unknown object type");
                        return QSharedPointer<Object>();
                }
                break;


            default :
                LOG("unknown nodeType"<<info.Meta());
                return QSharedPointer<Object>();
        }
    }


    QSharedPointer<Object> sharedObj(obj);
    listObjects.insert(info.ObjId(),sharedObj.toWeakRef());

    if(!obj->Open()) {
        listObjects.remove(info.ObjId());
        sharedObj.clear();
        return QSharedPointer<Object>();
    }
    obj->SetSleep(false);

    if(forcedObjId) {
        obj->ResetSavedIndex(forcedObjId);
    }

    return sharedObj;

}
