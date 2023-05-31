/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "presetsmodel.h"
#include <KConfigGroup>
#include <QRegularExpression>

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

    foreach (auto item, skeleton->items()) {
        keyInBlacklist = false;
        // do not write a key in windecoExceptionKeys
        foreach (auto blacklistKey, windecoExceptionKeys) {
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
    bool keyInBlacklist;
    QString groupName = presetGroupName(presetName);

    if (groupName.isEmpty() || !config->hasGroup(groupName))
        return;

    foreach (KConfigSkeletonItem *item, skeleton->items()) {
        keyInBlacklist = false;
        // do not read a key in windecoExceptionKeys
        foreach (auto blacklistKey, windecoExceptionKeys) {
            if (item->key() == blacklistKey) {
                keyInBlacklist = true;
                continue;
            }
        }

        if (keyInBlacklist)
            continue;
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

QStringList PresetsModel::readPresetsList(KConfig *config)
{
    QStringList presetsList;
    QRegularExpression re("^Windeco Preset (.+)");
    foreach (const QString group, config->groupList()) {
        QRegularExpressionMatch match = re.match(group);
        if (match.hasMatch()) {
            QString presetName = match.captured(1);
            presetsList.append(presetName);
        }
    }
    return presetsList;
}

bool PresetsModel::isPresetPresent(KConfig *config, const QString &presetName)
{
    return readPresetsList(config).contains(presetName);
}
}
