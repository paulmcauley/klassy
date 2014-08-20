
//////////////////////////////////////////////////////////////////////////////
// breezestyleconfig.h
// -------------------
//
// Copyright (C) 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License version 2 as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; see the file COPYING.LIB.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.
//////////////////////////////////////////////////////////////////////////////

#include "breezestyleconfig.h"
#include "breezestyleconfig.moc"

#include "../breeze.h"
#include "breezestyleconfigdata.h"

#include <QDBusMessage>
#include <QDBusConnection>

extern "C"
{
    Q_DECL_EXPORT QWidget* allocate_kstyle_config(QWidget* parent)
    { return new Breeze::StyleConfig(parent); }
}

namespace Breeze
{

    //__________________________________________________________________
    StyleConfig::StyleConfig(QWidget* parent):
        QWidget(parent)
    {
        setupUi(this);

        // connections
        connect( _windowDragMode, SIGNAL(currentIndexChanged(int)), SLOT(windowDragModeChanged(int)) );

        // load setup from configData
        load();

        connect( _toolBarDrawItemSeparator, SIGNAL(toggled(bool)), SLOT(updateChanged()) );
        connect( _sliderDrawTickMarks, SIGNAL(toggled(bool)), SLOT(updateChanged()) );
        connect( _splitterProxyEnabled, SIGNAL(toggled(bool)), SLOT(updateChanged()) );
        connect( _mnemonicsMode, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( _animationsEnabled, SIGNAL(toggled(bool)), SLOT(updateChanged()) );
        connect( _animationsDuration, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );
        connect( _scrollBarAddLineButtons, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( _scrollBarSubLineButtons, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( _lightSource, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( _windowDragMode, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( _useWMMoveResize, SIGNAL(toggled(bool)), SLOT(updateChanged()) );

    }

    //__________________________________________________________________
    void StyleConfig::save( void )
    {
        StyleConfigData::setToolBarDrawItemSeparator( _toolBarDrawItemSeparator->isChecked() );
        StyleConfigData::setSliderDrawTickMarks( _sliderDrawTickMarks->isChecked() );
        StyleConfigData::setSplitterProxyEnabled( _splitterProxyEnabled->isChecked() );
        StyleConfigData::setMnemonicsMode( _mnemonicsMode->currentIndex() );
        StyleConfigData::setScrollBarAddLineButtons( _scrollBarAddLineButtons->currentIndex() );
        StyleConfigData::setScrollBarSubLineButtons( _scrollBarSubLineButtons->currentIndex() );
        StyleConfigData::setLightSource( _lightSource->currentIndex() );
        StyleConfigData::setAnimationsEnabled( _animationsEnabled->isChecked() );
        StyleConfigData::setAnimationsDuration( _animationsDuration->value() );

        StyleConfigData::setUseWMMoveResize( _useWMMoveResize->isChecked() );
        if( _windowDragMode->currentIndex() == 0 )
        {

            StyleConfigData::setWindowDragEnabled( false  );

        } else {

            StyleConfigData::setWindowDragEnabled( true  );
            StyleConfigData::setWindowDragMode( windowDragMode()  );

        }

        StyleConfigData::self()->writeConfig();

        // emit dbus signal
        QDBusMessage message( QDBusMessage::createSignal( QStringLiteral( "/BreezeStyle" ),  QStringLiteral( "org.kde.Breeze.Style" ), QStringLiteral( "reparseConfiguration" ) ) );
        QDBusConnection::sessionBus().send(message);

    }

    //__________________________________________________________________
    void StyleConfig::defaults( void )
    {
        StyleConfigData::self()->setDefaults();
        load();
    }

    //__________________________________________________________________
    void StyleConfig::reset( void )
    {
        StyleConfigData::self()->readConfig();
        load();
    }

    //__________________________________________________________________
    void StyleConfig::updateChanged()
    {

        bool modified( false );

        // check if any value was modified
        if( _toolBarDrawItemSeparator->isChecked() != StyleConfigData::toolBarDrawItemSeparator() ) modified = true;
        else if( _sliderDrawTickMarks->isChecked() != StyleConfigData::sliderDrawTickMarks() ) modified = true;
        else if( _mnemonicsMode->currentIndex() != StyleConfigData::mnemonicsMode() ) modified = true;
        else if( _scrollBarAddLineButtons->currentIndex() != StyleConfigData::scrollBarAddLineButtons() ) modified = true;
        else if( _scrollBarSubLineButtons->currentIndex() != StyleConfigData::scrollBarSubLineButtons() ) modified = true;
        else if( _lightSource->currentIndex() != StyleConfigData::lightSource() ) modified = true;
        else if( _splitterProxyEnabled->isChecked() != StyleConfigData::splitterProxyEnabled() ) modified = true;
        else if( _animationsEnabled->isChecked() != StyleConfigData::animationsEnabled() ) modified = true;
        else if( _animationsDuration->value() != StyleConfigData::animationsDuration() ) modified = true;
        else if( _useWMMoveResize->isChecked() != StyleConfigData::useWMMoveResize() ) modified = true;
        if( !modified )
        {
            switch( _windowDragMode->currentIndex() )
            {
                case 0:
                {
                    if( StyleConfigData::windowDragEnabled() ) modified = true;
                    break;
                }

                case 1:
                case 2:
                default:
                {
                    if( !StyleConfigData::windowDragEnabled() || windowDragMode() != StyleConfigData::windowDragMode() )
                    { modified = true; }
                    break;
                }

            }
        }

        emit changed(modified);

    }

    //__________________________________________________________________
    void StyleConfig::load( void )
    {

        _toolBarDrawItemSeparator->setChecked( StyleConfigData::toolBarDrawItemSeparator() );
        _sliderDrawTickMarks->setChecked( StyleConfigData::sliderDrawTickMarks() );
        _mnemonicsMode->setCurrentIndex( StyleConfigData::mnemonicsMode() );
        _splitterProxyEnabled->setChecked( StyleConfigData::splitterProxyEnabled() );
        _scrollBarAddLineButtons->setCurrentIndex( StyleConfigData::scrollBarAddLineButtons() );
        _scrollBarSubLineButtons->setCurrentIndex( StyleConfigData::scrollBarSubLineButtons() );
        _lightSource->setCurrentIndex( StyleConfigData::lightSource() );
        _animationsEnabled->setChecked( StyleConfigData::animationsEnabled() );
        _animationsDuration->setValue( StyleConfigData::animationsDuration() );
        if( !StyleConfigData::windowDragEnabled() ) _windowDragMode->setCurrentIndex(0);
        else if( StyleConfigData::windowDragMode() == StyleConfigData::WD_MINIMAL ) _windowDragMode->setCurrentIndex(1);
        else _windowDragMode->setCurrentIndex(2);
        _useWMMoveResize->setChecked( StyleConfigData::useWMMoveResize() );

    }

    //__________________________________________________________________
    void StyleConfig::windowDragModeChanged( int value )
    { _useWMMoveResize->setEnabled( value != 0 ); }

    //____________________________________________________________
    int StyleConfig::windowDragMode( void ) const
    {
        switch( _windowDragMode->currentIndex() )
        {
            case 1: return StyleConfigData::WD_MINIMAL;
            case 2: default: return StyleConfigData::WD_FULL;
        }
    }

}
