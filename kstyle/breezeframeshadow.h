#ifndef breezeframeshadow_h
#define breezeframeshadow_h

//////////////////////////////////////////////////////////////////////////////
// breezeframeshadow.h
// handle frames' shadows and rounded corners
// -------------------
//
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Largely inspired from skulpture widget style
// Copyright (c) 2007-2009 Christoph Feck <christoph@maxiom.de>
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
#include "breezeaddeventfilter.h"
#include "breezehelper.h"

#include <QEvent>
#include <QObject>
#include <QSet>

#include <QWidget>
#include <QPaintEvent>
#include <KColorScheme>

namespace Breeze
{

    //* shadow manager
    class FrameShadowFactory: public QObject
    {

        Q_OBJECT

        public:

        //* constructor
        explicit FrameShadowFactory( QObject* parent ):
        QObject( parent )
        {}

        //* destructor
        virtual ~FrameShadowFactory( void )
        {}

        //* register widget
        bool registerWidget( QWidget*, Helper& );

        //* unregister
        void unregisterWidget( QWidget* );

        //* true if widget is registered
        bool isRegistered( const QWidget* widget ) const
        { return _registeredWidgets.contains( widget ); }

        //* event filter
        virtual bool eventFilter( QObject*, QEvent*);

        //* update state
        void updateState( const QWidget*, bool focus, bool hover, qreal opacity, AnimationMode ) const;

        protected:

        //* install shadows on given widget
        void installShadows( QWidget*, Helper& );

        //* remove shadows from widget
        void removeShadows( QWidget* );

        //* update shadows geometry
        void updateShadowsGeometry( QObject* ) const;

        //* raise shadows
        void raiseShadows( QObject* ) const;

        //* update shadows
        void update( QObject* ) const;

        //* install shadow on given side
        void installShadow( QWidget*, Helper&, ShadowArea ) const;

        protected Q_SLOTS:

        //* triggered by object destruction
        void widgetDestroyed( QObject* );

        private:

        //* needed to block ChildAdded events when creating shadows
        AddEventFilter _addEventFilter;

        //* set of registered widgets
        QSet<const QObject*> _registeredWidgets;

    };

    //* frame shadow
    /** this allows the shadow to be painted over the widgets viewport */
    class FrameShadowBase: public QWidget
    {

        Q_OBJECT

        public:

        //* constructor
        explicit FrameShadowBase( ShadowArea area ):
            _area( area )
        {}

        //* destructor
        virtual ~FrameShadowBase( void )
        {}

        //* shadow area
        void setShadowArea(ShadowArea area)
        { _area = area; }

        //* shadow area
        const ShadowArea& shadowArea() const
        { return _area; }

        //* update geometry
        virtual void updateGeometry( void ) = 0;

        //* update state
        virtual void updateState( bool, bool, qreal, AnimationMode )
        {}

        protected:

        //* event handler
        virtual bool event(QEvent *e);

        //* initialization
        virtual void init();

        //* return viewport associated to parent widget
        virtual QWidget* viewport( void ) const;

        private:

        //* shadow area
        ShadowArea _area;

    };

    //* frame shadow
    /** this allows the shadow to be painted over the widgets viewport */
    class SunkenFrameShadow : public FrameShadowBase
    {
        Q_OBJECT

        public:

        //* constructor
        SunkenFrameShadow( ShadowArea area, Helper& helper ):
            FrameShadowBase( area ),
            _helper( helper ),
            _hasFocus( false ),
            _mouseOver( false ),
            _opacity( -1 ),
            _mode( AnimationNone )
        { init(); }


        //* destructor
        virtual ~SunkenFrameShadow()
        {}

        //* update geometry
        virtual void updateGeometry( void );

        //* update state
        void updateState( bool focus, bool hover, qreal opacity, AnimationMode );

        protected:

        //* painting
        virtual void paintEvent(QPaintEvent *);

        private:

        //* shadow sizes
        enum
        {
            ShadowSizeTop = 3,
            ShadowSizeBottom = 3,
            ShadowSizeLeft = 3,
            ShadowSizeRight = 3
        };

        //* helper
        Helper& _helper;

        //*@name widget state
        //@{
        bool _hasFocus;
        bool _mouseOver;
        qreal _opacity;
        AnimationMode _mode;
    };
}

#endif
