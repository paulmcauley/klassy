//////////////////////////////////////////////////////////////////////////////
// breezemdiwindowdata.cpp
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

#include "breezemdiwindowdata.h"
#include "breezemdiwindowdata.moc"

namespace Breeze
{

    //________________________________________________
    MdiWindowData::MdiWindowData( QObject* parent, QWidget* target, int duration ):
        AnimationData( parent, target )
    {
        _currentData._animation = new Animation( duration, this );
        _previousData._animation = new Animation( duration, this );
        setupAnimation( currentAnimation(), "currentOpacity" );
        setupAnimation( previousAnimation(), "previousOpacity" );

        currentAnimation().data()->setDirection( Animation::Forward );
        previousAnimation().data()->setDirection( Animation::Backward );
    }

    //______________________________________________
    bool MdiWindowData::updateState( int primitive, bool state )
    {

        if( state )
        {

            if( primitive != _currentData._primitive )
            {

                _previousData.updateSubControl( _currentData._primitive );
                _currentData.updateSubControl( primitive );
                return true;

            } else return false;

        } else {

            bool changed( false );
            if( primitive == _currentData._primitive )
            {
                changed |= _currentData.updateSubControl( 0 );
                changed |= _previousData.updateSubControl( primitive );
            }

            return changed;

        }

    }

    //______________________________________________
    bool MdiWindowData::Data::updateSubControl( int value )
    {
        if( _primitive == value ) return false;
        else {

            _primitive = value;
            if( _animation.data()->isRunning() ) _animation.data()->stop();
            if( _primitive != 0 ) _animation.data()->start();
            return true;

        }
    }

}
