//////////////////////////////////////////////////////////////////////////////
// breezeconfigurationui.cpp
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

#include "breezeconfigwidget.h"
#include "breezeexceptionlist.h"
#include "breezesettings.h"

#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusMessage>

namespace Breeze
{

    //_________________________________________________________
    ConfigWidget::ConfigWidget( QWidget* parent, const QVariantList &args ):
        KCModule(parent, args),
        m_configuration( KSharedConfig::openConfig( QStringLiteral( "breezerc" ) ) ),
        m_changed( false )
    {

        // configuration
        m_ui.setupUi( this );

        // track ui changes
        connect( m_ui.titleAlignment, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.buttonSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( m_ui.drawBorderOnMaximizedWindows, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.drawSizeGrip, SIGNAL(clicked()), SLOT(updateChanged()) );

        // track exception changes
        connect( m_ui.exceptions, SIGNAL(changed(bool)), SLOT(updateChanged()) );

        // track animations changes
        connect( m_ui.animationsEnabled, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( m_ui.animationsDuration, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );

    }

    //_________________________________________________________
    void ConfigWidget::load( void )
    {

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr( new InternalSettings() );
        m_internalSettings->load();

        // assign to ui
        m_ui.titleAlignment->setCurrentIndex( m_internalSettings->titleAlignment() );
        m_ui.buttonSize->setCurrentIndex( m_internalSettings->buttonSize() );
        m_ui.drawBorderOnMaximizedWindows->setChecked( m_internalSettings->drawBorderOnMaximizedWindows() );
        m_ui.drawSizeGrip->setChecked( m_internalSettings->drawSizeGrip() );
        m_ui.animationsEnabled->setChecked( m_internalSettings->animationsEnabled() );
        m_ui.animationsDuration->setValue( m_internalSettings->animationsDuration() );

        // load exceptions
        ExceptionList exceptions;
        exceptions.readConfig( m_configuration );
        m_ui.exceptions->setExceptions( exceptions.get() );
        setChanged( false );

    }

    //_________________________________________________________
    void ConfigWidget::save( void )
    {

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr( new InternalSettings() );
        m_internalSettings->load();

        // apply modifications from ui
        m_internalSettings->setTitleAlignment( m_ui.titleAlignment->currentIndex() );
        m_internalSettings->setButtonSize( m_ui.buttonSize->currentIndex() );
        m_internalSettings->setDrawBorderOnMaximizedWindows( m_ui.drawBorderOnMaximizedWindows->isChecked() );
        m_internalSettings->setDrawSizeGrip( m_ui.drawSizeGrip->isChecked() );
        m_internalSettings->setAnimationsEnabled( m_ui.animationsEnabled->isChecked() );
        m_internalSettings->setAnimationsDuration( m_ui.animationsDuration->value() );

        // save configuration
        m_internalSettings->save();

        // save standard configuration
        ExceptionList::writeConfig( m_internalSettings.data(), m_configuration.data() );

        // get list of exceptions and write
        InternalSettingsList exceptions( m_ui.exceptions->exceptions() );
        ExceptionList( exceptions ).writeConfig( m_configuration );

        // sync configuration
        m_configuration->sync();
        setChanged( false );

        // needed to tell kwin to reload when running from external kcmshell
        QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
        QDBusConnection::sessionBus().send(message);

    }

    //_________________________________________________________
    void ConfigWidget::defaults( void )
    {

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr( new InternalSettings() );
        m_internalSettings->setDefaults();

        // assign to ui
        m_ui.titleAlignment->setCurrentIndex( m_internalSettings->titleAlignment() );
        m_ui.buttonSize->setCurrentIndex( m_internalSettings->buttonSize() );
        m_ui.drawBorderOnMaximizedWindows->setChecked( m_internalSettings->drawBorderOnMaximizedWindows() );
        m_ui.drawSizeGrip->setChecked( m_internalSettings->drawSizeGrip() );
        m_ui.animationsEnabled->setChecked( m_internalSettings->animationsEnabled() );
        m_ui.animationsDuration->setValue( m_internalSettings->animationsDuration() );

        setChanged( false );

    }

    //_______________________________________________
    void ConfigWidget::updateChanged( void )
    {

        // check configuration
        if( !m_internalSettings ) return;

        // track modifications
        bool modified( false );

        if( m_ui.titleAlignment->currentIndex() != m_internalSettings->titleAlignment() ) modified = true;
        else if( m_ui.buttonSize->currentIndex() != m_internalSettings->buttonSize() ) modified = true;
        else if( m_ui.drawBorderOnMaximizedWindows->isChecked() !=  m_internalSettings->drawBorderOnMaximizedWindows() ) modified = true;
        else if( m_ui.drawSizeGrip->isChecked() !=  m_internalSettings->drawSizeGrip() ) modified = true;

        // exceptions
        else if( m_ui.exceptions->isChanged() ) modified = true;

        // animations
        else if( m_ui.animationsEnabled->isChecked() !=  m_internalSettings->animationsEnabled() ) modified = true;
        else if( m_ui.animationsDuration->value() != m_internalSettings->animationsDuration() ) modified = true;

        setChanged( modified );

    }

    //_______________________________________________
    void ConfigWidget::setChanged( bool value )
    {
        emit changed( value );
    }

}
