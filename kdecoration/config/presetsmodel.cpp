/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "presetsmodel.h"

namespace Breeze
{

QString PresetsModel::presetGroupName(const QString str)
{
    return QString("Windeco Preset %1").arg(str);
}

//______________________________________________________________
void PresetsModel::writePreset(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &presetName)
{
    bool keyInBlacklist;
    QString groupName = presetGroupName(presetName);
    // list of items to be written
    QStringList blacklistKeys = {"Enabled",
                                 "ExceptionProgramNamePattern",
                                 "ExceptionWindowPropertyPattern",
                                 "ExceptionWindowPropertyType",
                                 "HideTitleBar",
                                 "OpaqueTitleBar",
                                 "PreventApplyOpacityToHeader",
                                 "Mask",
                                 "BorderSize"};

    foreach (auto item, skeleton->items()) {
        keyInBlacklist = false;
        foreach (auto blacklistKey, blacklistKeys) {
            if (item->key() == blacklistKey) {
                keyInBlacklist = true;
                continue;
            }
        }

        if (keyInBlacklist)
            continue;
        if (!groupName.isEmpty())
            item->setGroup(groupName);
        KConfigGroup configGroup(config, item->group());
        configGroup.writeEntry(item->key(), item->property());
    }
}

void PresetsModel::writePreset(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &presetName, const QStringList &whitelistKeys)
{
    bool keyInWhitelist;
    QString groupName = presetGroupName(presetName);

    foreach (auto item, skeleton->items()) {
        keyInWhitelist = false;
        foreach (auto whitelistKey, whitelistKeys) {
            if (item->key() == whitelistKey) {
                keyInWhitelist = true;
                continue;
            }
        }

        if (!keyInWhitelist)
            continue;
        if (!groupName.isEmpty())
            item->setGroup(groupName);
        KConfigGroup configGroup(config, item->group());
        configGroup.writeEntry(item->key(), item->property());
    }
}

void PresetsModel::readPreset(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &presetName)
{
    QString groupName = presetGroupName(presetName);

    foreach (KConfigSkeletonItem *item, skeleton->items()) {
        if (!groupName.isEmpty())
            item->setGroup(groupName);
        item->readConfig(config);
    }
}

void PresetsModel::deletePreset(KConfig *config, const QString &presetName)
{
    QString groupName = presetGroupName(presetName);

    if (config->hasGroup(groupName))
        config->deleteGroup(groupName);
}

}
