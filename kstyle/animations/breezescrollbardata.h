#ifndef breezescrollbar_data_h
#define breezescrollbar_data_h

/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

#include "breezewidgetstatedata.h"

#include <QStyle>

namespace Breeze
{

    //* scrollbar data
    class ScrollBarData: public WidgetStateData
    {

        Q_OBJECT
        Q_PROPERTY( qreal addLineOpacity READ addLineOpacity WRITE setAddLineOpacity )
        Q_PROPERTY( qreal subLineOpacity READ subLineOpacity WRITE setSubLineOpacity )
        Q_PROPERTY( qreal grooveOpacity READ grooveOpacity WRITE setGrooveOpacity )

        public:

        //* constructor
        ScrollBarData( QObject* parent, QWidget* target, int );

        //* event filter
        bool eventFilter( QObject*, QEvent* ) override;

        //* needed to avoid warning about virtual function being hidden
        using WidgetStateData::animation;
        using WidgetStateData::opacity;

        //* return animation for a given subcontrol
        const Animation::Pointer& animation( QStyle::SubControl ) const;

        //* return default opacity for a given subcontrol
        qreal opacity( QStyle::SubControl ) const;

        //* return default opacity for a given subcontrol
        bool isHovered( QStyle::SubControl control ) const
        {
            switch( control )
            {
                case QStyle::SC_ScrollBarAddLine: return addLineArrowHovered();
                case QStyle::SC_ScrollBarSubLine: return subLineArrowHovered();
                case QStyle::SC_ScrollBarGroove: return grooveHovered();
                default: return false;
            }


        }

        //* subControlRect
        QRect subControlRect( QStyle::SubControl control ) const
        {
            switch( control )
            {
                case QStyle::SC_ScrollBarAddLine: return _addLineData._rect;
                case QStyle::SC_ScrollBarSubLine: return _subLineData._rect;
                default: return QRect();
            }
        }


        //* subcontrol rect
        void setSubControlRect( QStyle::SubControl control, const QRect& rect )
        {
            switch( control )
            {
                case QStyle::SC_ScrollBarAddLine:
                _addLineData._rect = rect;
                break;

                case QStyle::SC_ScrollBarSubLine:
                _subLineData._rect = rect;
                break;

                default: break;
            }
        }

        //* duration
        void setDuration( int duration ) override
        {
            WidgetStateData::setDuration( duration );
            addLineAnimation().data()->setDuration( duration );
            subLineAnimation().data()->setDuration( duration );
            grooveAnimation().data()->setDuration( duration );
        }

        //* addLine opacity
        void setAddLineOpacity( qreal value )
        {
            value = digitize( value );
            if( _addLineData._opacity == value ) return;
            _addLineData._opacity = value;
            setDirty();
        }

        //* addLine opacity
        qreal addLineOpacity() const
        { return _addLineData._opacity; }

        //* subLine opacity
        void setSubLineOpacity( qreal value )
        {
            value = digitize( value );
            if( _subLineData._opacity == value ) return;
            _subLineData._opacity = value;
            setDirty();
        }

        //* subLine opacity
        qreal subLineOpacity() const
        { return _subLineData._opacity; }

        //* groove opacity
        void setGrooveOpacity( qreal value )
        {
            value = digitize( value );
            if( _grooveData._opacity == value ) return;
            _grooveData._opacity = value;
            setDirty();
        }

        //* groove opacity
        qreal grooveOpacity() const
        { return _grooveData._opacity; }

        //* mouse position
        QPoint position() const
        { return _position; }

        protected Q_SLOTS:

        //* clear addLineRect
        void clearAddLineRect()
        {
            if( addLineAnimation().data()->direction() == Animation::Backward )
            { _addLineData._rect = QRect(); }
        }

        //* clear subLineRect
        void clearSubLineRect()
        {
            if( subLineAnimation().data()->direction() == Animation::Backward )
            { _subLineData._rect = QRect(); }
        }

        private:

        //* hoverMoveEvent
        void hoverMoveEvent( QObject*, QEvent* );

        //* hoverMoveEvent
        void hoverLeaveEvent( QObject*, QEvent* );

        //*@name hover flags
        //@{

        bool addLineArrowHovered() const
        { return _addLineData._hovered; }

        void setAddLineArrowHovered( bool value )
        { _addLineData._hovered = value; }

        bool subLineArrowHovered() const
        { return _subLineData._hovered; }

        void setSubLineArrowHovered( bool value )
        { _subLineData._hovered = value; }

        bool grooveHovered() const
        { return _grooveData._hovered; }

        void setGrooveHovered( bool value )
        { _grooveData._hovered = value; }

        //@}

        //* update add line arrow
        void updateAddLineArrow( QStyle::SubControl );

        //* update sub line arrow
        void updateSubLineArrow( QStyle::SubControl );

        //*@name timelines
        //@{

        const Animation::Pointer& addLineAnimation() const
        { return _addLineData._animation; }

        const Animation::Pointer& subLineAnimation() const
        { return _subLineData._animation; }

        const Animation::Pointer& grooveAnimation() const
        { return _grooveData._animation; }

        //* stores sub control data
        class Data
        {

          public:

          //* constructor
          Data():
            _hovered( false ),
            _opacity( AnimationData::OpacityInvalid )
          {}

          //* true if hovered
          bool _hovered;

          //* animation
          Animation::Pointer _animation;

          //* opacity
          qreal _opacity;

          //* rect
          QRect _rect;

        };


        //* add line data (down arrow)
        Data _addLineData;

        //* subtract line data (up arrow)
        Data _subLineData;

        //* groove data
        Data _grooveData;

        //* mouse position
        QPoint _position;

    };

}

#endif
