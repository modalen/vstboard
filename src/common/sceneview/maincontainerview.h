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

#ifndef MAINCONTAINERVIEW_H
#define MAINCONTAINERVIEW_H

#include "objectview.h"
#include "containercontent.h"
#include "cableview.h"
#include "bridgeview.h"

class SceneModel;
namespace View {

    class MainContainerView : public ObjectView
    {
    Q_OBJECT
    public:
        explicit MainContainerView(const MetaInfo &info, SceneModel *model);
        ~MainContainerView();
        void SetParking(QWidget *parking);
        QWidget *GetParking();
        void SetConfig(ViewConfig *config);

        BridgeView *bridgeIn;
        BridgeView *bridgeOut;
        BridgeView *bridgeSend;
        BridgeView *bridgeReturn;

    private:
        ContainerContent *content;
        QList<CableView*>listCables;
        SceneModel *model;

    signals:
        void ParkingChanged(QWidget *parking);

    public slots:
        void OnViewChanged(QRectF rect);

    };
}

#endif // MAINCONTAINERVIEW_H
