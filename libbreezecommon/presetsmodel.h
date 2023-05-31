#ifndef BREEZE_PRESETSMODEL_H
#define BREEZE_PRESETSMODEL_H

/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "breeze.h"
#include "breezecommon_export.h"

namespace Breeze
{

/**
 * @brief Functions to read and write Presets from/to config file within Klassy
 */
class BREEZECOMMON_EXPORT PresetsModel
{
public:
    static QString presetGroupName(const QString str);
    static void writePreset(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &presetName);
    static void writePreset(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &presetName, const QStringList &whiteListKeys);
    static void readPreset(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &presetName);
    static void deletePreset(KConfig *config, const QString &presetName);
    static QStringList readPresetsList(KConfig *config);
    static bool isPresetPresent(KConfig *config, const QString &presetName);
};

}

#endif
