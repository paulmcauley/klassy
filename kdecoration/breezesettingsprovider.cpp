/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breezesettingsprovider.h"
#include "decorationexceptionlist.h"
#include "presetsmodel.h"

#include <QRegularExpression>
#include <QTextStream>

namespace Breeze
{

SettingsProvider *SettingsProvider::s_self = nullptr;

//__________________________________________________________________
SettingsProvider::SettingsProvider()
    : m_config(KSharedConfig::openConfig(QStringLiteral("klassyrc")))
{
    m_defaultSettings = InternalSettingsPtr(new InternalSettings());
    m_defaultSettings->setCurrentGroup(QStringLiteral("Windeco"));
}

//__________________________________________________________________
SettingsProvider::~SettingsProvider()
{
    s_self = nullptr;
}

//__________________________________________________________________
SettingsProvider *SettingsProvider::self()
{
    // TODO: this is not thread safe!
    if (!s_self) {
        s_self = new SettingsProvider();
    }

    return s_self;
}

//__________________________________________________________________
void SettingsProvider::reconfigure()
{
    m_defaultSettings->load();

    DecorationExceptionList exceptions;
    exceptions.readConfig(m_config);
    m_exceptions = exceptions.getDefault();
    m_exceptions.append(exceptions.get());
}

//__________________________________________________________________
InternalSettingsPtr SettingsProvider::internalSettings(Decoration *decoration) const
{
    // get the client
    auto client = decoration->client().toStrongRef();
    Q_ASSERT(client);

    foreach (auto internalSettings, m_exceptions) {
        // discard disabled exceptions
        if (!internalSettings->enabled()) {
            continue;
        }

        // discard exceptions with empty exception pattern
        if (internalSettings->exceptionWindowPropertyPattern().isEmpty()) {
            continue;
        }

        /*
        decide which windowPropertyValue is to be compared
        to the regular expression, based on exception type
        */
        QString windowPropertyValue;
        switch (internalSettings->exceptionWindowPropertyType()) {
        case InternalSettings::ExceptionWindowTitle: {
            windowPropertyValue = client->caption();
            break;
        }

        default:
        case InternalSettings::ExceptionWindowClassName: {
            windowPropertyValue = client->windowClass(); // windowClass() available from KDecoration 5.27 onwards
            break;
        }
        }

        // check matching
        QRegularExpression rx(internalSettings->exceptionWindowPropertyPattern());
        if (rx.match(windowPropertyValue).hasMatch()) {
            // load preset if set
            if (!internalSettings->exceptionPreset().isEmpty()) {
                PresetsModel::readPreset(internalSettings.data(), m_config.data(), internalSettings->exceptionPreset());
            }
            return internalSettings;
        }
    }

    return m_defaultSettings;
}
}
