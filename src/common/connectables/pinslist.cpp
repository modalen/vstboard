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

#include "pinslist.h"
#include "pins/pin.h"
#include "audiobuffer.h"
#include "pins/audiopin.h"
#include "pins/midipinin.h"
#include "pins/midipinout.h"
#include "pins/bridgepinin.h"
#include "pins/bridgepinout.h"
#include "mainhost.h"

using namespace Connectables;

PinsList::PinsList(const MetaPinsList &info, Object *parentObj) :
    QObject(parentObj),
    parentObj(parentObj),
    MetaPinsList( info )
{
    MetaObjEngine::SetParent(parentObj);

    connect(this,SIGNAL(PinAdded(int)),
            this,SLOT(AddPin(int)));
    connect(this,SIGNAL(PinRemoved(int)),
            this,SLOT(RemovePin(int)));
//    connect(this, SIGNAL(NbPinChanged(int)),
//            this,SLOT(SetNbPins(int)));
}

void PinsList::Hide()
{
    foreach(Pin* pin, listPins) {
        pin->Close();
    }
}

void PinsList::ChangeNumberOfPins(int newNb)
{
    emit SetNbPins(newNb);
}

void PinsList::SetVisible(bool visible) {
    if(visible)
        SetMeta(metaT::Hidden,true);
    else
        DelMeta(metaT::Hidden);

    foreach(Pin* pin, listPins) {
        pin->SetVisible(visible);
    }
}

void PinsList::SetBridge(bool bridge)
{
    SetMeta(metaT::Hidden,true);

    foreach(Pin* pin, listPins) {
        pin->SetBridge(bridge);
    }
}

Pin * PinsList::GetPin(int pinNumber, bool autoCreate)
{
     //resize the list if needed
    if(!listPins.contains(pinNumber)) {
        if(autoCreate) {
            AddPin(pinNumber);
        } else {
//            LOG("pin not in list"<<pinNumber<<info().toString());
            return 0;
        }
    }

    return listPins.value(pinNumber);
}

AudioBuffer *PinsList::GetBuffer(int pinNumber)
{
//    if(connInfo.type != MediaTypes::Audio)
//        return 0;

    if(!listPins.contains(pinNumber)) {
        LOG("pin not found"<<pinNumber);
        return 0;
    }

    return static_cast<AudioPin*>(listPins.value(pinNumber))->GetBuffer();
}

void PinsList::ConnectAllTo(Container* container, const PinsList *other, bool hidden)
{
    QMap<quint16,Pin*>::Iterator i = listPins.begin();
    while(i!=listPins.end()) {
        Pin *otherPin = other->listPins.value(i.key(),0);
        if(otherPin)
            container->AddCable( *static_cast<MetaPin*>(i.value()), *static_cast<MetaPin*>(otherPin), hidden);
        ++i;
    }
}

void PinsList::AsyncAddPin(int nb)
{
    if(listPins.contains(nb))
        return;
    emit PinAdded(nb);
}

void PinsList::AsyncRemovePin(int nb)
{
    if(!listPins.contains(nb))
        return;
    emit PinRemoved(nb);
}

void PinsList::SetNbPins(int nb, QList<quint16> *listAdded,QList<quint16> *listRemoved)
{
    QMap<quint16,Pin*>::iterator i = listPins.begin();
    while(i!=listPins.end()) {
        if(i.key()>=nb && i.key()<FIXED_PIN_STARTINDEX) {
            if(listRemoved) {
                *listRemoved << i.key();
                ++i;
            } else {
                RemovePin(i.key());
                i=listPins.erase(i);
            }
        } else {
            ++i;
        }
    }

    int cpt=0;
    while(cpt<nb) {
        if(!listPins.contains(cpt)) {
            if(listAdded) {
                *listAdded<<cpt;
            } else {
                AddPin(cpt);
            }
        }
        cpt++;
    }


}

Pin * PinsList::AddPin(int nb)
{
    if(listPins.contains(nb))
        return listPins.value(nb);

    Pin *newPin = parentObj->CreatePin( getMetaForPin(nb) );
    if(!newPin) {
        LOG("pin not created"<<nb);
        return 0;
    }
    listPins.insert(nb, newPin);
    newPin->SetParent(this);
    if(ContainerId()) {
        newPin->SetContainer(ContainerInfo());
    }
    newPin->AddToView();

    parentObj->OnProgramDirty();
    return newPin;
}

void PinsList::RemovePin(int nb)
{
    if(!listPins.contains(nb))
        return;

    parentObj->OnProgramDirty();
    delete listPins.take(nb);
}

MetaData PinsList::getMetaForPin(int nb)
{
    if(listPins.contains(nb))
        return *static_cast<MetaData*>(listPins.value(nb));

    MetaData info(MetaType::pin, MetaData::GetNewId());
    info.SetMeta<QString>(metaT::ObjName,"pin");
    info.SetMeta(metaT::Media, GetMetaData<int>(metaT::Media));
    info.SetMeta(metaT::Direction, GetMetaData<int>(metaT::Direction));
//    info.SetParentId(ObjId());
//    info.SetContainerId(ContainerId());
//    info.SetParentObjectId(ParentObjectId());
    info.SetMeta(metaT::PinNumber,nb);
    return info;
}

QDataStream & PinsList::toStream(QDataStream & out) const
{
    out << *(MetaData*)this;
    out << (quint16)listPins.count();
    QMap<quint16,Pin*>::ConstIterator i = listPins.constBegin();
    while(i!=listPins.constEnd()) {
        Pin *pin=i.value();
        out << i.key();
        out << pin->GetValue();
        ++i;
    }

    return out;
}

QDataStream & PinsList::fromStream(QDataStream & in)
{
    in >> *(MetaData*)this;
    quint16 nbPins;
    in >> nbPins;
    for(quint16 i=0; i<nbPins; i++) {
        quint16 id;
        in >> id;
        QVariant value;
        in >> value;

        Pin *newPin = parentObj->CreatePin( getMetaForPin(id) );
        if(!newPin)
            return in;
        listPins.insert(id,newPin);
    }

    return in;
}

QDataStream & operator<< (QDataStream & out, const Connectables::PinsList& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream & in, Connectables::PinsList& value)
{
    return value.fromStream(in);
}
