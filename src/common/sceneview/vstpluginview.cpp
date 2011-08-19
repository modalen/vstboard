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

#include "vstpluginview.h"

using namespace View;
class SceneModel;

VstPluginView::VstPluginView(const MetaInfo &info, MainContainerView * parent, SceneModel *model) :
    ConnectableObjectView(info,parent,model)
{
    setObjectName("vstPluginView");

    setAcceptDrops(true);

    actSaveBank = new QAction(QIcon(":/img16x16/filesave.png"),tr("Save Bank"),this);
    actSaveBank->setShortcut( Qt::CTRL + Qt::Key_B );
    actSaveBank->setShortcutContext(Qt::WidgetShortcut);
    connect(actSaveBank,SIGNAL(triggered()),
            this,SLOT(SaveBank()));
    addAction(actSaveBank);

    actSaveBankAs = new QAction(QIcon(":/img16x16/filesaveas.png"),tr("Save Bank As..."),this);
    actSaveBankAs->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_B );
    actSaveBankAs->setShortcutContext(Qt::WidgetShortcut);
    connect(actSaveBankAs,SIGNAL(triggered()),
            this,SLOT(SaveBankAs()));
    actSaveBankAs->setEnabled(false);
    addAction(actSaveBankAs);

    actSaveProgram = new QAction(QIcon(":/img16x16/filesave.png"),tr("Save Program"),this);
    actSaveProgram->setShortcut( Qt::CTRL + Qt::Key_P );
    actSaveProgram->setShortcutContext(Qt::WidgetShortcut);
    connect(actSaveProgram,SIGNAL(triggered()),
            this,SLOT(SaveProgram()));
    addAction(actSaveProgram);

    actSaveProgramAs = new QAction(QIcon(":/img16x16/filesaveas.png"),tr("Save Program As..."),this);
    actSaveProgramAs->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_P );
    actSaveProgramAs->setShortcutContext(Qt::WidgetShortcut);
    connect(actSaveProgramAs,SIGNAL(triggered()),
            this,SLOT(SaveProgramAs()));
    actSaveProgramAs->setEnabled(false);
    addAction(actSaveProgramAs);
}

void VstPluginView::SetConfig(ViewConfig *config)
{
    ConnectableObjectView::SetConfig(config);

    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::VstPlugin,Colors::Background) );
    setPalette( pal );
}

void VstPluginView::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(colorId==Colors::Background)  {
        if(groupId==ColorGroups::VstPlugin) {
            if(!highlighted) {
                QPalette pal(palette());
                pal.setColor(QPalette::Window,color);
                setPalette( pal );
            }
        }
        return;
    }

    if(groupId==ColorGroups::Object && colorId==Colors::HighlightBackground)  {
        if(highlighted) {
            QPalette pal(palette());
            pal.setColor(QPalette::Window,color);
            setPalette( pal );
        }
    }
    ConnectableObjectView::UpdateColor(groupId,colorId,color);
}

void VstPluginView::UpdateModelIndex(const MetaInfo &info)
{
    ConnectableObjectView::UpdateModelIndex(info);

    if(MetaInfo::data.GetMeta<QString*>(MetaInfos::bankFile)!=0 && !MetaInfo::data.GetMetaData<QString>(MetaInfos::bankFile).isEmpty()) {
        QFileInfo fileInfo;
        fileInfo.setFile( MetaInfo::data.GetMetaData<QString>(MetaInfos::bankFile) );
        actSaveBank->setText( QString(tr("Save Bank (%1)")).arg(fileInfo.fileName()) );
        actSaveBankAs->setEnabled(true);
    } else {
        actSaveBank->setText( tr("Save Bank") );
        actSaveBankAs->setEnabled(false);
    }

    if(MetaInfo::data.GetMeta<QString*>(MetaInfos::programFile)!=0 && !MetaInfo::data.GetMetaData<QString>(MetaInfos::programFile).isEmpty()) {
        QFileInfo fileInfo;
        fileInfo.setFile( MetaInfo::data.GetMetaData<QString>(MetaInfos::bankFile) );
        actSaveProgram->setText( QString(tr("Save Program (%1)")).arg(fileInfo.fileName()) );
        actSaveProgramAs->setEnabled(true);
    } else {
        actSaveProgram->setText( tr("Save Program") );
        actSaveProgramAs->setEnabled(false);
    }
}

