#ifndef breezescrollbar_datah
#define breezescrollbar_datah

//////////////////////////////////////////////////////////////////////////////
// breezescrollbardata.h
// data container for QScrollBar animations
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "breezesliderdata.h"

namespace Breeze
{

    //! scrollbar data
    class ScrollBarData: public SliderData
    {

        Q_OBJECT
        Q_PROPERTY( qreal addLineOpacity READ addLineOpacity WRITE setAddLineOpacity )
        Q_PROPERTY( qreal subLineOpacity READ subLineOpacity WRITE setSubLineOpacity )

        public:

        //! constructor
        ScrollBarData( QObject* parent, QWidget* target, int );

        //! destructor
        virtual ~ScrollBarData( void )
        {}

        //! event filter
        virtual bool eventFilter( QObject*, QEvent* );

        //! needed to avoid warning about virtual function being hidden
        using SliderData::animation;
        using SliderData::opacity;

        //! return animation for a given subcontrol
        virtual const Animation::Pointer& animation( QStyle::SubControl ) const;

        //! return default opacity for a given subcontrol
        virtual qreal opacity( QStyle::SubControl ) const;

        //! return default opacity for a given subcontrol
        virtual bool isHovered( QStyle::SubControl control ) const
        {
            switch( control )
            {
                case QStyle::SC_ScrollBarAddLine: return addLineArrowHovered();
                case QStyle::SC_ScrollBarSubLine: return subLineArrowHovered();
                default: return false;
            }


        }

        //! subControlRect
        virtual QRect subControlRect( QStyle::SubControl control ) const
        {
            switch( control )
            {
                case QStyle::SC_ScrollBarAddLine: return _addLineData._rect;
                case QStyle::SC_ScrollBarSubLine: return _subLineData._rect;
                default: return QRect();
            }
        }


        //! subcontrol rect
        virtual void setSubControlRect( QStyle::SubControl control, const QRect& rect )
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

        //! duration
        virtual void setDuration( int duration )
        {
            SliderData::setDuration( duration );
            addLineAnimation().data()->setDuration( duration );
            subLineAnimation().data()->setDuration( duration );
        }

        //! addLine opacity
        virtual void setAddLineOpacity( qreal value )
        {
            value = digitize( value );
            if( _addLineData._opacity == value ) return;
            _addLineData._opacity = value;
            setDirty();
        }

        //! addLine opacity
        virtual qreal addLineOpacity( void ) const
        { return _addLineData._opacity; }

        //! subLine opacity
        virtual void setSubLineOpacity( qreal value )
        {
            value = digitize( value );
            if( _subLineData._opacity == value ) return;
            _subLineData._opacity = value;
            setDirty();
        }

        //! subLine opacity
        virtual qreal subLineOpacity( void ) const
        { return _subLineData._opacity; }

        //! mouse position
        QPoint position( void ) const
        { return _position; }

        protected Q_SLOTS:

        //! clear addLineRect
        void clearAddLineRect( void )
        {
            if( addLineAnimation().data()->direction() == Animation::Backward )
            { _addLineData._rect = QRect(); }
        }

        //! clear subLineRect
        void clearSubLineRect( void )
        {
            if( subLineAnimation().data()->direction() == Animation::Backward )
            { _subLineData._rect = QRect(); }
        }

        protected:

        //! hoverMoveEvent
        virtual void hoverMoveEvent( QObject*, QEvent* );

        //! hoverMoveEvent
        virtual void hoverLeaveEvent( QObject*, QEvent* );

        //!@name hover flags
        //@{

        virtual bool addLineArrowHovered( void ) const
        { return _addLineData._hovered; }

        virtual void setAddLineArrowHovered( bool value )
        { _addLineData._hovered = value; }

        virtual bool subLineArrowHovered( void ) const
        { return _subLineData._hovered; }

        virtual void setSubLineArrowHovered( bool value )
        { _subLineData._hovered = value; }

        //@}

        //! update add line arrow
        virtual void updateAddLineArrow( QStyle::SubControl );

        //! update sub line arrow
        virtual void updateSubLineArrow( QStyle::SubControl );

        //!@name timelines
        //@{

        virtual const Animation::Pointer& addLineAnimation( void ) const
        { return _addLineData._animation; }

        virtual const Animation::Pointer& subLineAnimation( void ) const
        { return _subLineData._animation; }

        private:

        //! stores arrow data
        class Data
        {

          public:

          //! constructor
          Data( void ):
            _hovered( false ),
            _opacity( AnimationData::OpacityInvalid )
          {}

          //! true if hovered
          bool _hovered;

          //! animation
          Animation::Pointer _animation;

          //! opacity
          qreal _opacity;

          //! rect
          QRect _rect;

        };


        //! add line data (down arrow)
        Data _addLineData;

        //! subtract line data (up arrow)
        Data _subLineData;

        //! mouse position
        QPoint _position;

    };

}

#endif
