/*
 * SPDX-FileCopyrightText: 2023-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "breeze.h"
#include "breezecommon_export.h"

namespace Breeze
{

enum struct BREEZECOMMON_EXPORT PresetsErrorFlag {
    None,
    InvalidGlobalGroup,
    InvalidVersion,
    InvalidGroup,
    InvalidKey,
};

/**
 * @brief Functions to read and write Presets from/to config file within Klassy
 */
class BREEZECOMMON_EXPORT PresetsModel
{
public:
    static QString presetGroupName(const QString str);
    static void writePreset(KCoreConfigSkeleton *skeleton, KConfig *presetsConfig, const QString &presetName);

    //* needed to handle special data types like enums, intLists
    static void writeSkeletonItemToConfigGroup(KConfigSkeletonItem *item, KConfigGroup &configGroup);
    static bool loadPreset(KCoreConfigSkeleton *skeleton, KConfig *presetsConfig, const QString &presetName, bool writeKwinBorderConfig = false);

    //* loadPresetAndSave method is needed because the skeleton->save() member method does not work when loadPreset is called
    static bool
    loadPresetAndSave(KCoreConfigSkeleton *skeleton, KConfig *mainConfig, KConfig *presetsConfig, const QString &presetName, bool writeKwinBorderConfig);
    static bool presetHasKwinBorderSizeKey(KConfig *presetsConfig, const QString &presetName);

    //* used in the case where you want to use the preset KwinBorderSize in an exception (window-specific override)
    // returns true if the preset has a set KwinBorderSize key
    static void copyKwinBorderSizeFromPresetToExceptionBorderSize(KCoreConfigSkeleton *skeleton, KConfig *presetsConfig, const QString &presetName);

    static void writeBorderSizeToKwinConfig(const QString &borderSize);
    static void deletePreset(KConfig *presetsConfig, const QString &presetName);
    static void deleteBundledPresets(KConfig *presetsConfig);
    static QStringList readPresetsList(KConfig *presetsConfig);
    static bool isPresetPresent(KConfig *presetsConfig, const QString &presetName);
    static bool isPresetFromFilePresent(KConfig *presetsConfig, const QString &presetFileName, QString &presetName);
    static void exportPreset(KConfig *presetsConfig, const QString &presetName, const QString &filePath);
    static PresetsErrorFlag importPreset(KConfig *presetsConfig,
                                         const QString &filePath,
                                         QString &presetName,
                                         QString &error,
                                         bool forceInvalidVersion = false,
                                         bool markAsBundled = false);
    static bool isKeyValid(const QString &key);
    static bool isEnumValueValid(const QString &key, const QString &property);
    static void importBundledPresets(KConfig *presetsConfig);
};

}
