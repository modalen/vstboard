/******************************************************************************
#    Copyright 2010 Rapha�l Fran�ois
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
******************************************************************************/

#include "cable.h"
#include "../mainhost.h"

using namespace Connectables;

Cable::Cable(const ConnectionInfo &pinOut, const ConnectionInfo &pinIn) :
        pinOut(pinOut),
        pinIn(pinIn),
        modelIndex(QModelIndex())
{

}

Cable::Cable(const Cable & c) :
        pinOut(c.pinOut),
        pinIn(c.pinIn),
        modelIndex(c.modelIndex)
{

}

void Cable::AddToParentNode(const QModelIndex &parentIndex)
{
    QStandardItem *item = new QStandardItem(QString("cable %1:%2").arg(pinOut.objId).arg(pinIn.objId));
    item->setData(QVariant::fromValue(ObjectInfo(NodeType::cable)),UserRoles::objInfo);
    item->setData(QVariant::fromValue(pinOut),UserRoles::value);
    item->setData(QVariant::fromValue(pinIn),UserRoles::connectionInfo);

    MainHost::GetModel()->itemFromIndex(parentIndex)->appendRow(item);
    modelIndex = item->index();
}

void Cable::RemoveFromParentNode(const QModelIndex &parentIndex)
{
    if(modelIndex.isValid() && parentIndex.isValid())
        MainHost::GetModel()->removeRow(modelIndex.row(), parentIndex);

    modelIndex=QModelIndex();
}