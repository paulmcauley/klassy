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

#include <QAbstractItemView>
#include <QDial>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QProgressBar>
#include <QScrollBar>
#include <QSpinBox>
#include <QTextEdit>
#include <QToolButton>

namespace Breeze
{

    //____________________________________________________________
    Animations::Animations( QObject* parent ):
        QObject( parent )
    {
        _widgetEnabilityEngine = new WidgetStateEngine( this );
        _busyIndicatorEngine = new BusyIndicatorEngine( this );
        _comboBoxEngine = new WidgetStateEngine( this );

        registerEngine( _widgetStateEngine = new WidgetStateEngine( this ) );
        registerEngine( _lineEditEngine = new WidgetStateEngine( this ) );
        registerEngine( _scrollBarEngine = new ScrollBarEngine( this ) );
        registerEngine( _sliderEngine = new SliderEngine( this ) );
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
            _widgetEnabilityEngine->setEnabled( animationsEnabled &&  StyleConfigData::genericAnimationsEnabled() );
            _widgetStateEngine->setEnabled( animationsEnabled &&  StyleConfigData::genericAnimationsEnabled() );
            _comboBoxEngine->setEnabled( animationsEnabled &&  StyleConfigData::genericAnimationsEnabled() );
            _lineEditEngine->setEnabled( animationsEnabled &&  StyleConfigData::genericAnimationsEnabled() );
            _scrollBarEngine->setEnabled( animationsEnabled &&  StyleConfigData::genericAnimationsEnabled() );
            _sliderEngine->setEnabled( animationsEnabled &&  StyleConfigData::genericAnimationsEnabled() );

            // busy indicator
            _busyIndicatorEngine->setEnabled( StyleConfigData::progressBarAnimated() );

        }


        {

            // durations
            _widgetEnabilityEngine->setDuration( StyleConfigData::genericAnimationsDuration() );
            _widgetStateEngine->setDuration( StyleConfigData::genericAnimationsDuration() );
            _comboBoxEngine->setDuration( StyleConfigData::genericAnimationsDuration() );
            _lineEditEngine->setDuration( StyleConfigData::genericAnimationsDuration() );
            _scrollBarEngine->setDuration( StyleConfigData::genericAnimationsDuration() );
            _sliderEngine->setDuration( StyleConfigData::genericAnimationsDuration() );

            // busy indicator
            _busyIndicatorEngine->setDuration( StyleConfigData::progressBarBusyStepDuration() );

        }

    }

    //____________________________________________________________
    void Animations::registerWidget( QWidget* widget ) const
    {

        if( !widget ) return;

        // check against noAnimations propery
        QVariant propertyValue( widget->property( PropertyNames::noAnimations ) );
        if( propertyValue.isValid() && propertyValue.toBool() ) return;

        // all widgets are registered to the enability engine.
        _widgetEnabilityEngine->registerWidget( widget, AnimationEnable );

        // install animation timers
        // for optimization, one should put with most used widgets here first
        if( qobject_cast<QToolButton*>(widget) )
        {


        } else if( qobject_cast<QAbstractButton*>(widget) ) {

            _widgetStateEngine->registerWidget( widget, AnimationHover|AnimationFocus );

        } else if( qobject_cast<QDial*>(widget) ) {

            _widgetStateEngine->registerWidget( widget, AnimationHover|AnimationFocus );

        }

        // groupboxes
        else if( QGroupBox* groupBox = qobject_cast<QGroupBox*>( widget ) )
        {
            if( groupBox->isCheckable() )
            { _widgetStateEngine->registerWidget( widget, AnimationHover|AnimationFocus ); }
        }

        // scrollbar
        else if( qobject_cast<QScrollBar*>( widget ) ) { _scrollBarEngine->registerWidget( widget ); }

        // slider
        else if( qobject_cast<QSlider*>( widget ) ) { _sliderEngine->registerWidget( widget ); }

        // progress bar
        else if( qobject_cast<QProgressBar*>( widget ) ) { _busyIndicatorEngine->registerWidget( widget ); }

        // combo box
        else if( qobject_cast<QComboBox*>( widget ) ) {
            _comboBoxEngine->registerWidget( widget, AnimationHover );
            _lineEditEngine->registerWidget( widget, AnimationHover|AnimationFocus );

        }

        // spinbox
        else if( qobject_cast<QSpinBox*>( widget ) ) {
            // _spinBoxEngine->registerWidget( widget );
            _lineEditEngine->registerWidget( widget, AnimationHover|AnimationFocus );
        }

        // editor
        else if( qobject_cast<QLineEdit*>( widget ) ) { _lineEditEngine->registerWidget( widget, AnimationHover|AnimationFocus ); }
        else if( qobject_cast<QTextEdit*>( widget ) ) { _lineEditEngine->registerWidget( widget, AnimationHover|AnimationFocus ); }

        // lists
        else if( qobject_cast<QAbstractItemView*>( widget ) || widget->inherits("Q3ListView") )
        { _lineEditEngine->registerWidget( widget, AnimationHover|AnimationFocus ); }

        // scrollarea
        else if( QAbstractScrollArea* scrollArea = qobject_cast<QAbstractScrollArea*>( widget ) ) {

            if( scrollArea->frameShadow() == QFrame::Sunken && (widget->focusPolicy()&Qt::StrongFocus) )
            { _lineEditEngine->registerWidget( widget, AnimationHover|AnimationFocus ); }

        }

        return;

    }

    //____________________________________________________________
    void Animations::unregisterWidget( QWidget* widget ) const
    {

        if( !widget ) return;

        _widgetEnabilityEngine->unregisterWidget( widget );
        _busyIndicatorEngine->registerWidget( widget );

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
