/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2022-2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breezesettingsprovider.h"
#include "dbusmessages.h"
#include "decorationexceptionlist.h"
#include "presetsmodel.h"

#include <QRegularExpression>
#include <QTextStream>

#include <mutex>

namespace Klassy
{

SettingsProvider *SettingsProvider::s_self = nullptr;
static std::mutex g_migrationMutex;

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
    m_exceptions = exceptions.getBundled();
    m_exceptions.append(exceptions.get());
    migratePre674();
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
            return internalSettings;
        }
    }

    return m_defaultSettings;
}

// function for migrating configs pre-6.7.4 -- delete in 2 releases time
void SettingsProvider::migratePre674()
{
    if (m_defaultSettings->migratedPre674()) {
        return;
    }

    std::unique_lock<std::mutex> lock(g_migrationMutex, std::try_to_lock);
    if (lock.owns_lock()) {
        if (!m_presetsConfig) {
            KSharedConfigPtr presetsConfig = KSharedConfig::openConfig(QStringLiteral("klassy/windecopresetsrc"));
            m_presetsConfig.swap(presetsConfig);
        }
        if (!m_presetsConfig || !m_config) {
            exitMigratePre674();
        }

        if (!m_presetsConfig->hasGroup("Global")) {
            exitMigratePre674();
        }
        auto presetsGlobal = m_presetsConfig->group("Global");
        if (!presetsGlobal.hasKey("BundledWindecoPresetsImportedVersion")) {
            exitMigratePre674();
        }
        QString presetsImportedVersion = presetsGlobal.readEntry("BundledWindecoPresetsImportedVersion");
        QRegularExpression rx("(\\d)\\.(\\d)\\.(\\d).*");
        QRegularExpressionMatch match = rx.match(presetsImportedVersion);
        if (!match.hasMatch()) {
            exitMigratePre674();
        }
        int major = match.captured(1).toInt();
        int minor = match.captured(2).toInt();
        int patch = match.captured(3).toInt();

        qDebug() << "major" << major;
        qDebug() << "minor" << minor;
        qDebug() << "patch" << patch;

        if (!(major <= 6 && minor <= 7 && patch < 4)) {
            exitMigratePre674();
        }

        qDebug() << "version to patch";

        m_defaultSettings->setFullHeightButtonWidthMarginLeft(qRound(static_cast<qreal>(m_defaultSettings->fullHeightButtonWidthMarginLeft()) / 2) + 1);
        m_defaultSettings->setFullHeightButtonWidthMarginRight(qRound(static_cast<qreal>(m_defaultSettings->fullHeightButtonWidthMarginRight()) / 2) + 1);

        struct OldNewEnumValue {
            QString oldValue;
            QString newValue;
        };

        std::vector<OldNewEnumValue> oldNewButtonIconStyles = {OldNewEnumValue{"StyleSystemIconTheme", "SystemIconTheme"},
                                                               OldNewEnumValue{"StyleMaterialDynamic", "MaterialDynamic"},
                                                               OldNewEnumValue{"StyleMaterialCentered", "MaterialCentered"},
                                                               OldNewEnumValue{"StyleMaterial", "Material"},
                                                               OldNewEnumValue{"StyleOxygen", "Oxygen"},
                                                               OldNewEnumValue{"StyleKiteDynamic", "KiteDynamic"},
                                                               OldNewEnumValue{"StyleKite", "Kite"},
                                                               OldNewEnumValue{"StyleTraditionalDynamic", "TraditionalDynamic"},
                                                               OldNewEnumValue{"StyleTraditional", "Traditional"},
                                                               OldNewEnumValue{"StyleMetro", "Metro"},
                                                               OldNewEnumValue{"StyleFluent", "Fluent"},
                                                               OldNewEnumValue{"StyleArk", "Ark"},
                                                               OldNewEnumValue{"StyleKisweetDynamic", "KisweetDynamic"},
                                                               OldNewEnumValue{"StyleKisweet", "Kisweet"}};

        if (m_config->hasGroup("Windeco")) {
            auto windeco = m_config->group("Windeco");
            for (auto oldNewButtonIconStyle : oldNewButtonIconStyles) {
                if (!windeco.readEntry(oldNewButtonIconStyle.oldValue, "").isEmpty()) {
                    auto items = m_defaultSettings->items();
                    for (auto &item : items) {
                        if (item->key() == "ButtonIconStyle") {
                            item->setProperty(oldNewButtonIconStyle.newValue);
                        }
                    }
                }
            }
        }

        exitMigratePre674();
    }
}

void SettingsProvider::exitMigratePre674()
{
    m_defaultSettings->setMigratedPre674(true);
    m_defaultSettings->save();
}
}
