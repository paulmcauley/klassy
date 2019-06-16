#ifndef breezestackedwidgetengine_h
#define breezestackedwidgetengine_h

//////////////////////////////////////////////////////////////////////////////
// breezestackedwidgetengine.h
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
#include "breezestackedwidgetdata.h"

namespace Breeze
{

    //* used for simple widgets
    class StackedWidgetEngine: public BaseEngine
    {

        Q_OBJECT

        public:

        //* constructor
        explicit StackedWidgetEngine( QObject* parent ):
            BaseEngine( parent )
        {}

        //* register widget
        bool registerWidget( QStackedWidget* );

        //* duration
        void setEnabled( bool value ) override
        {
            BaseEngine::setEnabled( value );
            _data.setEnabled( value );
        }

        //* duration
        void setDuration( int value ) override
        {
            BaseEngine::setDuration( value );
            _data.setDuration( value );
        }

        public Q_SLOTS:

        //* remove widget from map
        bool unregisterWidget( QObject* object ) override
        { return _data.unregisterWidget( object ); }

        private:

        //* maps
        DataMap<StackedWidgetData> _data;

    };

}

#endif
