#ifndef breezemdiwindow_data_h
#define breezemdiwindow_data_h

//////////////////////////////////////////////////////////////////////////////
// breezemdiwindowdata.h
// mdi window data container for window titlebar buttons
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

    //! handles mdiwindow arrows hover
    class MdiWindowData: public AnimationData
    {

        Q_OBJECT

        //! declare opacity property
        Q_PROPERTY( qreal currentOpacity READ currentOpacity WRITE setCurrentOpacity )
        Q_PROPERTY( qreal previousOpacity READ previousOpacity WRITE setPreviousOpacity )

        public:

        //! constructor
        MdiWindowData( QObject*, QWidget*, int );

        //! destructor
        virtual ~MdiWindowData( void )
        {}

        //! animation state
        virtual bool updateState( int primitive, bool value );

        //! animation state
        virtual bool isAnimated( int primitive ) const
        {
            return(
                ( primitive == _currentData._primitive && currentAnimation().data()->isRunning() ) ||
                ( primitive == _previousData._primitive && previousAnimation().data()->isRunning() ) );
        }

        //! opacity
        virtual qreal opacity( int primitive ) const
        {
            if( primitive == _currentData._primitive ) return currentOpacity();
            else if( primitive == _previousData._primitive ) return previousOpacity();
            else return OpacityInvalid;
        }

        //! duration
        virtual void setDuration( int duration )
        {
            currentAnimation().data()->setDuration( duration );
            previousAnimation().data()->setDuration( duration );
        }

        //!@name current animation
        //@{

        //! opacity
        qreal currentOpacity( void ) const
        { return _currentData._opacity; }

        //! opacity
        void setCurrentOpacity( qreal value )
        {
            value = digitize( value );
            if( _currentData._opacity == value ) return;
            _currentData._opacity = value;
            setDirty();
        }

        //! animation
        Animation::Pointer currentAnimation( void ) const
        { return _currentData._animation; }

        //@}
        //!@name previous animation
        //@{

        //! opacity
        qreal previousOpacity( void ) const
        { return _previousData._opacity; }

        //! opacity
        void setPreviousOpacity( qreal value )
        {
            value = digitize( value );
            if( _previousData._opacity == value ) return;
            _previousData._opacity = value;
            setDirty();
        }

        //! animation
        Animation::Pointer previousAnimation( void ) const
        { return _previousData._animation; }

        //@}

        private:

        //! container for needed animation data
        class Data
        {

            public:

            //! default constructor
            Data( void ):
                _primitive( 0 ),
                _opacity(0)
                {}

            //! subcontrol
            bool updateSubControl( int );

            //! subcontrol
            int _primitive;

            //! animation
            Animation::Pointer _animation;

            //! opacity
            qreal _opacity;

        };

        //! current data
        Data _currentData;

        //! previous data
        Data _previousData;

    };


}

#endif
