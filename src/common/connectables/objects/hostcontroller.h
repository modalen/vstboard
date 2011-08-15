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

#ifndef HOSTCONTROLLER_H
#define HOSTCONTROLLER_H

//#include "precomp.h"
#include "object.h"

namespace Connectables {

    class HostController : public Object
    {
        enum Param {
            Param_Tempo,
            Param_Sign1,
            Param_Sign2,
            Param_Group,
            Param_Prog,
            Param_Bar
        };

    Q_OBJECT
    public:
        explicit HostController(MainHost *myHost,MetaInfo &info);
        void Render();
        void SetContainer(ObjectInfo *container);
        Pin* CreatePin(MetaInfo &info);

    protected:
        QList<QVariant> listTempo;
        QList<QVariant> listSign1;
        QList<QVariant> listSign2;
        QList<QVariant> list128;

        bool tempoChanged;
        bool progChanged;
        bool grpChanged;

    signals:
        void progChange(int prog);
        void grpChange(int grp);
        void tempoChange(int tempo, int sign1, int sign2);

    public slots:
        void OnHostProgChanged(const QModelIndex &idx);
        void OnHostGroupChanged(const QModelIndex &idx);
        void OnHostTempoChange(int tempo, int sign1, int sign2);
        void OnParameterChanged(const MetaInfo &pinInfo, float value);
    };

}


#endif // HOSTCONTROLLER_H
