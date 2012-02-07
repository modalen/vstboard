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
#include "gui.h"
#include <QtGui/QHBoxLayout>

namespace Steinberg
{
Gui::Gui() :
    widget(0),
    myWindow(0),
    plugFrame(0)
//    effect(effect),
//    resizeH(0)
{


    //reaper needs an offset.. can't find a good solution
//    char str[64];
//    effect->getHostProductString(str);
//    if(!strcmp(str,"REAPER")) {
//        widgetOffset.setY(27);
//    }

//    hostCanSizeWindow = (bool)effect->canHostDo("sizeWindow");
//    if(!hostCanSizeWindow)
//        qDebug()<<"host can't resize window";
}

Gui::~Gui()
{
    if(myWindow) {
        myWindow->writeSettings();
        myWindow->setParent(0);
    }
    if(widget) {
        delete widget;
        widget=0;
    }
}

void Gui::UpdateColor(ColorGroups::Enum groupId, Colors::Enum /*colorId*/, const QColor & /*color*/)
{
    if(groupId!=ColorGroups::Window)
        return;

    if(widget)
        widget->setPalette( myWindow->palette() );
}

void Gui::SetMainWindow(MainWindowVst *win)
{
    if(win==myWindow)
        return;

    myWindow = win;
    if(!myWindow)
        return;

    connect( myWindow->viewConfig, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
             this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)),
             Qt::UniqueConnection);
}

//void Gui::OnResizeHandleMove(const QPoint &pt)
//{
//    widget->resize( pt.x(), pt.y() );
//    widget->move(widgetOffset);

//    if(myWindow)
//        myWindow->resize(pt.x(), pt.y());

////    if(effect)
////        effect->sizeWindow(pt.x(), pt.y());

//    rectangle.right = pt.x();
//    rectangle.bottom = pt.y();
//}

tresult PLUGIN_API Gui::isPlatformTypeSupported (FIDString type)
{
#if WINDOWS
        if (strcmp (type, kPlatformTypeHWND) == 0)
                return kResultTrue;

#elif MAC
        #if MAC_CARBON
        if (strcmp (type, kPlatformTypeHIView) == 0)
                return kResultTrue;
        #endif

        #if MAC_COCOA
        if (strcmp (type, kPlatformTypeNSView) == 0)
                return kResultTrue;
        #endif
#endif

        return kInvalidArgument;
}

tresult PLUGIN_API Gui::attached (void* parent, FIDString /*type*/)
{
    if(!myWindow)
        return kInternalError;

//    AEffEditor::open(ptr);
    widget = new QWinWidget(static_cast<HWND>(parent));
    widget->setAutoFillBackground(false);
    widget->setObjectName("QWinWidget");

    myWindow->setParent(widget);
    myWindow->readSettings();
    myWindow->move(0,0);

    widget->move( 0, 0 );
    widget->resize( myWindow->size() );
    widget->setPalette( myWindow->palette() );

//    resizeH = new ResizeHandle(widget);
//    QPoint pos( widget->geometry().bottomRight() );
//    pos.rx()-=resizeH->width();
//    pos.ry()-=resizeH->height();
//    resizeH->move(pos);
//    resizeH->show();

//    connect(resizeH, SIGNAL(Moved(QPoint)),
//            this, SLOT(OnResizeHandleMove(QPoint)));

    widget->show();
    return kResultOk;
}

tresult PLUGIN_API Gui::removed ()
{
    if(myWindow) {
        myWindow->writeSettings();
        myWindow->setParent(0);
    }
    if(widget) {
        delete widget;
        widget=0;
    }

    return kResultOk;
}

tresult PLUGIN_API Gui::getSize (ViewRect* size)
{
    if (!size)
        return kInvalidArgument;

    if(widget) {
        size->left=0;
        size->top=0;
        size->right=widget->width();
        size->bottom=widget->height();
        return kResultTrue;
    }

    size->right = 600;
    size->bottom = 400;
    return kResultFalse;
}

tresult PLUGIN_API Gui::onSize (ViewRect* newSize)
{
    if (!newSize)
        return kInvalidArgument;

    if(myWindow) {
        myWindow->resize(newSize->getWidth(),newSize->getHeight());
    }
    if(widget) {
        widget->resize(newSize->getWidth(),newSize->getHeight());
//        widget->move(widgetOffset);
    }

    return kResultTrue;
}

//bool Gui::getRect (ERect** rect)
//{
//    if(!widget || !rect)
//        return false;

//    *rect = &rectangle;
//    return true;
//}

tresult PLUGIN_API Gui::setFrame (IPlugFrame* frame)
{
    plugFrame = frame;
    return kResultTrue;
}



}
