#ifndef breezeslider_datah
#define breezeslider_datah

//////////////////////////////////////////////////////////////////////////////
// breezesliderdata.h
// data container for QSlider animations
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

#include "breezegenericdata.h"

#include <QStyle>
#include <QStyleOptionSlider>

namespace Breeze
{

    //! scrollbar data
    class SliderData: public GenericData
    {

        Q_OBJECT

        public:

        //! constructor
        SliderData( QObject* parent, QWidget* target, int duration ):
        GenericData( parent, target, duration ),
        _sliderHovered( false )
        {}

        //! destructor
        virtual ~SliderData( void )
        {}

        //! update state
        virtual bool updateState( bool );

        protected:

        //! hover
        virtual bool sliderHovered( void ) const
        { return _sliderHovered; }

        //! hover
        virtual void setSliderHovered( bool value )
        { _sliderHovered = value; }

        //! update slider
        virtual void updateSlider( QStyle::SubControl );

        private:

        //! true when slider is hovered
        bool _sliderHovered;

    };

}

#endif
