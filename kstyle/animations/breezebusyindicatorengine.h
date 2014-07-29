#ifndef breezebusyindicatorengine_h
#define breezebusyindicatorengine_h

//////////////////////////////////////////////////////////////////////////////
// breezebusyindicatorengine.h
// handle progress bar busy indicator
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
#include "breezebusyindicatordata.h"
#include "breezedatamap.h"

#include <QBasicTimer>
#include <QSet>
#include <QWidget>
#include <QTimerEvent>

namespace Breeze
{

    //! handles progress bar animations
    class BusyIndicatorEngine: public BaseEngine
    {

        Q_OBJECT

        public:

        //! constructor
        explicit BusyIndicatorEngine( QObject* object ):
            BaseEngine( object )
        {}

        //! destructor
        virtual ~BusyIndicatorEngine( void )
        {}

        //!@name accessors
        //@{

        //! true if widget is animated
        virtual bool isAnimated( const QObject* );

        //! animation opacity
        virtual int value( const QObject* object )
        { return isAnimated( object ) ? data( object ).data()->value():0; }

        //@}

        //!@name modifiers
        //@{

        //! register progressbar
        virtual bool registerWidget( QObject* );

        //! duration
        virtual void setDuration( int );

        //! set object as animated
        virtual void setAnimated( const QObject*, bool );

        //@}

        public Q_SLOTS:

        //! remove widget from map
        virtual bool unregisterWidget( QObject* object )
        { return _data.unregisterWidget( object ); }

        protected:

        //! timer event
        virtual void timerEvent( QTimerEvent* );

        //! returns data associated to widget
        DataMap<BusyIndicatorData>::Value data( const QObject* );

        private:

        //! map widgets to progressbar data
        DataMap<BusyIndicatorData> _data;

        //! timer
        QBasicTimer _timer;

    };

}

#endif
