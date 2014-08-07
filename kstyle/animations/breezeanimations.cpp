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
#include <QComboBox>
#include <QDial>
#include <QGroupBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QProgressBar>
#include <QScrollBar>
#include <QSpinBox>
#include <QTextEdit>
#include <QToolBox>
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
        _spinBoxEngine = new SpinBoxEngine( this );
        _toolBoxEngine = new ToolBoxEngine( this );

        registerEngine( _headerViewEngine = new HeaderViewEngine( this ) );
        registerEngine( _widgetStateEngine = new WidgetStateEngine( this ) );
        registerEngine( _lineEditEngine = new WidgetStateEngine( this ) );
        registerEngine( _scrollBarEngine = new ScrollBarEngine( this ) );
        registerEngine( _sliderEngine = new SliderEngine( this ) );
        registerEngine( _tabBarEngine = new TabBarEngine( this ) );
        registerEngine( _dialEngine = new DialEngine( this ) );

    }

    //____________________________________________________________
    void Animations::setupEngines( void )
    {

        // animation steps
        AnimationData::setSteps( StyleConfigData::animationSteps() );

        const bool animationsEnabled( StyleConfigData::animationsEnabled() );
        const int animationsDuration( StyleConfigData::animationsDuration() );

        _widgetEnabilityEngine->setEnabled( animationsEnabled );
        _comboBoxEngine->setEnabled( animationsEnabled );
        _spinBoxEngine->setEnabled( animationsEnabled );
        _toolBoxEngine->setEnabled( animationsEnabled );

        _widgetEnabilityEngine->setDuration( animationsDuration );
        _comboBoxEngine->setDuration( animationsDuration );
        _spinBoxEngine->setDuration( animationsDuration );
        _toolBoxEngine->setDuration( animationsDuration );

        // registered engines
        foreach( const BaseEngine::Pointer& engine, _engines )
        {
            engine.data()->setEnabled( animationsEnabled );
            engine.data()->setDuration( animationsDuration );
        }

        // busy indicator
        _busyIndicatorEngine->setEnabled( StyleConfigData::progressBarAnimated() );
        _busyIndicatorEngine->setDuration( StyleConfigData::progressBarBusyStepDuration() );

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

        // buttons
        if( qobject_cast<QToolButton*>(widget) )
        {

            _widgetStateEngine->registerWidget( widget, AnimationHover|AnimationFocus );

        } else if( qobject_cast<QAbstractButton*>(widget) ) {

            // register to toolbox engine if needed
            if( qobject_cast<QToolBox*>( widget->parent() ) )
            { _toolBoxEngine->registerWidget( widget ); }

            _widgetStateEngine->registerWidget( widget, AnimationHover|AnimationFocus );

        }

        // groupboxes
        else if( QGroupBox* groupBox = qobject_cast<QGroupBox*>( widget ) )
        {
            if( groupBox->isCheckable() )
            { _widgetStateEngine->registerWidget( widget, AnimationHover|AnimationFocus ); }
        }

        // sliders
        else if( qobject_cast<QScrollBar*>( widget ) ) { _scrollBarEngine->registerWidget( widget ); }
        else if( qobject_cast<QSlider*>( widget ) ) { _sliderEngine->registerWidget( widget ); }
        else if( qobject_cast<QDial*>( widget ) ) { _dialEngine->registerWidget( widget ); }

        // progress bar
        else if( qobject_cast<QProgressBar*>( widget ) ) { _busyIndicatorEngine->registerWidget( widget ); }

        // combo box
        else if( qobject_cast<QComboBox*>( widget ) ) {
            _comboBoxEngine->registerWidget( widget, AnimationHover );
            _lineEditEngine->registerWidget( widget, AnimationHover|AnimationFocus );

        }

        // spinbox
        else if( qobject_cast<QSpinBox*>( widget ) ) {
            _spinBoxEngine->registerWidget( widget );
            _lineEditEngine->registerWidget( widget, AnimationHover|AnimationFocus );
        }

        // editors
        else if( qobject_cast<QLineEdit*>( widget ) ) { _lineEditEngine->registerWidget( widget, AnimationHover|AnimationFocus ); }
        else if( qobject_cast<QTextEdit*>( widget ) ) { _lineEditEngine->registerWidget( widget, AnimationHover|AnimationFocus ); }

        // header views
        // need to come before abstract item view, otherwise is skipped
        else if( qobject_cast<QHeaderView*>( widget ) ) { _headerViewEngine->registerWidget( widget ); }

        // lists
        else if( qobject_cast<QAbstractItemView*>( widget ) || widget->inherits("Q3ListView") )
        { _lineEditEngine->registerWidget( widget, AnimationHover|AnimationFocus ); }

        // tabbar
        else if( qobject_cast<QTabBar*>( widget ) ) { _tabBarEngine->registerWidget( widget ); }

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
        _spinBoxEngine->unregisterWidget( widget );
        _comboBoxEngine->unregisterWidget( widget );
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
