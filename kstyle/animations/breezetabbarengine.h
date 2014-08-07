#ifndef breezetabbarengine_h
#define breezetabbarengine_h

//////////////////////////////////////////////////////////////////////////////
// breezetabbarengine.h
// stores event filters and maps widgets to timelines for animations
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

#include "breeze.h"
#include "breezebaseengine.h"
#include "breezedatamap.h"
#include "breezetabbardata.h"

namespace Breeze
{

    //! stores tabbar hovered action and timeLine
    class TabBarEngine: public BaseEngine
    {

        Q_OBJECT

        public:

        //! constructor
        explicit TabBarEngine( QObject* parent ):
            BaseEngine( parent )
        {}

        //! destructor
        virtual ~TabBarEngine( void )
        {}

        //! register tabbar
        virtual bool registerWidget( QWidget* );

        //! true if widget hover state is changed
        virtual bool updateState( const QObject*, const QPoint&, AnimationMode, bool );

        //! true if widget is animated
        virtual bool isAnimated( const QObject* object, const QPoint& point, AnimationMode );

        //! animation opacity
        virtual qreal opacity( const QObject* object, const QPoint& point, AnimationMode mode )
        { return isAnimated( object, point, mode ) ? data( object, mode ).data()->opacity( point ) : AnimationData::OpacityInvalid; }

        //! enability
        virtual void setEnabled( bool value )
        {
            BaseEngine::setEnabled( value );
            _hoverData.setEnabled( value );
            _focusData.setEnabled( value );
        }

        //! duration
        virtual void setDuration( int value )
        {
            BaseEngine::setDuration( value );
            _hoverData.setDuration( value );
            _focusData.setDuration( value );
        }

        public Q_SLOTS:

        //! remove widget from map
        virtual bool unregisterWidget( QObject* object )
        {
            if( !object ) return false;
            bool found = false;
            if( _hoverData.unregisterWidget( object ) ) found = true;
            if( _focusData.unregisterWidget( object ) ) found = true;
            return found;
        }

        private:

        //! returns data associated to widget
        DataMap<TabBarData>::Value data( const QObject*, AnimationMode );

        //! data map
        DataMap<TabBarData> _hoverData;
        DataMap<TabBarData> _focusData;

    };

}

#endif
