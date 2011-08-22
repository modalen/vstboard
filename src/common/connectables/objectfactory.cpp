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
#include "objects/midisender.h"
#include "objects/miditoautomation.h"
#include "objects/hostcontroller.h"
#include "objects/container.h"
#include "objects/bridge.h"
#include "mainhost.h"

#ifdef SCRIPTENGINE
    #include "objects/script.h"
#endif

#ifdef VSTSDK
    #include "objects/vstplugin.h"
    #include "vst/cvsthost.h"
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

Pin *ObjectFactory::GetPin(const MetaData &pinInfo)
{
    if(!listObjects.contains(pinInfo.GetMetaData<quint32>(metaT::ParentObjId))) {
        LOG("obj not found"<<pinInfo.toString());
        return 0;
    }

    QSharedPointer<Object> objPtr = listObjects.value( pinInfo.GetMetaData<quint32>(metaT::ParentObjId) ).toStrongRef();
    if(objPtr)
        return objPtr->GetPin(pinInfo);

    return 0;
}

bool ObjectFactory::UpdatePinInfo(MetaData &pinInfo)
{
    if(!listObjects.contains(pinInfo.GetMetaData<quint32>(metaT::ParentObjId))) {
        LOG("obj not found"<<pinInfo.toString());
        return false;
    }

    QSharedPointer<Object> objPtr = listObjects.value( pinInfo.GetMetaData<quint32>(metaT::ParentObjId) ).toStrongRef();
    if(!objPtr)
        return false;

    Pin *pin = objPtr->GetPin(pinInfo);
    if(!pin) {
        pinInfo=MetaData();
        return false;
    }
    pinInfo = *static_cast<MetaData*>(pin);
    return true;
}

QSharedPointer<Object> ObjectFactory::NewObject( MetaData &info)
{
    int forcedObjId = 0;//cptListObjects;
    if(info.MetaId()!=0) {
        forcedObjId = info.MetaId();
        if(listObjects.contains(forcedObjId)) {
            LOG("forcedId already exists"<<forcedObjId);
        }
    } else {
        info.SetMetaId( GetNewId() );
    }

    Object *obj=0;

    obj=CreateOtherObjects(info);

    if(!obj) {
        switch(info.Type()) {

            case MetaType::container :
                obj = new Container(myHost, info);
                break;

            case MetaType::bridge :
                obj = new Bridge(myHost, info);
                break;

            case MetaType::object :

                switch(info.GetMetaData<int>(metaT::ObjType)) {
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
                        info.SetMeta<QString>(metaT::errorMessage,"Dummy object");
                        obj = new Object(myHost, info);
                        break;

                    default:
                        LOG("unknown object type");
                        return QSharedPointer<Object>();
                }
                break;


            default :
                LOG("unknown nodeType"<<info.Type());
                return QSharedPointer<Object>();
        }
    }

    QSharedPointer<Object> sharedObj(obj);
    listObjects.insert(info.MetaId(),sharedObj.toWeakRef());

    if(!obj->Open()) {
        listObjects.remove(info.MetaId());
        sharedObj.clear();
        return QSharedPointer<Object>();
    }
    obj->SetSleep(false);

    if(forcedObjId) {
        obj->ResetSavedIndex(forcedObjId);
    }

    return sharedObj;

}
