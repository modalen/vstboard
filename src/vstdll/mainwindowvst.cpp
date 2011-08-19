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

}

void MainWindowVst::Init()
{
    MainWindow::Init();

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

    QStandardItem *parentItem = listToolsModel->invisibleRootItem();

    //audio devices (vst in/out)
    //================================

    {
        //vst audio in
        MetaInfo info(MetaTypes::object);
        info.data.SetMeta(MetaInfos::ObjType, ObjTypes::AudioInterface);
        info.data.SetMeta(MetaInfos::Direction, Directions::Input);
        info.SetName(tr("Vst audio In"));

        QStandardItem *item = new QStandardItem(tr("Vst audio input"));
        item->setData(QVariant::fromValue(info), UserRoles::metaInfo);
        parentItem->appendRow(item);
    }

    {
        //vst audio out
        MetaInfo info(MetaTypes::object);
        info.data.SetMeta(MetaInfos::ObjType, ObjTypes::AudioInterface);
        info.data.SetMeta(MetaInfos::Direction, Directions::Output);
        info.SetName(tr("Vst audio Out"));

        QStandardItem *item = new QStandardItem(tr("Vst audio output"));
        item->setData(QVariant::fromValue(info), UserRoles::metaInfo);
        parentItem->appendRow(item);
    }

    //midi devices (vst in/out)
    //================================

    {
        //vst midi in
        MetaInfo info(MetaTypes::object);
        info.data.SetMeta(MetaInfos::ObjType, ObjTypes::MidiInterface);
        info.data.SetMeta(MetaInfos::nbInputs, 1);
        info.SetName(tr("Vst midi In"));

        QStandardItem *item = new QStandardItem(tr("Vst midi input"));
        item->setData(QVariant::fromValue(info), UserRoles::metaInfo);
        parentItem->appendRow(item);
    }

    {
        //vst midi out
        MetaInfo info(MetaTypes::object);
        info.data.SetMeta(MetaInfos::ObjType, ObjTypes::MidiInterface);
        info.data.SetMeta(MetaInfos::nbOutputs, 1);
        info.SetName(tr("Vst midi Out"));

        QStandardItem *item = new QStandardItem(tr("Vst midi output"));
        item->setData(QVariant::fromValue(info), UserRoles::metaInfo);
        parentItem->appendRow(item);
    }

    {
        //vst automation
        MetaInfo info(MetaTypes::object);
        info.data.SetMeta(MetaInfos::ObjType, ObjTypes::VstAutomation);
        info.SetName(tr("Vst Automation"));

        QStandardItem *item = new QStandardItem(tr("Vst Automation"));
        item->setData(QVariant::fromValue(info), UserRoles::metaInfo);
        parentItem->appendRow(item);
    }
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