void VstPluginView::SaveBankAs()
{
//    QString lastDir = myHost->GetSetting("lastBankPath").toString();
//    QString filename = QFileDialog::getSaveFileName(0, tr("Save Bank"), lastDir, tr("Bank File (*.%1)").arg(VST_BANK_FILE_EXTENSION) );
//    if(filename.isEmpty())
//        return;

//    if( !filename.endsWith( QString(".")+VST_BANK_FILE_EXTENSION, Qt::CaseInsensitive ) ) {
//        filename.append( QString(".")+VST_BANK_FILE_EXTENSION );
//    }

//    myHost->SetSetting("lastBankPath",QFileInfo(filename).absolutePath());
//    data.SetMeta(MetaInfos::bankFile,filename);
}

void VstPluginView::SaveBank()
{
    if(MetaInfo::data.GetMeta<QString*>(MetaInfos::bankFile)!=0 && !MetaInfo::data.GetMetaData<QString>(MetaInfos::bankFile).isEmpty()) {
        MetaInfo::data.SetMeta(MetaInfos::bankFile, MetaInfo::data.GetMetaData<QString>(MetaInfos::bankFile) );
    } else {
        SaveBankAs();
    }
}

void VstPluginView::SaveProgramAs()
{
//    QString lastDir = myHost->GetSetting("lastBankPath").toString();
//    QString filename = QFileDialog::getSaveFileName(0, tr("Save Program"), lastDir, tr("Program File (*.%1)").arg(VST_PROGRAM_FILE_EXTENSION) );
//    if(filename.isEmpty())
//        return;

//    if( !filename.endsWith(QString(".")+VST_PROGRAM_FILE_EXTENSION, Qt::CaseInsensitive) ) {
//        filename.append(QString(".")+VST_PROGRAM_FILE_EXTENSION);
//    }

//    myHost->SetSetting("lastBankPath",QFileInfo(filename).absolutePath());
//    data.SetMeta(MetaInfos::programFile,filename);
}

void VstPluginView::SaveProgram()
{
    if(MetaInfo::data.GetMeta<QString*>(MetaInfos::programFile)!=0 && !MetaInfo::data.GetMetaData<QString>(MetaInfos::programFile).isEmpty()) {
        MetaInfo::data.SetMeta(MetaInfos::programFile, MetaInfo::data.GetMetaData<QString>(MetaInfos::programFile) );
    } else {
        SaveProgramAs();
    }
}

void VstPluginView::dragEnterEvent( QGraphicsSceneDragDropEvent *event)
{
    //accept fxp files and replacement plugin
    if (event->mimeData()->hasUrls()) {
        QString fName;
        QFileInfo info;

        QStringList acceptedFiles;
        acceptedFiles << "fxb" << "fxp" ;

        foreach(QUrl url,event->mimeData()->urls()) {
            fName = url.toLocalFile();
            info.setFile( fName );
            if ( info.isFile() && info.isReadable() ) {
                if( acceptedFiles.contains( info.suffix(), Qt::CaseInsensitive) ) {
                    event->setDropAction(Qt::TargetMoveAction);
                    event->accept();
                    HighlightStart();
                    return;
                }
            }
        }
    }
    event->ignore();
}

void VstPluginView::dragLeaveEvent( QGraphicsSceneDragDropEvent *event)
{
    HighlightStop();
}

//void VstPluginView::dropEvent( QGraphicsSceneDragDropEvent *event)
//{
//    HighlightStop();
//    QGraphicsWidget::dropEvent(event);
////    event->setAccepted(model->dropMimeData(event->mimeData(), event->proposedAction(), info()));
////    DropMime(event->mimeData(), event->proposedAction());
//    Events::dropMime *e = new Events::dropMime(event->mimeData(), info(), insertType);
//    myHost->mainWindow->PostEvent(e);
//}

void VstPluginView::HighlightStart()
{
    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Object,Colors::HighlightBackground) );
    setPalette( pal );
}

void VstPluginView::HighlightStop()
{
    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::VstPlugin,Colors::Background) );
    setPalette( pal );
}
