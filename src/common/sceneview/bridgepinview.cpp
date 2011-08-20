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

#include "bridgepinview.h"
#include "objectview.h"

using namespace View;

/*!
  \class View::BridgePinView
  \brief view class for the bridge pins
  */

/*!
  \param angle angle in rad (0=output, pi=input) used by CableView
  \param model pointer to the model item representing the pin
  \param parent pointer to the parent object view
  \param pinInfo description of the pin
  */
BridgePinView::BridgePinView(const MetaData &info, float angle, QGraphicsItem * parent, ViewConfig *config) :
    PinView(info, angle, parent, config),
    value(.0f),
//    valueType(MediaTypes::ND),
    bridgeOutline(0),
    bridgeVuValue(0),
    bridgeHighlight(0)
{
    setGeometry(0,0,PINSIZE,PINSIZE);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMaximumSize(PINSIZE,PINSIZE);
    setMinimumSize(PINSIZE,PINSIZE);


    QPolygonF pol;

    if( (MetaData::data.GetMetaData<Directions::Enum>(metaT::Direction)==Directions::Input && pinAngle<0)
        || (MetaData::data.GetMetaData<Directions::Enum>(metaT::Direction)==Directions::Output && pinAngle>0) ) {
        pol << QPointF(PINSIZE/2,PINSIZE) << QPointF(0,0) << QPointF(PINSIZE,0);
    } else {
        pol << QPointF(PINSIZE/2,0) << QPointF(0,PINSIZE) << QPointF(PINSIZE,PINSIZE);
    }

    bridgeVuValue = new QGraphicsPolygonItem(pol,this);
    bridgeVuValue->setPen(Qt::NoPen);

    bridgeOutline = new QGraphicsPolygonItem(pol,this);
    bridgeOutline->setBrush(Qt::NoBrush);
    bridgeOutline->setPen( config->GetColor(ColorGroups::Bridge,Colors::Lines) );

    bridgeHighlight = new QGraphicsPolygonItem(pol,this);
    bridgeHighlight->setVisible(false);
    bridgeHighlight->setBrush( config->GetColor(ColorGroups::Object, Colors::HighlightBackground) );

    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Bridge,Colors::Background) );
    setPalette( pal );

    UpdateModelIndex(info);
}

void BridgePinView::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId==ColorGroups::Bridge && colorId==Colors::Lines) {
        bridgeOutline->setPen( color );
        return;
    }

    if(groupId==ColorGroups::Bridge && colorId==Colors::Background) {
        QPalette pal(palette());
        pal.setColor(QPalette::Window,color);
        setPalette( pal );
        return;
    }

    if(groupId==ColorGroups::Object && colorId==Colors::HighlightBackground) {
        bridgeHighlight->setBrush( color );
    }
}

const QPointF BridgePinView::pinPos() const
{
    if(pinAngle>0) {
        return QPointF(PINSIZE/2,PINSIZE);
    } else {
        return QPointF(PINSIZE/2,0);
    }
}

void BridgePinView::UpdateModelIndex(const MetaData &info)
{
    *(MetaData*)this=info;

    float newVal=MetaData::data.GetMetaData<float>(metaT::Value);
    value = std::max(value,newVal);
//    valueType = (MediaTypes::Enum)data.GetMetaData<MediaTypes::Enum>(metaT::Media);
}

void BridgePinView::updateVu()
{
    if(value<.0f)
        return;

    value-=.3f;
    if(value<.0f) {
        value=-1.0f;
        bridgeVuValue->setBrush(Qt::NoBrush);
        return;
    }

    QColor c;
    switch(MetaData::data.GetMetaData<int>(metaT::BridgeMedia)) {
        case MediaTypes::Audio:
            c = config->GetColor(ColorGroups::AudioPin, Colors::Background);
            break;
        case MediaTypes::Midi :
            c = config->GetColor(ColorGroups::MidiPin, Colors::Background);
            break;
        case MediaTypes::Parameter :
            c = config->GetColor(ColorGroups::ParameterPin, Colors::Background);
            break;
        default :
            c = Qt::darkGray;
            break;
    }
    bridgeVuValue->setBrush(c);
}
