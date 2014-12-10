//////////////////////////////////////////////////////////////////////////////
// config.cpp
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
// Copyright (C) 2008 Lubos Lunak <l.lunak@kde.org>
//
// Based on the Quartz configuration module,
//     Copyright (c) 2001 Karol Szwed <gallium@kde.org>
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

#include "breezeconfig.h"
#include "../breezesettings.h"

#include <QTextStream>
#include <QDBusConnection>
#include <QDBusMessage>

#include <KLocalizedString>
#include <KPluginFactory>

//_______________________________________________________________________
//* plugin definition
/**
 * this is the old style/KDE4 plugin declaration.
 * it is used in breeze-settings
 */
extern "C"
{
    Q_DECL_EXPORT QObject* allocate_config( KConfig*, QWidget* parent )
    { return ( new Breeze::Config( parent ) ); }
}

/** this is the new style/KF5 plugin declaration, used internally by KWin */
K_PLUGIN_FACTORY_WITH_JSON(
    BreezeConfigPlugin,
    "config.json",
    registerPlugin<Breeze::ConfigurationModule>(QStringLiteral("kcmodule"));
)
#include "breezeconfig.moc"

namespace Breeze
{

    // create new configuration
    QObject *Config::create(QWidget *parentWidget, QObject *, const QList<QVariant> &)
    { return new Config(parentWidget); }

    //_______________________________________________________________________
    Config::Config(QWidget* parent ):
        QObject( parent )
    {

        // configuration
        m_configuration = KSharedConfig::openConfig( QStringLiteral( "breezerc" ) );

        // create new configuration widget and add to layout, if any
        m_configWidget = new ConfigWidget( parent );
        if( parent && parent->layout() ) parent->layout()->addWidget( m_configWidget );
        else m_configWidget->show();

        load();
        connect( m_configWidget, SIGNAL(changed(bool)), SLOT(updateChanged()) );
    }

    //_______________________________________________________________________
    Config::~Config()
    { delete m_configWidget; }

    //_______________________________________________________________________
    void Config::load( void )
    {

        // load standard configuration
        InternalSettingsPtr internalSettings( new InternalSettings() );

        internalSettings->load();
        loadInternalSettings( internalSettings );

        updateChanged();

    }

    //_______________________________________________________________________
    void Config::updateChanged( void )
    {

        bool modified( false );

        // exceptions
        if( m_configWidget->isChanged() ) modified = true;

        // emit relevant signals
        if( modified ) emit changed();
        emit changed( modified );

    }

    //_______________________________________________________________________
    void Config::save( void )
    {

        // create configuration from group
        InternalSettingsPtr internalSettings( new InternalSettings() );
        internalSettings->load();

        // save config widget
        m_configWidget->setInternalSettings( internalSettings );
        m_configWidget->save();

        // save configuration
        internalSettings->save();

        // sync configuration
        m_configuration->sync();

        QDBusMessage message( QDBusMessage::createSignal( QStringLiteral( "/BreezeWindeco" ),  QStringLiteral( "org.kde.Breeze.Style" ), QStringLiteral( "reparseConfiguration") ) );
        QDBusConnection::sessionBus().send(message);

    }

    //_______________________________________________________________________
    void Config::defaults( void )
    {

        // install default configuration
        InternalSettingsPtr internalSettings( new InternalSettings() );
        internalSettings->setDefaults();
        loadInternalSettings( internalSettings );

        updateChanged();

    }

    //_______________________________________________________________________
    void Config::loadInternalSettings( InternalSettingsPtr configuration )
    {

        m_configWidget->setInternalSettings( configuration );
        m_configWidget->load();

    }

    //_______________________________________________________________________
    ConfigurationModule::ConfigurationModule(QWidget *parent, const QVariantList &args):
        KCModule(parent, args)
    {
        setLayout(new QVBoxLayout(this));
        m_config = new Config( this );
    }

    //_______________________________________________________________________
    void ConfigurationModule::defaults()
    {
        m_config->defaults();
        KCModule::defaults();
    }

    //_______________________________________________________________________
    void ConfigurationModule::load()
    {
        m_config->load();
        KCModule::load();
    }

    //_______________________________________________________________________
    void ConfigurationModule::save()
    {
        m_config->save();
        KCModule::save();
    }

}
