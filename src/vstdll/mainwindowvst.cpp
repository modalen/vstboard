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
#include "mainwindowvst.h"
#include "mainhostvst.h"
#include "views/configdialogvst.h"

MainWindowVst::MainWindowVst(MainHost * myHost,QWidget *parent) :
        MainWindow(myHost,parent)
{
    ui->actionRefresh_Audio_devices->setDisabled(true);
    ui->actionRefresh_Midi_devices->setDisabled(true);
    setWindowTitle(APP_NAME);
    BuildListTools();
    setAcceptDrops(false);
}

void MainWindowVst::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindowVst::BuildListTools()
{
    MainWindow::BuildListTools();

    QStandardItem *parentItem=0;
    QStandardItem *item=0;
    ObjectInfo info;

    parentItem = listToolsModel->invisibleRootItem();

    //audio devices (vst in/out)
    //================================

    //vst audio in
    item = new QStandardItem(tr("Vst audio input"));
    ObjectInfo infoai;
    infoai.metaType = MetaTypes::object;
    infoai.listInfos[MetaInfos::ObjType] = ObjTypes::AudioInterface;
    infoai.listInfos[MetaInfos::Direction] = Directions::Input;
    infoai.name = "Vst audio In";
    item->setData(QVariant::fromValue(infoai), UserRoles::objInfo);
    parentItem->appendRow(item);

    //vst audio out
    item = new QStandardItem(tr("Vst audio output"));
    ObjectInfo infoao;
    infoao.metaType = MetaTypes::object;
    infoao.listInfos[MetaInfos::ObjType] = ObjTypes::AudioInterface;
    infoai.listInfos[MetaInfos::Direction] = Directions::Output;
    infoao.name = "Vst audio Out";
    item->setData(QVariant::fromValue(infoao), UserRoles::objInfo);
    parentItem->appendRow(item);

    //midi devices (vst in/out)
    //================================

    //vst midi in
    item = new QStandardItem(tr("Vst midi input"));
    ObjectInfo infomi;
    infomi.metaType = MetaTypes::object;
    infomi.listInfos[MetaInfos::ObjType] = ObjTypes::MidiInterface;
    infomi.inputs = 1;
    infomi.name = "Vst midi In";
    item->setData(QVariant::fromValue(infomi), UserRoles::objInfo);
    parentItem->appendRow(item);

    //vst midi out
    item = new QStandardItem(tr("Vst midi output"));
    ObjectInfo infomo;
    infomo.metaType = MetaTypes::object;
    infomo.listInfos[MetaInfos::ObjType] = ObjTypes::MidiInterface;
    infomo.outputs = 1;
    infomo.name = "Vst midi Out";
    item->setData(QVariant::fromValue(infomo), UserRoles::objInfo);
    parentItem->appendRow(item);

    //vst automation
    item = new QStandardItem(tr("Vst Automation"));
    info.metaType = MetaTypes::object;
    info.listInfos[MetaInfos::ObjType] = ObjTypes::VstAutomation;
    item->setData(QVariant::fromValue(info), UserRoles::objInfo);
    parentItem->appendRow(item);
}

void MainWindowVst::readSettings()
{
    MainWindow::readSettings();
    ui->dockAudioDevices->hide();
    ui->dockMidiDevices->hide();
}

void MainWindowVst::resetSettings()
{
    MainWindow::resetSettings();
    ui->dockAudioDevices->hide();
    ui->dockMidiDevices->hide();
}

void MainWindowVst::on_actionConfig_triggered()
{
    ConfigDialogVst conf(myHost,this);
    conf.exec();
}
