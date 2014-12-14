#ifndef breeze_config_h
#define breeze_config_h

//////////////////////////////////////////////////////////////////////////////
// config.h
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

#include "breezeconfigwidget.h"

#include <KSharedConfig>
#include <KCModule>

namespace Breeze {

    class Configuration;

    // breeze configuration object
    class Config: public QObject
    {

        Q_OBJECT

        public:

        //* constructor
        Config( QWidget* parent );

        //* destructor
        ~Config();

        static QObject *create(QWidget *parentWidget, QObject *parent, const QList<QVariant> &arguments);

        Q_SIGNALS:

        //* emmited whenever configuration is changed
        void changed();

        //* emmited whenever configuration is changed
        void changed( bool );

        public Q_SLOTS:

        //* load configuration
        /** although kconfiggroup argument is not used. It is required by KWin API */
        void load( const KConfigGroup& )
        { load(); }

        //* save configuration
        /** although kconfiggroup argument is not used. It is required by KWin API */
        void save( KConfigGroup& )
        { save(); }

        //* load configuration
        void load( void );

        //* save configuration
        void save( void );

        //* restore defaults
        void defaults( void );

        private Q_SLOTS:

        //* update change state
        void updateChanged( void );

        private:

        //* load configuration
        void loadInternalSettings( InternalSettingsPtr );

        //* user interface
        ConfigWidget* m_configWidget;

        //* kconfiguration object
        KSharedConfig::Ptr m_configuration;

    };

    //* configuration module
    class ConfigurationModule: public KCModule
    {

        Q_OBJECT

        public:
        ConfigurationModule(QWidget *parent, const QVariantList &args);

        public Q_SLOTS:

        void defaults() override;
        void load() override;
        void save() override;

        private:

        //* configuration
        Config* m_config;

    };

} //namespace Breeze

#endif
