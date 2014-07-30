//////////////////////////////////////////////////////////////////////////////
// breezespinboxdata.cpp
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

#include "breezespinboxdata.h"
#include "breezespinboxdata.moc"

namespace Breeze
{

    //________________________________________________
    SpinBoxData::SpinBoxData( QObject* parent, QWidget* target, int duration ):
        AnimationData( parent, target )
    {
        _upArrowData._animation = new Animation( duration, this );
        _downArrowData._animation = new Animation( duration, this );
        setupAnimation( upArrowAnimation(), "upArrowOpacity" );
        setupAnimation( downArrowAnimation(), "downArrowOpacity" );
    }

    //______________________________________________
    bool SpinBoxData::Data::updateState( bool value )
    {
        if( _state == value ) return false;
        else {

            _state = value;
            _animation.data()->setDirection( _state ? Animation::Forward : Animation::Backward );
            if( !_animation.data()->isRunning() ) _animation.data()->start();
            return true;

        }
    }

}
