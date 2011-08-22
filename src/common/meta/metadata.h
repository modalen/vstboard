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

#ifndef METADATA_H
#define METADATA_H

#include "precomp.h"
#include "types.h"

class MetaData;
typedef QMultiMap < MetaData, MetaData > mapCables;

class MetaData
{
public:
    MetaData(MetaType::Enum metaType=MetaType::ND, quint32 metaId=0) :
        metaType(metaType),
        metaId(metaId)
    {}

    MetaData(const MetaData &c) {
        *this=c;
    }

    MetaData& operator=(const MetaData& c);

    ~MetaData() {
        qDeleteAll(listInfos);
    }

    static quint32 GetNewId() {
        return nextId++;
    }

    void SetMetaId(const quint32 id) {metaId=id;}
    const quint32 MetaId() const {return metaId;}

    const MetaType::Enum Type() const {return metaType;}

    template<class T> const T GetMetaPtr(const metaT::Enum type) const {
        return static_cast<const T>(listInfos.value(type,0));
    }

    template<class T> T GetMetaData(const metaT::Enum type) const {
        if(!listInfos.contains(type))
            return T();
        return *static_cast<const T*>(listInfos.value(type));
    }

    template<class T> void SetMeta(const metaT::Enum type, const T &value) {
        if(listInfos.contains(type))
            DelMeta(type);
        AddMeta(type,value);
    }

    template<> void SetMeta(const metaT::Enum type, const float &value) {
        SetMetaFast(type,value);
    }
    template<> void SetMeta(const metaT::Enum type, const int &value) {
        SetMetaFast(type,value);
    }
    template<> void SetMeta(const metaT::Enum type, const bool &value) {
        SetMetaFast(type,value);
    }
    template<> void SetMeta(const metaT::Enum type, const MetaType::Enum &value) {
        SetMetaFast(type,value);
    }
    template<> void SetMeta(const metaT::Enum type, const ObjTypes::Enum &value) {
        SetMetaFast(type,value);
    }
    template<> void SetMeta(const metaT::Enum type, const MediaTypes::Enum &value) {
        SetMetaFast(type,value);
    }
    template<> void SetMeta(const metaT::Enum type, const Directions::Enum &value) {
        SetMetaFast(type,value);
    }

    void DelMeta(const metaT::Enum type) {
        delete listInfos.take(type);
    }

//#ifndef QT_NO_DEBUG
    QString toString() const;
    static QString keyName(const metaT::Enum type);
//#endif

    QDataStream & toStream(QDataStream& stream) const;
    QDataStream & fromStream(QDataStream& stream);

    friend bool operator==(const MetaData &c1, const MetaData &c2);
    friend bool operator<(const MetaData &c1, const MetaData &c2);

private:
    template<class T> void SetMetaFast(const metaT::Enum type, const T &value) {
        if(listInfos.contains(type))
            ReplaceMeta(type,value);
        else
            AddMeta(type,value);
    }

    template<class T> void ReplaceMeta(const metaT::Enum type, const T &value) {
        memcpy(listInfos[type],&value,sizeof(T));
    }

    template<class T> void AddMeta(const metaT::Enum type, const T &value) {
        T *copy = new T(value);
//        if(type<metaT::COMPLEX_START)
            listInfos.insert(type,copy);
//        else
//            listComplexInfos.insert(type, QPair<int,void*>(sizeof(value) ,copy) );
    }


    QMap<metaT::Enum,void*>listInfos;
//    QMap<metaT::Enum, QPair<int, void*> >listComplexInfos;
    quint32 metaId;
    MetaType::Enum metaType;

    static quint32 nextId;
};


Q_DECLARE_METATYPE(MetaData);

QDataStream & operator<< (QDataStream& stream, const MetaData& info);
QDataStream & operator>> (QDataStream& stream, MetaData& info);



#endif // METADATA_H
