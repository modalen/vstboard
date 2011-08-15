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

#include "connectablepinview.h"
#include "connectables/objectfactory.h"

using namespace View;

ConnectablePinView::ConnectablePinView(const MetaInfo &info, float angle, QGraphicsItem * parent, ViewConfig *config) :
    PinView(info,angle,parent,config),
    value(0),
    overload(0)
{
    setGeometry(0,0,50,15);
    setMinimumSize(50,15);
    setMaximumSize(50,15);

    outline = new QGraphicsRectItem(geometry(), this);
    highlight = new QGraphicsRectItem(geometry(), this);
    highlight->setVisible(false);

    rectVu = new QGraphicsRectItem(this);
    outline->setBrush(Qt::NoBrush);

    textItem = new QGraphicsSimpleTextItem(this);
    textItem->moveBy(2,1);
    textItem->setZValue(1);

    switch(Meta(MetaInfos::Media).toInt()) {
    case MediaTypes::Audio :
        colorGroupId=ColorGroups::AudioPin;
        break;
    case MediaTypes::Midi :
        colorGroupId=ColorGroups::MidiPin;
        rectVu->setRect(rect());
        break;
    case MediaTypes::Parameter :
        colorGroupId=ColorGroups::ParameterPin;
        break;
    default :
        colorGroupId=ColorGroups::ND;
        break;
    }

    outline->setBrush( config->GetColor(colorGroupId, Colors::Background) );
    vuColor = config->GetColor(colorGroupId, Colors::VuMeter);
    rectVu->setBrush( vuColor );
    highlight->setBrush( config->GetColor(ColorGroups::Object, Colors::HighlightBackground) );
    textItem->setBrush(  config->GetColor(ColorGroups::Object, Colors::Text) );

    if(Meta(MetaInfos::displayedText).isValid())
        textItem->setText(Meta(MetaInfos::displayedText).toString());
    else
        textItem->setText(Name());
}

void ConnectablePinView::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId==colorGroupId && colorId==Colors::Background) {
        outline->setBrush(color);
        return;
    }
    if(groupId==colorGroupId && colorId==Colors::VuMeter) {
        vuColor=color;
        if(Meta(MetaInfos::Media).toInt() != MediaTypes::Midi) {
            rectVu->setBrush(color);
        }
        return;
    }
    if(groupId==ColorGroups::Object && colorId==Colors::HighlightBackground) {
        highlight->setBrush(color);
        return;
    }

    if(groupId==ColorGroups::Object && colorId==Colors::Text) {
        textItem->setBrush(color);
        return;
    }
}

void ConnectablePinView::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    ResetOveload();
    PinView::mousePressEvent(event);
}

void ConnectablePinView::UpdateModelIndex(const MetaInfo &info)
{
    *(MetaInfo*)this = info;

    QString newName = Name();
    if(Meta(MetaInfos::displayedText).isValid())
        newName = Meta(MetaInfos::displayedText).toString();

    if(newName!=textItem->text())
        textItem->setText(newName);

    if(Meta(MetaInfos::Media).toInt() == MediaTypes::Parameter) {
        value = Meta(MetaInfos::Value).toFloat();
        float newVu = geometry().width() * value;
        rectVu->setRect(0,0, newVu, geometry().height());
    } else {
        float newVal = Meta(MetaInfos::Value).toFloat();
        value = std::max(value,newVal);
    }
}

void ConnectablePinView::ResetOveload()
{
    overload=0;
    rectVu->setBrush(vuColor);
}

void ConnectablePinView::updateVu()
{
    if(value<.0f)
        return;

    if(Meta(MetaInfos::Media).toInt()==MediaTypes::Audio) {
        value-=.05f;

        if(value>1.0f) {
            if(overload==0) {
                rectVu->setBrush(Qt::red);
                overload=500;
            }
            value=1.0f;
        }

        if(overload>0) {
            overload--;
            if(overload==0) {
                ResetOveload();
            }
        }

        float newVu=.0f;
        if(value<.0f) {
            value=-1.0f;
        } else {
            newVu = geometry().width() * value;
        }
        if(newVu<0.0) {
            LOG("updateVu <0"<<newVu);
            newVu=0.0f;
        }

        rectVu->setRect(0,0, newVu, geometry().height());
    }

    if(Meta(MetaInfos::Media).toInt()== MediaTypes::Midi) {
        value-=.1f;
        if(value<.0f) {
            value=-1.0f;
            rectVu->setBrush(Qt::NoBrush);
            return;
        }
        QColor c = vuColor;

        if(value<0.7)
            c.setAlphaF( value/0.7 );

        rectVu->setBrush(c);
    }
}

void ConnectablePinView::ValueChanged(float newVal)
{
    if(value==newVal)
        return;
    if(newVal>1.0f) newVal=1.0f;
    if(newVal<0.0f) newVal=0.0f;

    SetMeta(MetaInfos::Value,newVal);
//    model->setData(pinIndex,newVal,UserRoles::value);
}

void ConnectablePinView::wheelEvent ( QGraphicsSceneWheelEvent * event )
{
    if(Meta(MetaInfos::Media).toInt() != MediaTypes::Parameter) {
        QGraphicsWidget::wheelEvent(event);
        return;
    }

    event->accept();

    float increm=1.0f;
    if(event->delta()<0)
        increm=-1.0f;

    increm*=Meta(MetaInfos::StepSize).toFloat();

    if(event->modifiers() & Qt::ControlModifier)
        increm/=10.0f;

    ValueChanged( Meta(MetaInfos::Value).toFloat() + increm);

}

void ConnectablePinView::keyPressEvent ( QKeyEvent * event )
{
    if(Meta(MetaInfos::Media).toInt() != MediaTypes::Parameter) {
        QGraphicsWidget::keyPressEvent(event);
        return;
    }

    float increm=0.0f;

    switch(event->key()) {
        case Qt::Key_Left:
            increm=-1.0f;
            break;
        case Qt::Key_Right:
            increm=1.0f;
            break;
        default :
            QGraphicsWidget::keyPressEvent(event);
            return;
    }

    event->accept();

    increm*=Meta(MetaInfos::StepSize).toFloat();

    if(event->modifiers() & Qt::ControlModifier)
        increm/=10.0f;

    ValueChanged( Meta(MetaInfos::Value).toFloat() + increm);
}
