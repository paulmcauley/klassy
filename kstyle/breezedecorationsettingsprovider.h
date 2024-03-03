/*
 * SPDX-FileCopyrightText: 2022-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

/*
 * breezedecorationsettingsprovider.h
 * Provides access to kdecoration exceptions to kstyle
 *
 */

#pragma once

#include "breeze.h"
#include "breezesettings.h"

#include <KSharedConfig>
#include <QMainWindow>
#include <QObject>

namespace Breeze
{

class DecorationSettingsProvider : public QObject
{
    Q_OBJECT

public:
    //* destructor
    ~DecorationSettingsProvider();

    //* singleton
    static DecorationSettingsProvider *self();

    //* decoration internal settings for qApp
    InternalSettingsPtr internalSettings();

public Q_SLOTS:

    //* reconfigure
    void reconfigure();

private:
    //* constructor
    DecorationSettingsProvider();

    //* default configuration
    InternalSettingsPtr m_defaultSettings;

    //* exceptions
    InternalSettingsList m_exceptions;

    //* config object
    KSharedConfigPtr m_config;

    //* presets config object
    KSharedConfigPtr m_presetsConfig;

    //* singleton
    static DecorationSettingsProvider *s_self;
};

}
