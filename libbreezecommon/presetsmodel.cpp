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

        QStringList blacklistKeys = windecoExceptionKeys;
        blacklistKeys << "BundledWindecoPresetsImportedVersion";

        // do not write a key in blacklist
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
    bool keyInBlacklist;
    QString groupName = presetGroupName(presetName);

    if (groupName.isEmpty() || !config->hasGroup(groupName))
        return;

    foreach (KConfigSkeletonItem *item, skeleton->items()) {
        keyInBlacklist = false;

        QStringList blacklistKeys = windecoExceptionKeys;
        blacklistKeys << "BundledWindecoPresetsImportedVersion";

        // do not read a key in windecoExceptionKeys
        foreach (auto blacklistKey, blacklistKeys) {
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

void PresetsModel::exportPreset(KConfig *config, const QString &presetName, const QString &fileName)
{
    if (presetName.isEmpty() || fileName.isEmpty())
        return;

    KSharedConfig::Ptr outputPresetConfig = KSharedConfig::openConfig(fileName);
    QString groupName = presetGroupName(presetName);

    if (groupName.isEmpty() || !config->hasGroup(groupName))
        return;

    KConfigGroup inputPresetGroup = config->group(groupName);
    KConfigGroup outputGlobalGroup = outputPresetConfig->group("Klassy Window Decoration Preset File");
    KConfigGroup outputPresetGroup = outputPresetConfig->group(groupName);

    outputGlobalGroup.writeEntry("version", klassyLongVersion());

    for (QString &inputKey : inputPresetGroup.keyList()) {
        outputPresetGroup.writeEntry(inputKey, inputPresetGroup.readEntry(inputKey));
    }
    outputPresetConfig->sync();
}

void PresetsModel::importPresetValidate(const QString &fileName,
                                        KSharedConfig::Ptr &importPresetConfig,
                                        bool &validGlobalGroup,
                                        bool &versionValid,
                                        QString &presetName)
{
    importPresetConfig = KSharedConfig::openConfig(fileName);

    if (!(validGlobalGroup = importPresetConfig->hasGroup("Klassy Window Decoration Preset File")))
        return;
    KConfigGroup importGlobalGroup = importPresetConfig->group("Klassy Window Decoration Preset File");
    QString importVersion = importGlobalGroup.readEntry("version");
    versionValid = (importVersion == klassyLongVersion());

    presetName = readPresetsList(importPresetConfig.data())[0];
}

void PresetsModel::importPreset(KConfig *config, KSharedConfig::Ptr &importPresetConfig, const QString &presetName)
{
    QString importGroupName = presetGroupName(presetName);

    if (!importPresetConfig->hasGroup(importGroupName))
        return;
    KConfigGroup importGroup = importPresetConfig->group(importGroupName);

    for (QString &importKey : importGroup.keyList()) {
        KConfigGroup configGroup(config, importGroupName);
        configGroup.writeEntry(importKey, importGroup.readEntry(importKey));
    }
}
}
