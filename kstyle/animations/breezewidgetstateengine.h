#ifndef breezewidgetstateengine_h
#define breezewidgetstateengine_h

//////////////////////////////////////////////////////////////////////////////
// breezewidgetstateengine.h
// stores event filters and maps widgets to animations
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

#include "breezebaseengine.h"
#include "breezedatamap.h"
#include "breezewidgetstatedata.h"
#include "breezeanimationmodes.h"

namespace Breeze
{

    //! used for simple widgets
    class WidgetStateEngine: public BaseEngine
    {

        Q_OBJECT

        public:

        //! constructor
        explicit WidgetStateEngine( QObject* parent ):
            BaseEngine( parent )
        {}

        //! destructor
        virtual ~WidgetStateEngine( void )
        {}

        //! register widget
        virtual bool registerWidget( QWidget*, AnimationModes );

        //! returns registered widgets
        virtual WidgetList registeredWidgets( AnimationModes ) const;

        using BaseEngine::registeredWidgets;

        //! true if widget hover state is changed
        virtual bool updateState( const QObject*, AnimationMode, bool );

        //! true if widget is animated
        virtual bool isAnimated( const QObject*, AnimationMode );

        //! animation opacity
        virtual qreal opacity( const QObject* object, AnimationMode mode )
        { return isAnimated( object, mode ) ? data( object, mode ).data()->opacity(): AnimationData::OpacityInvalid; }

        //! animation mode
        /*! precedence on focus */
        virtual AnimationMode frameAnimationMode( const QObject* object )
        {
            if( isAnimated( object, AnimationEnable ) ) return AnimationEnable;
            else if( isAnimated( object, AnimationFocus ) ) return AnimationFocus;
            else if( isAnimated( object, AnimationHover ) ) return AnimationHover;
            else return AnimationNone;
        }

        //! animation opacity
        /*! precedence on focus */
        virtual qreal frameOpacity( const QObject* object )
        {
            if( isAnimated( object, AnimationEnable ) ) return data( object, AnimationEnable ).data()->opacity();
            else if( isAnimated( object, AnimationFocus ) ) return data( object, AnimationFocus ).data()->opacity();
            else if( isAnimated( object, AnimationHover ) ) return data( object, AnimationHover ).data()->opacity();
            else return AnimationData::OpacityInvalid;
        }

        //! animation mode
        /*! precedence on mouseOver */
        virtual AnimationMode buttonAnimationMode( const QObject* object )
        {
            if( isAnimated( object, AnimationEnable ) ) return AnimationEnable;
            else if( isAnimated( object, AnimationHover ) ) return AnimationHover;
            else if( isAnimated( object, AnimationFocus ) ) return AnimationFocus;
            else return AnimationNone;
        }

        //! animation opacity
        /*! precedence on mouseOver */
        virtual qreal buttonOpacity( const QObject* object )
        {
            if( isAnimated( object, AnimationEnable ) ) return data( object, AnimationEnable ).data()->opacity();
            else if( isAnimated( object, AnimationHover ) ) return data( object, AnimationHover ).data()->opacity();
            else if( isAnimated( object, AnimationFocus ) ) return data( object, AnimationFocus ).data()->opacity();
            else return AnimationData::OpacityInvalid;
        }

        //! duration
        virtual void setEnabled( bool value )
        {
            BaseEngine::setEnabled( value );
            _hoverData.setEnabled( value );
            _focusData.setEnabled( value );
            _enableData.setEnabled( value );
        }

        //! duration
        virtual void setDuration( int value )
        {
            BaseEngine::setDuration( value );
            _hoverData.setDuration( value );
            _focusData.setDuration( value );
            _enableData.setDuration( value );
        }

        public Q_SLOTS:

        //! remove widget from map
        virtual bool unregisterWidget( QObject* object )
        {
            if( !object ) return false;
            bool found = false;
            if( _hoverData.unregisterWidget( object ) ) found = true;
            if( _focusData.unregisterWidget( object ) ) found = true;
            if( _enableData.unregisterWidget( object ) ) found = true;
            return found;
        }

        protected:

        //! returns data associated to widget
        DataMap<WidgetStateData>::Value data( const QObject*, AnimationMode );

        private:

        //! maps
        DataMap<WidgetStateData> _hoverData;
        DataMap<WidgetStateData> _focusData;
        DataMap<WidgetStateData> _enableData;

    };

}

#endif
