/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breezedecorationsettingsprovider.h"

#include "decorationexceptionlist.h"

#include <KWindowInfo>

#include <QRegularExpression>
#include <QTextStream>

namespace Breeze
{

SettingsProvider *SettingsProvider::s_self = nullptr;

//__________________________________________________________________
SettingsProvider::SettingsProvider()
    : m_config(KSharedConfig::openConfig(QStringLiteral("klassyrc")))
{
    reconfigure();
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
    if (!m_defaultSettings) {
        m_defaultSettings = InternalSettingsPtr(new InternalSettings());
        m_defaultSettings->setCurrentGroup(QStringLiteral("Windeco"));
    }

    m_defaultSettings->load();

    DecorationExceptionList exceptions;
    exceptions.readConfig(m_config);
    m_exceptions = exceptions.getDefault();
    m_exceptions.append(exceptions.get());
}

//__________________________________________________________________
InternalSettingsPtr SettingsProvider::internalSettings(const QMainWindow *mw) const
{
    if (!mw)
        return m_defaultSettings;

    foreach (auto internalSettings, m_exceptions) {
        // discard disabled exceptions
        if (!internalSettings->enabled())
            continue;

        // discard exceptions with empty exception pattern
        if (internalSettings->exceptionProgramNamePattern().isEmpty())
            continue;

        // check matching
        QRegularExpression rx(internalSettings->exceptionProgramNamePattern());
        if (rx.match(qAppName()).hasMatch()) {
            return internalSettings;
        }
    }

    return m_defaultSettings;
}

}
