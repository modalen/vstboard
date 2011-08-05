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
#include "gui.h"
#include <QtGui/QHBoxLayout>

Gui::Gui(AudioEffectX* effect) : widget(0), effect(effect)
{

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

bool Gui::open(void* ptr)
{
    if(!ptr)
        return false;

    AEffEditor::open(ptr);
    widget = new QWinWidget(static_cast<HWND>(ptr));
    widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    QHBoxLayout layout(widget);
    layout.setContentsMargins(0,0,0,0);
    layout.addWidget(myWindow);

    widget->move( 0, 0 );
    widget->adjustSize();
    rectangle.top = 0;
    rectangle.left = 0;
    rectangle.bottom = widget->height();
    rectangle.right = widget->width();

    widget->show();
    myWindow->readSettings();
//    clientResize(static_cast<HWND>(ptr), widget->width(), widget->height());
    return true;
}

void Gui::close()
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

bool Gui::getRect (ERect** rect)
{
    if(!widget || !rect)
        return false;

    *rect = &rectangle;
    return true;
}

//void clientResize(HWND h_parent, int width, int height)
//{
//    RECT rcClient, rcWindow;
//    POINT ptDiff;
//    GetClientRect(h_parent, &rcClient);
//    GetWindowRect(h_parent, &rcWindow);
//    ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
//    ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
//    MoveWindow(h_parent, rcWindow.left, rcWindow.top, width + ptDiff.x, height + ptDiff.y, TRUE);
//}
