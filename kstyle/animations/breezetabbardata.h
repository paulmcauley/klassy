#ifndef breezetabbar_data_h
#define breezetabbar_data_h

//////////////////////////////////////////////////////////////////////////////
// breezetabbardata.h
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

#include "breezeanimationdata.h"

#include <QTabBar>

namespace Breeze
{

    //* tabbars
    class TabBarData: public AnimationData
    {

        Q_OBJECT

        //* declare opacity property
        Q_PROPERTY( qreal currentOpacity READ currentOpacity WRITE setCurrentOpacity )
        Q_PROPERTY( qreal previousOpacity READ previousOpacity WRITE setPreviousOpacity )

        public:

        //* constructor
        TabBarData( QObject* parent, QWidget* target, int duration );

        //* destructor
        virtual ~TabBarData( void )
        {}

        //* duration
        void setDuration( int duration )
        {
            currentIndexAnimation().data()->setDuration( duration );
            previousIndexAnimation().data()->setDuration( duration );
        }

        //* update state
        bool updateState( const QPoint&, bool );

        //*@name current index handling
        //@{

        //* current opacity
        virtual qreal currentOpacity( void ) const
        { return _current._opacity; }

        //* current opacity
        virtual void setCurrentOpacity( qreal value )
        {
            if( _current._opacity == value ) return;
            _current._opacity = value;
            setDirty();
        }

        //* current index
        virtual int currentIndex( void ) const
        { return _current._index; }

        //* current index
        virtual void setCurrentIndex( int index )
        { _current._index = index; }

        //* current index animation
        virtual const Animation::Pointer& currentIndexAnimation( void ) const
        { return _current._animation; }

        //@}

        //*@name previous index handling
        //@{

        //* previous opacity
        virtual qreal previousOpacity( void ) const
        { return _previous._opacity; }

        //* previous opacity
        virtual void setPreviousOpacity( qreal value )
        {
            if( _previous._opacity == value ) return;
            _previous._opacity = value;
            setDirty();
        }

        //* previous index
        virtual int previousIndex( void ) const
        { return _previous._index; }

        //* previous index
        virtual void setPreviousIndex( int index )
        { _previous._index = index; }

        //* previous index Animation
        virtual const Animation::Pointer& previousIndexAnimation( void ) const
        { return _previous._animation; }

        //@}

        //* return Animation associated to action at given position, if any
        virtual Animation::Pointer animation( const QPoint& position ) const;

        //* return opacity associated to action at given position, if any
        virtual qreal opacity( const QPoint& position ) const;

        private:

        //* container for needed animation data
        class Data
        {
            public:

            //* default constructor
            Data( void ):
                _opacity(0),
                _index(-1)
            {}

            Animation::Pointer _animation;
            qreal _opacity;
            int _index;
        };

        //* current tab animation data (for hover enter animations)
        Data _current;

        //* previous tab animations data (for hover leave animations)
        Data _previous;

    };

}

#endif
