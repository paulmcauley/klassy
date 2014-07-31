//////////////////////////////////////////////////////////////////////////////
// breezescrollbardata.cpp
// data container for QTabBar animations
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

#include "breezescrollbardata.h"
#include "breezescrollbardata.moc"

#include <QHoverEvent>
#include <QScrollBar>
#include <QStyleOptionSlider>

Q_GUI_EXPORT QStyleOptionSlider qt_qscrollbarStyleOption(QScrollBar*);

namespace Breeze
{

    //______________________________________________
    ScrollBarData::ScrollBarData( QObject* parent, QWidget* target, int duration ):
        SliderData( parent, target, duration ),
        _position( -1, -1 )
    {

        target->installEventFilter( this );

        _addLineData._animation = new Animation( duration, this );
        _subLineData._animation = new Animation( duration, this );

        connect( addLineAnimation().data(), SIGNAL(finished()), SLOT(clearAddLineRect()) );
        connect( subLineAnimation().data(), SIGNAL(finished()), SLOT(clearSubLineRect()) );

        // setup animation
        setupAnimation( addLineAnimation(), "addLineOpacity" );
        setupAnimation( subLineAnimation(), "subLineOpacity" );

    }

    //______________________________________________
    bool ScrollBarData::eventFilter( QObject* object, QEvent* event )
    {

        if( object != target().data() )
        { return SliderData::eventFilter( object, event ); }

        // check event type
        switch( event->type() )
        {

            case QEvent::HoverEnter:
            case QEvent::HoverMove:
            hoverMoveEvent( object, event );
            break;

            case QEvent::HoverLeave:
            hoverLeaveEvent( object, event );
            break;

            default: break;

        }

        return SliderData::eventFilter( object, event );

    }

    //______________________________________________
    const Animation::Pointer& ScrollBarData::animation( QStyle::SubControl subcontrol ) const
    {
        switch( subcontrol )
        {
            default:
            case QStyle::SC_ScrollBarSlider:
            return animation();

            case QStyle::SC_ScrollBarAddLine:
            return addLineAnimation();

            case QStyle::SC_ScrollBarSubLine:
            return subLineAnimation();
        }

    }

    //______________________________________________
    qreal ScrollBarData::opacity( QStyle::SubControl subcontrol ) const
    {
        switch( subcontrol )
        {
            default:
            case QStyle::SC_ScrollBarSlider:
            return opacity();

            case QStyle::SC_ScrollBarAddLine:
            return addLineOpacity();

            case QStyle::SC_ScrollBarSubLine:
            return subLineOpacity();
        }

    }

    //______________________________________________
    void ScrollBarData::hoverMoveEvent(  QObject* object, QEvent* event )
    {

        // try cast object to scrollbar
        QScrollBar* scrollBar( qobject_cast<QScrollBar*>( object ) );
        if( !scrollBar || scrollBar->isSliderDown() ) return;

        // retrieve scrollbar option
        QStyleOptionSlider opt( qt_qscrollbarStyleOption( scrollBar ) );

        // cast event
        QHoverEvent *hoverEvent = static_cast<QHoverEvent*>(event);
        QStyle::SubControl hoverControl = scrollBar->style()->hitTestComplexControl(QStyle::CC_ScrollBar, &opt, hoverEvent->pos(), scrollBar);

        // update hover state
        updateAddLineArrow( hoverControl );
        updateSubLineArrow( hoverControl );

        // store position
        _position = hoverEvent->pos();

    }


    //______________________________________________
    void ScrollBarData::hoverLeaveEvent(  QObject*, QEvent* )
    {

        // reset hover state
        updateSubLineArrow( QStyle::SC_None );
        updateAddLineArrow( QStyle::SC_None );

        // reset mouse position
        _position = QPoint( -1, -1 );
    }

    //_____________________________________________________________________
    void ScrollBarData::updateSubLineArrow( QStyle::SubControl hoverControl )
    {
        if( hoverControl == QStyle::SC_ScrollBarSubLine )
        {

            if( !subLineArrowHovered() )
            {
                setSubLineArrowHovered( true );
                if( enabled() )
                {
                    subLineAnimation().data()->setDirection( Animation::Forward );
                    if( !subLineAnimation().data()->isRunning() ) subLineAnimation().data()->start();
                } else setDirty();
             }

        } else {

            if( subLineArrowHovered() )
            {
                setSubLineArrowHovered( false );
                if( enabled() )
                {
                    subLineAnimation().data()->setDirection( Animation::Backward );
                    if( !subLineAnimation().data()->isRunning() ) subLineAnimation().data()->start();
                } else setDirty();
            }

        }
    }

    //_____________________________________________________________________
    void ScrollBarData::updateAddLineArrow( QStyle::SubControl hoverControl )
    {
        if( hoverControl == QStyle::SC_ScrollBarAddLine )
        {

            if( !addLineArrowHovered() )
            {
                setAddLineArrowHovered( true );
                if( enabled() )
                {
                    addLineAnimation().data()->setDirection( Animation::Forward );
                    if( !addLineAnimation().data()->isRunning() ) addLineAnimation().data()->start();
                } else setDirty();
            }

        } else {

            if( addLineArrowHovered() )
            {
                setAddLineArrowHovered( false );
                if( enabled() )
                {
                    addLineAnimation().data()->setDirection( Animation::Backward );
                    if( !addLineAnimation().data()->isRunning() ) addLineAnimation().data()->start();
                } else setDirty();
            }

        }
    }

}
