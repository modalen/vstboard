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
#include "mididevices.h"
#include "objectinfo.h"
#include "mainhost.h"

MidiDevices::MidiDevices(MainHost *myHost) :
        QObject(myHost),
        model(0),
        myHost(myHost)
{
    Init();
}

MidiDevices::~MidiDevices()
{
    if(Pt_Started())
        Pt_Stop();

//    if(model) {
//        model->deleteLater();
        Pm_Terminate();
//    }
}

bool MidiDevices::Init()
{
//    mutexListMidi.lock();

    foreach(Connectables::MidiDevice* md, listOpenedMidiDevices) {
        md->SetSleep(true);
        md->CloseStream();
    }

    if(Pt_Started())
        Pt_Stop();

    if(model) {
        Pm_Terminate();
        model->deleteLater();
    }

    PmError pmRet = Pm_Initialize();
    if(pmRet!=pmNoError) {
        QMessageBox msgBox;
        msgBox.setText(tr("Unable to initialize midi engine : %1").arg( Pm_GetErrorText(pmRet) ));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return false;
    }

    PtError ptRet = Pt_Start(1, MidiDevices::MidiReceive_poll,this);
    if(ptRet!=ptNoError) {
        QMessageBox msgBox;
        msgBox.setText(tr("Unable to start midi engine"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return false;
    }

    BuildModel();

    foreach(QSharedPointer<Connectables::Object>obj, myHost->objFactory->GetListObjects()) {
        if(obj.isNull())
            continue;

        if(obj->info().GetMetaData<int>(metaT::ObjType) == ObjTypes::MidiInterface) {
//            if(!obj->errorMessage.isEmpty())
                obj->Open();
//            obj->UpdateModelNode();
                obj->AddToView();
        }
    }

    foreach(Connectables::MidiDevice* md, listOpenedMidiDevices) {
        if(md->OpenStream())
            md->SetSleep(false);
    }

//    mutexListMidi.unlock();

    return true;
}

void MidiDevices::OpenDevice(Connectables::MidiDevice* objPtr)
{
//    mutexListMidi.lock();
    listOpenedMidiDevices << objPtr;
//    mutexListMidi.unlock();
}

void MidiDevices::CloseDevice(Connectables::MidiDevice* objPtr)
{
//    mutexListMidi.lock();
    listOpenedMidiDevices.removeAll( objPtr );
//    mutexListMidi.unlock();
}

void MidiDevices::BuildModel()
{
    QStringList headerLabels;
    headerLabels << "Name";
    headerLabels << "In";
    headerLabels << "Out";

    if(model)
        model->deleteLater();
    model = new ListMidiInterfacesModel(this);
    model->setHorizontalHeaderLabels(  headerLabels );
    QStandardItem *parentItem = model->invisibleRootItem();

    QString lastName;
    int cptDuplicateNames=0;

    for(int i=0;i<Pm_CountDevices();i++) {
        QList<QStandardItem *>  items;
        const PmDeviceInfo *devInfo = Pm_GetDeviceInfo(i);

        QString devName= QString::fromLocal8Bit(devInfo->name);
        if(lastName == devName) {
            cptDuplicateNames++;
        } else {
            cptDuplicateNames=0;
        }
        lastName = devName;

        MetaData obj(MetaType::object);
        obj.SetName(devName);
        obj.SetMeta(metaT::ObjType, ObjTypes::MidiInterface);
        obj.SetMeta(metaT::devId, i);
        obj.SetMeta(metaT::devName, devName);
        obj.SetMeta(metaT::apiName, QString::fromLocal8Bit(devInfo->interf ));
        obj.SetMeta(metaT::duplicateNamesCounter, cptDuplicateNames);
        obj.SetMeta(metaT::nbInputs, devInfo->input);
        obj.SetMeta(metaT::nbOutputs, devInfo->output);

        items << new QStandardItem(devName);
        items << new QStandardItem(QString::number(devInfo->input));
        items << new QStandardItem(QString::number(devInfo->output));

        items[0]->setData(QVariant::fromValue(obj), UserRoles::metaInfo);

        parentItem->appendRow(items);
    }

}

//midi interfaces entry point
//===============================
void MidiDevices::MidiReceive_poll(PtTimestamp timestamp, void *userData)
{
    PmEvent buffer;
    PmError result = pmNoError;

    MidiDevices *devices = static_cast<MidiDevices*>(userData);

//    devices->mutexListMidi.lock();
    foreach(Connectables::MidiDevice* device, devices->listOpenedMidiDevices) {
        if(device->GetSleep())
            continue;

        if(!device->stream || !device->queue)
           continue;

        //lock device while processing (no rendering, no delete)
//        device->objMutex.lock();

        //it's a midi input
        if(device->devInfo->input) {
            do {
                result = Pm_Poll(device->stream);
                if (result) {
                    PmError rslt = (PmError)Pm_Read(device->stream, &buffer, 1);
                    if (rslt == pmBufferOverflow) {
                        LOG("midi buffer overflow");
                        continue;
                    }
                    if(rslt == 1 ) {
                        Pm_Enqueue(device->queue, &buffer);
                    } else {
                        LOG("midi in error on");
                        continue;
                    }
                }
            } while (result);
        }

        //it's a midi output
        if(device->devInfo->output) {
            while (!Pm_QueueEmpty(device->queue)) {
                result = Pm_Dequeue(device->queue, &buffer);
                Pm_Write(device->stream, &buffer, 1);
            }
        }

//        device->objMutex.unlock();
    }
//    devices->mutexListMidi.unlock();
}
