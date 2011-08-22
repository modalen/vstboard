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

#ifndef METATRANSPORTER_H
#define METATRANSPORTER_H

class MetaObjEngine;
class MetaTransporter
{
public:
    MetaTransporter::MetaTransporter() :
        autoUpdate(false)
    {}

//    bool dropMime ( const QMimeData * data, MetaData & senderInfo, QPointF &pos, InsertionType::Enum insertType=InsertionType::NoInsertion ) =0;
    void ValueChanged( const MetaObjEngine & senderInfo, int type, const QVariant &value);
    void PostEvent( QEvent * event);
    void AddListener(QObject *obj) { listeners << obj; }
    void RemoveListener(QObject *obj) { listeners.removeAll(obj); }
    void RemoveAllListeners() { listeners.clear(); }

protected:
    bool autoUpdate;

private:
    QList<QObject*>listeners;
};

#endif // METATRANSPORTER_H
