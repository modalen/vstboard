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

#include "metadata.h"

QString MetaData::keyName(const metaT::Enum type)
{
    if(type<metaT::INT_END)
        return metaT::INTNames[type-metaT::INT_BEGIN-1];
    if(type<metaT::FLOAT_END)
        return metaT::FLOATNames[type-metaT::FLOAT_BEGIN-1];
    if(type<metaT::BOOL_END)
        return metaT::BOOLNames[type-metaT::BOOL_BEGIN-1];
    if(type<metaT::STRING_END)
        return metaT::STRINGNames[type-metaT::STRING_BEGIN-1];
    if(type<metaT::OTHER_END)
        return metaT::OTHERNames[type-metaT::OTHER_BEGIN-1];
    return "nd";
}

//#ifndef QT_NO_DEBUG
QString MetaData::toString() const {
    QString str;

    QMap<MetaInfos::Enum,void*>::const_iterator i = listInfos.constBegin();
    while(i!=listInfos.constEnd()) {

        QString name = keyName(i.key());
        if(name.isEmpty())
            str+= QString("unknownProp(%1):").arg(i.key());
        else
            str+= QString("%1:").arg(name);

        if(i.key()<MetaInfos::INT_END) {
            str += QString::number(*static_cast<int*>(i.value()));
        } else if(i.key()<MetaInfos::FLOAT_END) {
            str += QString::number(*static_cast<float*>(i.value()));
        } else if(i.key()<MetaInfos::BOOL_END) {
            str.append(*static_cast<bool*>(i.value()));
        } else if(i.key()<MetaInfos::STRING_END) {
            str.append(*static_cast<QString*>(i.value()));
        } else if(i.key()<MetaInfos::OTHER_END) {
            str += QString("%1:%2")
                .arg(static_cast<QPointF*>(i.value())->x())
                .arg(static_cast<QPointF*>(i.value())->y());
        }
        str+=" / ";
        ++i;
    }
    return str;
}
//#endif

MetaData & MetaData::operator =(const MetaData &c) {
    if (this == &c)
        return *this;

    qDeleteAll(listInfos);
    listInfos.clear();

    QMap<metaT::Enum,void*>::const_iterator i = c.listInfos.constBegin();
    while(i!=c.listInfos.constEnd()) {
        if(i.key()<metaT::INT_END)
            AddMeta(i.key(),*static_cast<int*>(i.value()));
        else if(i.key()<metaT::FLOAT_END)
            AddMeta(i.key(),*static_cast<float*>(i.value()));
        else if(i.key()<metaT::BOOL_END)
            AddMeta(i.key(),*static_cast<bool*>(i.value()));
        else if(i.key()<metaT::STRING_END)
            AddMeta(i.key(),*static_cast<QString*>(i.value()));
        else if(i.key()<metaT::OTHER_END)
            AddMeta(i.key(),*static_cast<QPointF*>(i.value()));
        ++i;
    }
    return *this;
}

QDataStream & MetaData::toStream(QDataStream& stream) const
{
    stream << static_cast<quint16>(listInfos.size());

    QMap<metaT::Enum,void*>::const_iterator i = listInfos.constBegin();
    while(i!=listInfos.constEnd()) {

        stream << static_cast<quint16>(i.key());

        if(i.key()<metaT::INT_END)
            stream << *static_cast<int*>(i.value());
        else if(i.key()<metaT::FLOAT_END)
            stream << *static_cast<float*>(i.value());
        else if(i.key()<metaT::BOOL_END)
            stream << *static_cast<bool*>(i.value());
        else if(i.key()<metaT::STRING_END)
            stream << *static_cast<QString*>(i.value());
        else if(i.key()<metaT::OTHER_END)
            stream << *static_cast<QPointF*>(i.value());

        ++i;
    }
    return stream;
}

QDataStream & MetaData::fromStream(QDataStream& stream)
{
    quint16 size;
    stream >> size;
    for(int i=0; i<size; ++i) {

        quint16 key;
        stream >> key;

        if(key<metaT::INT_END) {
            int i;
            stream >> i;
            SetMeta( static_cast<metaT::Enum>(key), i);
        } else if(key<metaT::FLOAT_END) {
            float f;
            stream >> f;
            SetMeta(static_cast<metaT::Enum>(key), f);
        } else if(key<metaT::BOOL_END) {
            bool b;
            stream >> b;
            SetMeta(static_cast<metaT::Enum>(key), b);
        } else if(key<metaT::STRING_END) {
            QString s;
            stream >> s;
            SetMeta(static_cast<metaT::Enum>(key), s);
        } else if(key<metaT::OTHER_END) {
            QPointF p;
            stream >> p;
            SetMeta(static_cast<metaT::Enum>(key), p);
        }
    }
    return stream;
}

QDataStream & operator<< (QDataStream& s, const MetaData& data) { return data.toStream(s); }
QDataStream & operator>> (QDataStream& s, MetaData& data) { return data.fromStream(s); }

bool operator==(const MetaData &c1, const MetaData &c2)
{
    if(c1.listInfos.size()!=c2.listInfos.size())
        return false;

    QMap<metaT::Enum,void*>::const_iterator i = c1.listInfos.constBegin();
    while(i!=c1.listInfos.constEnd()) {
        if(i.key() > metaT::INT_BEGIN && i.key() < metaT::INT_END)
            if( *(int*)i.value() != *(int*)c2.listInfos.value(i.key()) )
                return false;
        if(i.key() > metaT::FLOAT_BEGIN && i.key() < metaT::FLOAT_END)
            if( *(float*)i.value() != *(float*)c2.listInfos.value(i.key()) )
                return false;
        if(i.key() > metaT::BOOL_BEGIN && i.key() < metaT::BOOL_END)
            if( *(bool*)i.value() != *(bool*)c2.listInfos.value(i.key()) )
                return false;
        if(i.key() > metaT::STRING_BEGIN && i.key() < metaT::STRING_END)
            if( *(QString*)i.value() != *(QString*)c2.listInfos.value(i.key()) )
                return false;
        if(i.key()==metaT::Position)
            if( *(QPointF*)i.value() != *(QPointF*)c2.listInfos.value(i.key()) )
                return false;
        if(i.key()==metaT::EditorSize)
            if( *(QSize*)i.value() != *(QSize*)c2.listInfos.value(i.key()) )
                return false;
        if(i.key()==metaT::EditorPosition)
            if( *(QPoint*)i.value() != *(QPoint*)c2.listInfos.value(i.key()) )
                return false;
        ++i;
    }
    return true;
}
//bool operator<(const MetaData &c1, const MetaData &c2) { if(c1.listInfos < c2.listInfos; }
