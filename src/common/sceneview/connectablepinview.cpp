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

using namespace View;

ConnectablePinView::ConnectablePinView(const MetaData &info, float angle, QGraphicsItem * parent, ViewConfig *config) :
    PinView(info,angle,parent,config),
    value(0),
    overload(0),
    textItem(0),
    rectVu(0),
    colorGroupId(ColorGroups::ND),
    vuColor(Qt::gray)
{
    outline = new QGraphicsRectItem(rect(), this);
    highlight = new QGraphicsRectItem(rect(), this);
    highlight->setVisible(false);

    rectVu = new QGraphicsRectItem(this);
    outline->setBrush(Qt::NoBrush);

    textItem = new QGraphicsSimpleTextItem(this);
    textItem->moveBy(2,1);
    textItem->setZValue(1);

    switch(MetaData::GetMetaData<MediaTypes::Enum>(metaT::Media)) {
    case MediaTypes::Audio :
        colorGroupId=ColorGroups::AudioPin;
        break;
    case MediaTypes::Midi :
        colorGroupId=ColorGroups::MidiPin;
        rectVu->setRect(rect());
        break;
    case MediaTypes::Parameter :
        defaultCursor = Qt::PointingHandCursor;
        setCursor(defaultCursor);
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

    setGeometry(0,0,50,15);
    setMinimumSize(50,15);
    setMaximumSize(75,15);

    UpdateModelIndex(info);

    setFocusPolicy(Qt::WheelFocus);
}

void ConnectablePinView::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId==colorGroupId && colorId==Colors::Background) {
        outline->setBrush(color);
        return;
    }
    if(groupId==colorGroupId && colorId==Colors::VuMeter) {
        vuColor=color;
        if(MetaData::GetMetaData<MediaTypes::Enum>(metaT::Media) != MediaTypes::Midi) {
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

void ConnectablePinView::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    PinView::resizeEvent(event);

    if(outline) {
        QRectF r = outline->rect();
        r.setSize(event->newSize());
        outline->setRect(r);
    }
    if(highlight){
        QRectF r = highlight->rect();
        r.setSize(event->newSize());
        highlight->setRect(r);
    }
    if(rectVu) {
        QRectF r = rectVu->rect();
        if(MetaData::GetMetaData<MediaTypes::Enum>(metaT::Media) == MediaTypes::Midi) {
            r.setSize(event->newSize());
            rectVu->setRect(r);
        } else if(value>.0f) {
            r.setWidth(value*event->newSize().width());
            r.setHeight(event->newSize().height());
            rectVu->setRect(r);
        }
    }

    UpdateCablesPosition();
}

void ConnectablePinView::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    ResetOveload();
    PinView::mousePressEvent(event);
}

void ConnectablePinView::UpdateModelIndex(const MetaData &info)
{
    *(MetaData*)this = info;

    QString newName = Name();
    if(MetaData::data.GetMeta<QString*>(metaT::displayedText)!=0)
        newName = MetaData::GetMetaData<QString>(metaT::displayedText);

    if(newName!=textItem->text()) {
        textItem->setText(newName);

        if(textItem->boundingRect().width()>size().width())
            setMinimumWidth( textItem->boundingRect().width()+10 );
    }

    if(MetaData::GetMetaData<MediaTypes::Enum>(metaT::Media) == MediaTypes::Parameter) {
        value = MetaData::GetMetaData<float>(metaT::Value);
        float newVu = size().width() * value;
        rectVu->setRect(0,0, newVu, size().height());
    } else {
        float newVal = MetaData::GetMetaData<float>(metaT::Value);
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

    if(MetaData::GetMetaData<MediaTypes::Enum>(metaT::Media)==MediaTypes::Audio) {
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
            newVu = size().width() * value;
        }
        if(newVu<0.0f) {
            LOG("updateVu <0"<<newVu);
            newVu=0.0f;
        }

        rectVu->setRect(0,0, newVu, size().height());
    }

    if(MetaData::GetMetaData<MediaTypes::Enum>(metaT::Media)== MediaTypes::Midi) {
        value-=.1f;
        if(value<.0f) {
            value=-1.0f;
            rectVu->setBrush(Qt::NoBrush);
            return;
        }
        QColor c = vuColor;

        if(value<0.7f)
            c.setAlphaF( value/0.7f );

        rectVu->setBrush(c);
    }
}

void ConnectablePinView::ValueChanged(float newVal)
{
    if(value==newVal)
        return;
    if(newVal>1.0f) newVal=1.0f;
    if(newVal<0.0f) newVal=0.0f;

    MetaData::SetMeta(metaT::Value,newVal);
//    model->setData(pinIndex,newVal,UserRoles::value);
}

void ConnectablePinView::wheelEvent ( QGraphicsSceneWheelEvent * event )
{
    if(MetaData::GetMetaData<MediaTypes::Enum>(metaT::Media) != MediaTypes::Parameter) {
        QGraphicsWidget::wheelEvent(event);
        return;
    }

    event->accept();

    float increm=1.0f;
    if(event->delta()<0)
        increm=-1.0f;

    increm*=MetaData::GetMetaData<float>(metaT::StepSize);

    if(event->modifiers() & Qt::ControlModifier)
        increm/=10.0f;

    ValueChanged( MetaData::GetMetaData<float>(metaT::Value) + increm);

}

void ConnectablePinView::keyPressEvent ( QKeyEvent * event )
{
    if(MetaData::GetMetaData<MediaTypes::Enum>(metaT::Media) != MediaTypes::Parameter) {
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

    increm*=MetaData::GetMetaData<float>(metaT::StepSize);

    if(event->modifiers() & Qt::ControlModifier)
        increm/=10.0f;

    ValueChanged( MetaData::GetMetaData<float>(metaT::Value) + increm);
}
