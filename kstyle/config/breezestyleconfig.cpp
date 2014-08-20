
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
        connect( _windowDragMode, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );

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
        StyleConfigData::setAnimationsEnabled( _animationsEnabled->isChecked() );
        StyleConfigData::setAnimationsDuration( _animationsDuration->value() );
        StyleConfigData::setWindowDragMode( _windowDragMode->currentIndex()  );

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
        else if( _splitterProxyEnabled->isChecked() != StyleConfigData::splitterProxyEnabled() ) modified = true;
        else if( _animationsEnabled->isChecked() != StyleConfigData::animationsEnabled() ) modified = true;
        else if( _animationsDuration->value() != StyleConfigData::animationsDuration() ) modified = true;
        else if( _windowDragMode->currentIndex() != StyleConfigData::windowDragMode() ) modified = true;

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
        _animationsEnabled->setChecked( StyleConfigData::animationsEnabled() );
        _animationsDuration->setValue( StyleConfigData::animationsDuration() );
        _windowDragMode->setCurrentIndex( StyleConfigData::windowDragMode() );

    }

}
