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

    // write window decoration configuration as a preset
    for (auto item : skeleton->items()) {
        keyInBlacklist = false;

        QStringList blacklistKeys = windecoExceptionKeys;
        blacklistKeys << "BundledWindecoPresetsImportedVersion";

        // do not write a key in blacklist
        for (const QString &blacklistKey : blacklistKeys) {
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

        // enum properties are ints, but it is more robust to write the full string name to the file, rather than an int
        // therefore if an enum get the name instead
        if (auto enumItem = dynamic_cast<KCoreConfigSkeleton::ItemEnum *>(item)) { // if the item is an enum
            if (item->property().toInt() >= 0) { // invalid enum values are set to -1
                configGroup.writeEntry(item->key(), enumItem->choices()[item->property().toInt()].name);
            }
        } else if (auto intListItem = dynamic_cast<KCoreConfigSkeleton::ItemIntList *>(
                       item)) { // if the item is an IntList, need to loop through each element in list and write
            QVariant property = intListItem->property();
            QList<int> *list = static_cast<QList<int> *>(property.data());

            QString colorListString;
            int i = 0;
            for (int colorInt : *list) {
                QTextStream(&colorListString) << colorInt;
                if (i < (list->count() - 1))
                    colorListString += ",";
                i++;
            }
            if (!colorListString.isNull())
                configGroup.writeEntry(item->key(), colorListString);
        } else {
            configGroup.writeEntry(item->key(), item->property());
        }
    }

    // read kwin window border setting and write to the preset
    KSharedConfig::Ptr kwinConfig = KSharedConfig::openConfig(QStringLiteral("kwinrc"));
    if (kwinConfig->hasGroup(QStringLiteral("org.kde.kdecoration2"))) {
        KConfigGroup kdecoration2Group = kwinConfig->group(QStringLiteral("org.kde.kdecoration2"));
        QString borderSize;
        if (!kdecoration2Group.hasKey(QStringLiteral("BorderSize"))) {
            borderSize = QStringLiteral("Normal"); // Normal is the KWin default, so will nor write a BorderSize key in this case
        } else {
            borderSize = kdecoration2Group.readEntry(QStringLiteral("BorderSize"));
        }
        if (!groupName.isEmpty()) {
            KConfigGroup configGroup(config, groupName);
            configGroup.writeEntry(QStringLiteral("KwinBorderSize"), borderSize);
        }
    }
}

