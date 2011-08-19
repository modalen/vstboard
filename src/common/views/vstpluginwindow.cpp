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

#include "vstpluginwindow.h"
#include "ui_vstpluginwindow.h"
#include "connectables/objects/vstplugin.h"
#include "mainhost.h"

using namespace View;

VstPluginWindow::VstPluginWindow(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::VstPluginWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_ShowWithoutActivating);
    ui->setupUi(this);
}

VstPluginWindow::~VstPluginWindow()
{
    if(plugin)
        plugin->editorWnd=0;
    delete ui;
}

WId VstPluginWindow::GetWinId()
{
    return ui->scrollAreaWidgetContents->winId();
}

void VstPluginWindow::UnsetPlugin()
{
    if(plugin) {
        disconnect(plugin);
        plugin->EffEditClose();
        plugin = 0;
    }
    close();
}

bool VstPluginWindow::SetPlugin(QObject *plug)
{
    plugin = static_cast<Connectables::VstPlugin*>(plug);
    if(!plugin)
        return false;

    bool windowOk = false;

    setWindowFlags(Qt::Tool);

    long res;
    ERect *rect = 0;

    //try to get rect (some plugins wants it before editopen)
    res = plugin->EffEditGetRect(&rect);
    if(res == 1L)
        windowOk=true;

    //try to open the window
//    plugin->objMutex.lock();
    res = plugin->EffEditOpen(GetWinId());
//    plugin->objMutex.unlock();

    if(res == 1L)
        windowOk=true;

    //try to get rect again
    res = plugin->EffEditGetRect(&rect);
    if(res == 1L)
        windowOk=true;

    if(!windowOk) {
        plugin=0;
        return false;
    }

    SetWindowSize(rect->right, rect->bottom);
    setWindowTitle(plugin->objectName());

    plugin->SetEditorWnd(this);
    return true;
}

void VstPluginWindow::LoadAttribs()
{
    if(!plugin)
        return;

    QSize s = plugin->data.GetMetaData<QSize>(MetaInfos::EditorSize);
    if(s==QSize(-1,-1))
        return;

    resize( s );
    move( plugin->data.GetMetaData<QPoint>(MetaInfos::EditorPosition) );
    ui->scrollArea->verticalScrollBar()->setValue( plugin->data.GetMetaData<int>(MetaInfos::EditorVScroll) );
    ui->scrollArea->horizontalScrollBar()->setValue( plugin->data.GetMetaData<int>(MetaInfos::EditorHScroll) );
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void VstPluginWindow::SaveAttribs()
{
    if(!plugin)
        return;

    plugin->data.SetMeta(MetaInfos::EditorSize, size());
    plugin->data.SetMeta(MetaInfos::EditorPosition, pos());
    plugin->data.SetMeta(MetaInfos::EditorVScroll, ui->scrollArea->verticalScrollBar()->value());
    plugin->data.SetMeta(MetaInfos::EditorHScroll, ui->scrollArea->horizontalScrollBar()->value());
}

void VstPluginWindow::closeEvent( QCloseEvent * event )
{
    if(!plugin) {
        event->accept();
        return;
    }

    hide();
    emit Hide();
    event->ignore();
}

void VstPluginWindow::SetWindowSize(int newWidth, int newHeight)
{
    ui->scrollAreaWidgetContents->setFixedSize(newWidth,newHeight);
    setMaximumSize(newWidth,newHeight);
    resize(newWidth,newHeight);
}

void VstPluginWindow::showEvent ( QShowEvent * event )
{
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    int w = ui->scrollAreaWidgetContents->width();
    int h = ui->scrollAreaWidgetContents->height();
    setMaximumSize(w,h);
}

void VstPluginWindow::resizeEvent ( QResizeEvent * event )
{
    int maxH=ui->scrollAreaWidgetContents->height();
    int maxW=ui->scrollAreaWidgetContents->width();

    if( event->size().width() >= maxW &&
        event->size().height() >= maxH &&
        ui->scrollArea->horizontalScrollBarPolicy()!=Qt::ScrollBarAlwaysOff) {

        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setMaximumSize(maxW,maxH);
    } else
        if(ui->scrollArea->horizontalScrollBarPolicy()!=Qt::ScrollBarAsNeeded) {

        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        maxW+=ui->scrollArea->verticalScrollBar()->width();
        maxH+=ui->scrollArea->horizontalScrollBar()->height();
        setMaximumSize(maxW,maxH);
    }
}


//const QPixmap VstPluginWindow::GetScreenshot()
//{
//    QPixmap pix = QPixmap::grabWindow(ui->scrollAreaWidgetContents->winId() ,0,0,size().width(),size().height());//45);
//    QPainter p(&pix);
//    p.fillRect(pix.rect(), QColor(255,255,255,150));
//    return pix;
//}
