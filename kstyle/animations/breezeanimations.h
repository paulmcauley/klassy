#ifndef breezeanimations_h
#define breezeanimations_h

//////////////////////////////////////////////////////////////////////////////
// breezeanimations.h
// container for all animation engines
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

#include "breezebusyindicatorengine.h"
#include "breezescrollbarengine.h"
#include "breezesliderengine.h"
#include "breezewidgetstateengine.h"

#include <QObject>
#include <QList>

namespace Breeze
{

    //! stores engines
    class Animations: public QObject
    {

        Q_OBJECT

        public:

        //! constructor
        explicit Animations( QObject* );

        //! destructor
        virtual ~Animations( void )
        {}

        //! register animations corresponding to given widget, depending on its type.
        void registerWidget( QWidget* widget ) const;

        /*! unregister all animations associated to a widget */
        void unregisterWidget( QWidget* widget ) const;

        //! enability engine
        WidgetStateEngine& widgetEnabilityEngine( void ) const
        { return *_widgetEnabilityEngine; }

        //! abstractButton engine
        WidgetStateEngine& widgetStateEngine( void ) const
        { return *_widgetStateEngine; }

        //! editable combobox arrow hover engine
        WidgetStateEngine& comboBoxEngine( void ) const
        { return *_comboBoxEngine; }

        //! lineEdit engine
        WidgetStateEngine& lineEditEngine( void ) const
        { return *_lineEditEngine; }

        //! busy indicator
        BusyIndicatorEngine& busyIndicatorEngine( void ) const
        { return *_busyIndicatorEngine; }

        //! scrollbar engine
        ScrollBarEngine& scrollBarEngine( void ) const
        { return *_scrollBarEngine; }

        //! slider engine
        SliderEngine& sliderEngine( void ) const
        { return *_sliderEngine; }

        //! setup engines
        void setupEngines( void );

        protected Q_SLOTS:

        //! enregister engine
        void unregisterEngine( QObject* );
        private:

        //! register new engine
        void registerEngine( BaseEngine* engine );

        //! busy indicator
        BusyIndicatorEngine* _busyIndicatorEngine;

        //! widget enability engine
        WidgetStateEngine* _widgetEnabilityEngine;

        //! abstract button engine
        WidgetStateEngine* _widgetStateEngine;

        //! line editor engine
        WidgetStateEngine* _lineEditEngine;

        //! editable combobox arrow hover effect
        WidgetStateEngine* _comboBoxEngine;

        //! scrollbar engine
        ScrollBarEngine* _scrollBarEngine;

        //! slider engine
        SliderEngine* _sliderEngine;

        //! keep list of existing engines
        QList< BaseEngine::Pointer > _engines;

    };

}

#endif
