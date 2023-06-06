/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breezedecorationsettingsprovider.h"
#include "decorationexceptionlist.h"
#include "presetsmodel.h"

#include <KWindowInfo>

#include <QRegularExpression>
#include <QTextStream>

namespace Breeze
{

DecorationSettingsProvider *DecorationSettingsProvider::s_self = nullptr;

//__________________________________________________________________
DecorationSettingsProvider::DecorationSettingsProvider()
    : m_config(KSharedConfig::openConfig(QStringLiteral("klassyrc")))
{
    m_defaultSettings = InternalSettingsPtr(new InternalSettings());
    m_defaultSettings->setCurrentGroup(QStringLiteral("Windeco"));
}

//__________________________________________________________________
DecorationSettingsProvider::~DecorationSettingsProvider()
{
    s_self = nullptr;
}

//__________________________________________________________________
DecorationSettingsProvider *DecorationSettingsProvider::self()
{
    // TODO: this is not thread safe!
    if (!s_self) {
        s_self = new DecorationSettingsProvider();
    }

    return s_self;
}

//__________________________________________________________________
void DecorationSettingsProvider::reconfigure()
{
    m_defaultSettings->load();

    DecorationExceptionList exceptions;
    exceptions.readConfig(m_config);
    m_exceptions = exceptions.getDefault();
    m_exceptions.append(exceptions.get());
}

//__________________________________________________________________
InternalSettingsPtr DecorationSettingsProvider::internalSettings() const
{
    for (const auto &internalSettings : m_exceptions) {
        // discard disabled exceptions
        if (!internalSettings->enabled())
            continue;

        // discard exceptions with empty exception pattern
        if (internalSettings->exceptionProgramNamePattern().isEmpty())
            continue;

        // check matching
        QRegularExpression rx(internalSettings->exceptionProgramNamePattern());
        if (rx.match(qAppName()).hasMatch()) {
            // load window decoration preset if set
            if (!internalSettings->exceptionPreset().isEmpty()) {
                PresetsModel::readPreset(internalSettings.data(), m_config.data(), internalSettings->exceptionPreset());
            }
            return internalSettings;
        }
    }

    return m_defaultSettings;
}
}
