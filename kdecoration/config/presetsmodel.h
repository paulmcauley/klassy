#ifndef BREEZE_PRESETSMODEL_H
#define BREEZE_PRESETSMODEL_H

/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breeze.h"

namespace Breeze
{

/**
 * @brief Functions to read and write Presets from/to config file within Klassy
 */
class PresetsModel
{
public:
    static QString presetGroupName(const QString str);
    static void writePreset(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &presetName);
    static void writePreset(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &presetName, const QStringList &whiteListKeys);
    static void readPreset(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &presetName);
    static void deletePreset(KConfig *config, const QString &presetName);
};

}

#endif
