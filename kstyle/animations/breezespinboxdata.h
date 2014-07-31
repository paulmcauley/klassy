#ifndef breezespinbox_data_h
#define breezespinbox_data_h

//////////////////////////////////////////////////////////////////////////////
// breezespinboxdata.h
// spinbox data container for up/down arrow hover (mouse-over) animations
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

#include "breezeanimationdata.h"

#include <QStyle>

namespace Breeze
{

    //! handles spinbox arrows hover
    class SpinBoxData: public AnimationData
    {

        Q_OBJECT

        //! declare opacity property
        Q_PROPERTY( qreal upArrowOpacity READ upArrowOpacity WRITE setUpArrowOpacity )
        Q_PROPERTY( qreal downArrowOpacity READ downArrowOpacity WRITE setDownArrowOpacity )

        public:

        //! constructor
        SpinBoxData( QObject*, QWidget*, int );

        //! destructor
        virtual ~SpinBoxData( void )
        {}

        //! animation state
        virtual bool updateState( QStyle::SubControl subControl, bool value )
        {
            if( subControl == QStyle::SC_SpinBoxUp ) return _upArrowData.updateState( value );
            else if( subControl == QStyle::SC_SpinBoxDown ) return _downArrowData.updateState( value );
            else return false;
        }

        //! animation state
        virtual bool isAnimated( QStyle::SubControl subControl ) const
        {
            return(
                ( subControl == QStyle::SC_SpinBoxUp && upArrowAnimation().data()->isRunning() ) ||
                ( subControl == QStyle::SC_SpinBoxDown && downArrowAnimation().data()->isRunning() ) );
        }

        //! opacity
        virtual qreal opacity( QStyle::SubControl subControl ) const
        {
            if( subControl == QStyle::SC_SpinBoxUp ) return upArrowOpacity();
            else if( subControl == QStyle::SC_SpinBoxDown ) return downArrowOpacity();
            else return OpacityInvalid;
        }

        //! duration
        virtual void setDuration( int duration )
        {
            upArrowAnimation().data()->setDuration( duration );
            downArrowAnimation().data()->setDuration( duration );
        }

        //!@name up arrow animation
        //@{

        //! opacity
        qreal upArrowOpacity( void ) const
        { return _upArrowData._opacity; }

        //! opacity
        void setUpArrowOpacity( qreal value )
        {
            value = digitize( value );
            if( _upArrowData._opacity == value ) return;
            _upArrowData._opacity = value;
            setDirty();
        }

        //! animation
        Animation::Pointer upArrowAnimation( void ) const
        { return _upArrowData._animation; }

        //@}

        //!@name down arrow animation
        //@{

        //! opacity
        qreal downArrowOpacity( void ) const
        { return _downArrowData._opacity; }

        //! opacity
        void setDownArrowOpacity( qreal value )
        {
            value = digitize( value );
            if( _downArrowData._opacity == value ) return;
            _downArrowData._opacity = value;
            setDirty();
        }

        //! animation
        Animation::Pointer downArrowAnimation( void ) const
        { return _downArrowData._animation; }

        //@}

        private:

        //! container for needed animation data
        class Data
        {

            public:

            //! default constructor
            Data( void ):
                _state( false ),
                _opacity(0)
                {}

            //! state
            bool updateState( bool );

            //! arrow state
            bool _state;

            //! animation
            Animation::Pointer _animation;

            //! opacity
            qreal _opacity;

        };

        //! up arrow data
        Data _upArrowData;

        //! down arrow data
        Data _downArrowData;

    };


}

#endif
