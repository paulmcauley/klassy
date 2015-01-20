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
#include "breezeconfigwidget.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include <QDBusConnection>
#include <QDBusMessage>

namespace Breeze
{

    //_______________________________________________________________________
    ConfigurationModule::ConfigurationModule(QWidget *parent, const QVariantList &args):
        KCModule(parent, args)
    {
        setLayout(new QVBoxLayout(this));
        layout()->addWidget( m_config = new ConfigWidget( this ) );
        connect(m_config, static_cast<void (ConfigWidget::*)(bool)>(&ConfigWidget::changed), this, static_cast<void (KCModule::*)(bool)>(&KCModule::changed));
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
        QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
        QDBusConnection::sessionBus().send(message);
    }

}