void PresetsModel::loadPreset(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &presetName, bool writeKwinBorderConfig)
{
    bool keyInBlacklist;
    QString groupName = presetGroupName(presetName);

    if (groupName.isEmpty() || !config->hasGroup(groupName))
        return;

    for (KConfigSkeletonItem *item : skeleton->items()) {
        keyInBlacklist = false;

        QStringList blacklistKeys = windecoExceptionKeys;
        blacklistKeys << "BundledWindecoPresetsImportedVersion";

        // do not read a key in windecoExceptionKeys
        for (const QString &blacklistKey : blacklistKeys) {
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

    // writes the value of KwinBorderSize from the preset into the kwinrc file
    if (writeKwinBorderConfig) {
        KConfigGroup configGroup = config->group(groupName);
        if (configGroup.hasKey(QStringLiteral("KwinBorderSize"))) {
            writeBorderSizeToKwinConfig(configGroup.readEntry(QStringLiteral("KwinBorderSize")));
        }
    }
}

void PresetsModel::copyKwinBorderSizeFromPresetToExceptionBorderSize(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &presetName)
{
    QString groupName = presetGroupName(presetName);

    if (groupName.isEmpty() || !config->hasGroup(groupName))
        return;

    KCoreConfigSkeleton::ItemEnum *borderSize = static_cast<KCoreConfigSkeleton::ItemEnum *>(skeleton->findItem("BorderSize"));
    KConfigGroup configGroup = config->group(groupName);
    if (configGroup.hasKey(QStringLiteral("KwinBorderSize"))) {
        auto choiceList = borderSize->choices();
        int borderValue = -1;
        for (int i = 0; i < choiceList.count(); i++) { // need to convert the string value of the enum value to an int for compatibility
            if (choiceList[i].name == configGroup.readEntry(QStringLiteral("KwinBorderSize")))
                borderValue = i;
        }
        if (borderValue == -1)
            return;
        borderSize->setProperty(borderValue);
    }
}

bool PresetsModel::presetHasKwinBorderSizeKey(KConfig *config, const QString &presetName)
{
    QString groupName = presetGroupName(presetName);

    if (groupName.isEmpty() || !config->hasGroup(groupName))
        return false;
    KConfigGroup configGroup = config->group(groupName);
    if (configGroup.hasKey(QStringLiteral("KwinBorderSize")))
        return true;
    else
        return false;
}

void PresetsModel::writeBorderSizeToKwinConfig(const QString &borderSize)
{
    KSharedConfig::Ptr kwinConfig = KSharedConfig::openConfig(QStringLiteral("kwinrc"));
    KConfigGroup kdecoration2Group = kwinConfig->group(QStringLiteral("org.kde.kdecoration2"));
    kdecoration2Group.writeEntry(QStringLiteral("BorderSize"), borderSize);
    kwinConfig->sync();
}

void PresetsModel::deletePreset(KConfig *config, const QString &presetName)
{
    QString groupName = presetGroupName(presetName);

    if (config->hasGroup(groupName))
        config->deleteGroup(groupName);
}

void PresetsModel::deleteBundledPresets(KConfig *config)
{
    QStringList presetList = readPresetsList(config);
    for (const QString &presetName : presetList) {
        QString groupName = presetGroupName(presetName);
        if (config->hasGroup(groupName)) {
            KConfigGroup presetGroup = config->group(groupName);
            if (presetGroup.hasKey("BundledPreset")) {
                if (presetGroup.readEntry("BundledPreset") == "true") {
                    config->deleteGroup(groupName);
                }
            }
        }
    }
}

QStringList PresetsModel::readPresetsList(KConfig *config)
{
    QStringList presetsList;
    QRegularExpression re("^Windeco Preset (.+)");
    for (const QString &group : config->groupList()) {
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
    QStringList list = readPresetsList(config);
    return list.contains(presetName);
}

bool PresetsModel::isPresetFromFilePresent(KConfig *config, const QString &presetFileName, QString &presetName)
{
    KSharedConfig::Ptr importPresetConfig = KSharedConfig::openConfig(presetFileName);
    if (!importPresetConfig) {
        return false;
    }

    QStringList list = readPresetsList(importPresetConfig.data());
    if (!list.count())
        return false;
    presetName = list[0];
    return isPresetPresent(config, presetName);
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

    auto internalSettings = InternalSettingsPtr(new InternalSettings());

    for (const QString &inputKey : inputPresetGroup.keyList()) {
        auto item = internalSettings->findItem(inputKey);
        if (!item)
            continue;

        QString exportProperty = inputPresetGroup.readEntry(inputKey);
        outputPresetGroup.writeEntry(inputKey, exportProperty);
    }
    if (inputPresetGroup.hasKey("KwinBorderSize"))
        outputPresetGroup.writeEntry("KwinBorderSize", inputPresetGroup.readEntry("KwinBorderSize"));

    outputPresetConfig->sync();
}

PresetsErrorFlag
PresetsModel::importPreset(KConfig *config, const QString &fileName, QString &presetName, QString &error, bool forceInvalidVersion, bool markAsBundled)
{
    KSharedConfig::Ptr importPresetConfig = KSharedConfig::openConfig(fileName);

    // perform validation first
    if (!(importPresetConfig->hasGroup("Klassy Window Decoration Preset File")))
        return PresetsErrorFlag::InvalidGlobalGroup;
    KConfigGroup importGlobalGroup = importPresetConfig->group("Klassy Window Decoration Preset File");
    QString importVersion = importGlobalGroup.readEntry("version");
    bool versionValid = (importVersion == klassyLongVersion());
    if (!versionValid && !forceInvalidVersion)
        return PresetsErrorFlag::InvalidVersion;

    QStringList presetsList = readPresetsList(importPresetConfig.data());
    if (presetsList.count())
        presetName = presetsList[0];
    else {
        return PresetsErrorFlag::InvalidGroup;
    }

    QString importGroupName = presetGroupName(presetName);

    KConfigGroup importGroup = importPresetConfig->group(importGroupName);

    for (const QString &importKey : importGroup.keyList()) {
        if (!isKeyValid(importKey)) {
            error = importKey;
            return PresetsErrorFlag::InvalidKey;
        }
    }

    // end of validation

    // delete an existing preset if has the same name
    if (isPresetPresent(config, presetName)) {
        deletePreset(config, presetName);
    }

    // start writing the values
    auto internalSettings = InternalSettingsPtr(new InternalSettings());
    KConfigGroup configGroup(config, importGroupName);

    for (const QString &importKey : importGroup.keyList()) {
        QString importProperty = importGroup.readEntry(importKey);
        configGroup.writeEntry(importKey, importProperty);
    }

    if (markAsBundled)
        configGroup.writeEntry("BundledPreset", "true");

    return PresetsErrorFlag::None;
}

bool PresetsModel::isKeyValid(const QString &key)
{
    auto internalSettings = InternalSettingsPtr(new InternalSettings());

    for (const auto &item : internalSettings->items()) {
        if (item->key() == key) {
            return true;
        }
    }

    if (key == "KwinBorderSize")
        return true; // additional valid key containing KWin border size setting from kwinrc

    return false;
}
}
