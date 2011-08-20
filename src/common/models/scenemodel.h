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

#ifndef HOSTMODEL_H
#define HOSTMODEL_H

#include "precomp.h"
#include "objectinfo.h"
#include "globals.h"

namespace View {
    class SceneView;
}

class MainHost;
class MetaData;
class SceneModel : public QObject, public MetaTransporter
{
    Q_OBJECT
public:
    SceneModel(MainHost *myHost, View::SceneView *view, QObject *parent=0);
    bool dropMime ( const QMimeData * data, MetaData & senderInfo, QPointF &pos, InsertionType::Enum insertType=InsertionType::NoInsertion );
    void valueChanged( const MetaData & senderInfo, int type, const QVariant &value);
    bool event(QEvent *event);

protected:
    bool dropFile( const QString &fName, MetaData &info, MetaData &senderInfo);

    MainHost *myHost;
    QTimer *delayedAction;
    QSignalMapper *LoadFileMapper;
    View::SceneView *mySceneView;
};

#endif // HOSTMODEL_H
