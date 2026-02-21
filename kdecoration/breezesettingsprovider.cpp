/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2022-2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breezesettingsprovider.h"
#include "dbusmessages.h"
#include "decorationexceptionlist.h"
#include "kdecorationglobals.h"
#include "presetsmodel.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QTextStream>

namespace Breeze
{

SettingsProvider *SettingsProvider::s_self = nullptr;

//__________________________________________________________________
SettingsProvider::SettingsProvider()
    : m_config(KSharedConfig::openConfig(QStringLiteral("klassy/klassyrc")))
    , m_presetsConfig(KSharedConfigPtr())
{
    m_defaultSettings = InternalSettingsPtr(new InternalSettings());
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
    refreshConfig();
}

//__________________________________________________________________
InternalSettingsPtr SettingsProvider::internalSettings(Decoration *decoration)
{
    // get the client
    auto client = decoration->window();

    for (auto internalSettings : std::as_const(m_exceptions)) {
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
        case InternalSettings::EnumExceptionWindowPropertyType::ExceptionWindowTitle: {
            windowPropertyValue = client->caption();
            break;
        }

        default:
        case InternalSettings::EnumExceptionWindowPropertyType::ExceptionWindowClassName: {
            windowPropertyValue = client->windowClass(); // windowClass() available from KDecoration 5.27 onwards
            break;
        }
        }

        // check matching
        QRegularExpression rx(internalSettings->exceptionWindowPropertyPattern(), QRegularExpression::MultilineOption);
        if (rx.match(windowPropertyValue).hasMatch()) {
            // load preset if set
            if (!internalSettings->exceptionPreset().isEmpty()) {
                if (!m_presetsConfig) {
                    KSharedConfigPtr presetsConfig = KSharedConfig::openConfig(QStringLiteral("klassy/windecopresetsrc"));
                    m_presetsConfig.swap(presetsConfig);
                }
                if (!m_presetsConfig) {
                    return internalSettings;
                }

                // load the preset values into internalSettings if a preset is set as an exception
                PresetsModel::loadPreset(internalSettings.data(), m_presetsConfig.data(), internalSettings->exceptionPreset());

                // if a border size exception is not set then replace it with the KwinBorderSize value from the preset
                if ((!internalSettings->exceptionBorder())) {
                    if (PresetsModel::presetHasKwinBorderSizeKey(m_presetsConfig.data(), internalSettings->exceptionPreset())) {
                        PresetsModel::copyKwinBorderSizeFromPresetToExceptionBorderSize(internalSettings.data(),
                                                                                        m_presetsConfig.data(),
                                                                                        internalSettings->exceptionPreset());
                        internalSettings->setExceptionBorder(true);
                    }
                }
                internalSettings->setProperty("noCacheException",
                                              true); // this property is to indicate not to cache shadows or colours for an exception with a Preset
                                                     // -- this is because the Preset exception can alter shadows and colours
            }
            if (internalSettings->opaqueTitleBar()) {
                internalSettings->setProperty("noCacheException", true);
            }
            if (internalSettings->exceptionMatchTitleBarToApplicationColor()) {
                internalSettings->setProperty("noCacheException", true);
                internalSettings->setMatchTitleBarToApplicationColor(true);
            }
#if KDECORATION_VERSION < KDECORATION_VERSION_CHECK(6, 5, 0)
            internalSettings->setRoundAllCornersWhenNoBorders(false);
#endif
            return internalSettings;
        }
    }
#if KDECORATION_VERSION < KDECORATION_VERSION_CHECK(6, 5, 0)
    m_defaultSettings->setRoundAllCornersWhenNoBorders(false);
#endif
    return m_defaultSettings;
}
// delete this after Klassy v6.5; v6.5 spacing is standardised to be all in pixels and will make the existing configs look corrupt
void SettingsProvider::refreshConfig()
{
    QString klassyVersion = klassyLongVersion();
    if (klassyVersion == "6.5" || klassyVersion == "6.5.1" || klassyVersion == "6.5.2" || klassyVersion == "6.5.3" || klassyVersion == "6.5.git") {
        QString refreshedConfig = m_defaultSettings->refreshedConfig();
        if (refreshedConfig != QStringLiteral("6.5") && refreshedConfig != QStringLiteral("6.5.1") && refreshedConfig != QStringLiteral("6.5.2")
            && refreshedConfig != QStringLiteral("6.5.3") && refreshedConfig != QStringLiteral("6.5.git")) {
            // backup the user's existing config
            auto backupConfig = m_config->copyTo(m_config->name() + QDateTime::currentDateTime().toString(Qt::ISODate) + QStringLiteral(".old"));
            backupConfig->sync();

            // refresh the config to defaults
            m_defaultSettings->setDefaults();
            m_defaultSettings->setRefreshedConfig(klassyVersion);
            m_defaultSettings->save();
            m_config->sync();
        }
    }
}
}
