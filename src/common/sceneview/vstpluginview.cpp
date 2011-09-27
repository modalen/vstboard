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

#include "vstpluginview.h"

using namespace View;

VstPluginView::VstPluginView(MainHost *myHost,QAbstractItemModel *model,MainContainerView * parent, Qt::WindowFlags wFlags) :
    ConnectableObjectView(myHost,model,parent,wFlags),
    actSaveBank(0),
    actSaveBankAs(0),
    actSaveProgram(0),
    actSaveProgramAs(0)
{
    setObjectName("vstPluginView");

    setAcceptDrops(true);

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

void VstPluginView::SetModelIndex(QPersistentModelIndex index)
{
    ConnectableObjectView::SetModelIndex(index);

    actSaveBank = new QAction(QIcon(":/img16x16/filesave.png"),tr("Save Bank"),this);
    actSaveBank->setShortcutContext(Qt::WidgetShortcut);
    connect(actSaveBank,SIGNAL(triggered()),
            this,SLOT(SaveBank()));
    addAction(actSaveBank);

    actSaveBankAs = new QAction(QIcon(":/img16x16/filesaveas.png"),tr("Save Bank As..."),this);
    actSaveBankAs->setShortcutContext(Qt::WidgetShortcut);
    connect(actSaveBankAs,SIGNAL(triggered()),
            this,SLOT(SaveBankAs()));
    actSaveBankAs->setEnabled(false);
    addAction(actSaveBankAs);

    actSaveProgram = new QAction(QIcon(":/img16x16/filesave.png"),tr("Save Program"),this);
    actSaveProgram->setShortcutContext(Qt::WidgetShortcut);
    connect(actSaveProgram,SIGNAL(triggered()),
            this,SLOT(SaveProgram()));
    addAction(actSaveProgram);

    actSaveProgramAs = new QAction(QIcon(":/img16x16/filesaveas.png"),tr("Save Program As..."),this);
    actSaveProgramAs->setShortcutContext(Qt::WidgetShortcut);
    connect(actSaveProgramAs,SIGNAL(triggered()),
            this,SLOT(SaveProgramAs()));
    actSaveProgramAs->setEnabled(false);
    addAction(actSaveProgramAs);

    UpdateKeyBinding();
}

void VstPluginView::UpdateKeyBinding()
{
    ObjectView::UpdateKeyBinding();
    if(actSaveBank) actSaveBank->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::saveBank) );
    if(actSaveBankAs) actSaveBankAs->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::saveBankAs) );
    if(actSaveProgram) actSaveProgram->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::saveProgram) );
    if(actSaveProgramAs) actSaveProgramAs->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::saveProgramAs) );
}

void VstPluginView::UpdateModelIndex()
{
    ConnectableObjectView::UpdateModelIndex();
    if(objIndex.data(UserRoles::bankFile).isValid() && !objIndex.data(UserRoles::bankFile).toString().isEmpty()) {
        QFileInfo info;
        info.setFile( objIndex.data(UserRoles::bankFile).toString() );
        actSaveBank->setText( QString(tr("Save Bank (%1)")).arg(info.fileName()) );
        actSaveBankAs->setEnabled(true);
    } else {
        actSaveBank->setText( tr("Save Bank") );
        actSaveBankAs->setEnabled(false);
    }

    if(objIndex.data(UserRoles::programFile).isValid() && !objIndex.data(UserRoles::programFile).toString().isEmpty()) {
        QFileInfo info;
        info.setFile( objIndex.data(UserRoles::programFile).toString() );
        actSaveProgram->setText( QString(tr("Save Program (%1)")).arg(info.fileName()) );
        actSaveProgramAs->setEnabled(true);
    } else {
        actSaveProgram->setText( tr("Save Program") );
        actSaveProgramAs->setEnabled(false);
    }
}

void VstPluginView::SaveBankAs()
{
    QString lastDir = myHost->GetSetting("lastBankPath").toString();
    QString filename = QFileDialog::getSaveFileName(0, tr("Save Bank"), lastDir, tr("Bank File (*.%1)").arg(VST_BANK_FILE_EXTENSION) );
    if(filename.isEmpty())
        return;

    if( !filename.endsWith( QString(".")+VST_BANK_FILE_EXTENSION, Qt::CaseInsensitive ) ) {
        filename.append( QString(".")+VST_BANK_FILE_EXTENSION );
    }

    myHost->SetSetting("lastBankPath",QFileInfo(filename).absolutePath());
    model->setData(objIndex,filename,UserRoles::bankFile);
}

void VstPluginView::SaveBank()
{
    if(objIndex.data(UserRoles::bankFile).isValid() && !objIndex.data(UserRoles::bankFile).toString().isEmpty()) {
        model->setData(objIndex, objIndex.data(UserRoles::bankFile).toString(), UserRoles::bankFile);
    } else {
        SaveBankAs();
    }
}

void VstPluginView::SaveProgramAs()
{
    QString lastDir = myHost->GetSetting("lastBankPath").toString();
    QString filename = QFileDialog::getSaveFileName(0, tr("Save Program"), lastDir, tr("Program File (*.%1)").arg(VST_PROGRAM_FILE_EXTENSION) );
    if(filename.isEmpty())
        return;

    if( !filename.endsWith(QString(".")+VST_PROGRAM_FILE_EXTENSION, Qt::CaseInsensitive) ) {
        filename.append(QString(".")+VST_PROGRAM_FILE_EXTENSION);
    }

    myHost->SetSetting("lastBankPath",QFileInfo(filename).absolutePath());
    model->setData(objIndex,filename,UserRoles::programFile);
}

void VstPluginView::SaveProgram()
{
    if(objIndex.data(UserRoles::programFile).isValid() && !objIndex.data(UserRoles::programFile).toString().isEmpty()) {
        model->setData(objIndex, objIndex.data(UserRoles::programFile).toString(), UserRoles::programFile);
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

void VstPluginView::dropEvent( QGraphicsSceneDragDropEvent *event)
{
    HighlightStop();
    QGraphicsWidget::dropEvent(event);
    event->setAccepted(model->dropMimeData(event->mimeData(), event->proposedAction(), 0, 0, objIndex));
}

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
