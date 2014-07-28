//////////////////////////////////////////////////////////////////////////////
// breezeanimations.cpp
// container for all animation engines
// -------------------
//
// Copyright (c) 2006, 2007 Riccardo Iaconelli <riccardo@kde.org>
// Copyright (c) 2006, 2007 Casper Boemann <cbr@boemann.dk>
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

#include "breezeanimations.h"
#include "breezeanimations.moc"
#include "breezepropertynames.h"
#include "breezestyleconfigdata.h"

#include <QScrollBar>

namespace Breeze
{

    //____________________________________________________________
    Animations::Animations( QObject* parent ):
        QObject( parent )
    {
        registerEngine( _scrollBarEngine = new ScrollBarEngine( this ) );
    }

    //____________________________________________________________
    void Animations::setupEngines( void )
    {

        // animation steps
        AnimationData::setSteps( StyleConfigData::animationSteps() );

        {
            // default enability, duration and maxFrame
            bool animationsEnabled( StyleConfigData::animationsEnabled() );

            // enability
            _scrollBarEngine->setEnabled( animationsEnabled &&  StyleConfigData::genericAnimationsEnabled() );

        }


        {

            // durations
            _scrollBarEngine->setDuration( StyleConfigData::genericAnimationsDuration() );

        }

    }

    //____________________________________________________________
    void Animations::registerWidget( QWidget* widget ) const
    {

        if( !widget ) return;

        // check against noAnimations propery
        QVariant propertyValue( widget->property( PropertyNames::noAnimations ) );
        if( propertyValue.isValid() && propertyValue.toBool() ) return;

        // scrollbar
        if( qobject_cast<QScrollBar*>( widget ) ) { _scrollBarEngine->registerWidget( widget ); }

        return;

    }

    //____________________________________________________________
    void Animations::unregisterWidget( QWidget* widget ) const
    {

        if( !widget ) return;

        // the following allows some optimization of widget unregistration
        // it assumes that a widget can be registered atmost in one of the
        // engines stored in the list.
        foreach( const BaseEngine::Pointer& engine, _engines )
        { if( engine && engine.data()->unregisterWidget( widget ) ) break; }

    }

    //_______________________________________________________________
    void Animations::unregisterEngine( QObject* object )
    {
        int index( _engines.indexOf( qobject_cast<BaseEngine*>(object) ) );
        if( index >= 0 ) _engines.removeAt( index );
    }

    //_______________________________________________________________
    void Animations::registerEngine( BaseEngine* engine )
    {
        _engines.push_back( engine );
        connect( engine, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterEngine(QObject*)) );
    }

}
