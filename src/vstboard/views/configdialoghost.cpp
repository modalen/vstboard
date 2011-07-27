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
#include "configdialoghost.h"
#include "../common/ui_configdialog.h"
#include "mainhost.h"

ConfigDialogHost::ConfigDialogHost(MainHost *myHost, QWidget *parent) :
    ConfigDialog(myHost, parent)
{
    ui->sampleRate->addItem("44.1 kHz",44100);
    ui->sampleRate->addItem("48 kHz",48000);
    ui->sampleRate->addItem("88.2 kHz",88200);
    ui->sampleRate->addItem("96 kHz",96000);

    int index=ui->sampleRate->findData( myHost->GetSampleRate() );
    if(index==-1) {
        LOG("invalid sample rate"<<index);
        return;
    }
    ui->sampleRate->setCurrentIndex(index);

//buffer size
//    ui->bufferSize->addItem("Auto",0);
//    index = ui->bufferSize->findData( (unsigned int)myHost->GetBufferSize() );
//    if(index==-1) {
//        debug("ConfigDialog invalid buffer size")
//        index=0;
//    }
//    ui->bufferSize->setCurrentIndex(index);
}

void ConfigDialogHost::accept()
{
    float rate = ui->sampleRate->itemData(ui->sampleRate->currentIndex()).toFloat();
    if(rate!=defaultSampleRate(myHost)) {
        myHost->SetSampleRate( rate );
    }
    myHost->SetSetting("sampleRate", rate);

//    int buffer = ui->bufferSize->itemData(ui->bufferSize->currentIndex()).toInt();
//    settings.setValue("bufferSize", buffer);

    ConfigDialog::accept();
}
