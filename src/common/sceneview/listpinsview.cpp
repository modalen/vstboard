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

#include "listpinsview.h"
#include "cableview.h"

using namespace View;

ListPinsView::ListPinsView(QGraphicsItem * parent, Qt::WindowFlags wFlags) :
    QGraphicsWidget(parent,wFlags)
{
    setObjectName("listPinView");
    layout = new QGraphicsLinearLayout(this);
    layout->setOrientation(Qt::Vertical);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);
}

void ListPinsView::UpdateCablesPosition()
{
    for(int i=0; i<layout->count(); i++) {
        static_cast<PinView*>(layout->itemAt(i))->UpdateCablesPosition();
    }
}

int ListPinsView::GetPinPosition(int newPinNumber)
{
    for(int i=0; i<layout->count(); i++) {
        if( static_cast<PinView*>(layout->itemAt(i))->MetaInfo::data.GetMetaData<int>(MetaInfos::PinNumber) > newPinNumber) {
            return i;
        }
    }
    return layout->count();
}

