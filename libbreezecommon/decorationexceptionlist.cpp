//////////////////////////////////////////////////////////////////////////////
// breezeexceptionlist.cpp
// window decoration exceptions
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
// SPDX-FileCopyrightText: 2022-2023 Paul A McAuley <kde@paulmcauley.com>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "decorationexceptionlist.h"

namespace Breeze
{

//______________________________________________________________
void DecorationExceptionList::readConfig(KSharedConfig::Ptr config, const bool readDefaults)
{
    _exceptions.clear();
    _defaultExceptions.clear();

    // set the default exceptions that are bundled with Klassy
    InternalSettingsPtr defaultException0(new InternalSettings());
    defaultException0->setExceptionWindowPropertyType(InternalSettings::EnumExceptionWindowPropertyType::ExceptionWindowClassName);
    defaultException0->setExceptionWindowPropertyPattern("");
    defaultException0->setOpaqueTitleBar(false);
    defaultException0->setExceptionProgramNamePattern("VirtualBox.*");
    defaultException0->setPreventApplyOpacityToHeader(true);
    _defaultExceptions.append(defaultException0);

    QString groupName;

    // load default enabled settings for the default exceptions
    if (!readDefaults) {
        for (int index = 0; index < _defaultExceptions.count(); ++index) {
            readExceptionEnabledFromConfig(config, defaultExceptionGroupName(index), _defaultExceptions, index);
        }
    }

    // load user-set exceptions from the config file
    for (int index = 0; config->hasGroup(groupName = exceptionGroupName(index)); ++index) {
        readIndividualExceptionFromConfig(config, groupName, _exceptions);
    }
}

void DecorationExceptionList::readIndividualExceptionFromConfig(KSharedConfig::Ptr config, QString &groupName, InternalSettingsList &appendTo)
{
    // create exception
    InternalSettings exception;

    // reset group
    readConfig(&exception, config.data(), groupName);

    // create new configuration
    InternalSettingsPtr configuration(new InternalSettings());
    configuration.data()->load();

    // apply changes from exception
    configuration->setEnabled(exception.enabled());
    configuration->setExceptionWindowPropertyType(exception.exceptionWindowPropertyType());
    configuration->setExceptionProgramNamePattern(exception.exceptionProgramNamePattern());
    configuration->setExceptionWindowPropertyPattern(exception.exceptionWindowPropertyPattern());

    configuration->setExceptionPreset(exception.exceptionPreset());
    configuration->setExceptionBorder(exception.exceptionBorder());

    if (exception.exceptionBorder())
        configuration->setBorderSize(exception.borderSize());
    configuration->setHideTitleBar(exception.hideTitleBar());
    configuration->setOpaqueTitleBar(exception.opaqueTitleBar());
    configuration->setPreventApplyOpacityToHeader(exception.preventApplyOpacityToHeader());

    // append to exceptions
    appendTo.append(configuration);
}

void DecorationExceptionList::readExceptionEnabledFromConfig(KSharedConfig::Ptr config, QString groupName, InternalSettingsList &settingsList, int index)
{
    // create exception
    InternalSettings exception;

    // reset group
    readConfig(&exception, config.data(), groupName);

    // append to exceptions
    settingsList[index]->setEnabled(exception.enabled());
}

int DecorationExceptionList::numberDefaults()
{
    return _defaultExceptions.size();
}

//______________________________________________________________
void DecorationExceptionList::writeConfig(KSharedConfig::Ptr config)
{
    QString groupName;
    // remove all existing default-set exceptions
    for (int index = 0; config->hasGroup(groupName = defaultExceptionGroupName(index)); ++index) {
        config->deleteGroup(groupName);
    }

    // rewrite current default exceptions with user-set enable flag
    int index = 0;
    for (const InternalSettingsPtr &exception : std::as_const(_defaultExceptions)) {
        writeDefaultsConfig(exception.data(), config.data(), defaultExceptionGroupName(index));
        ++index;
    }

    // remove all existing user-set exceptions
    for (int index = 0; config->hasGroup(groupName = exceptionGroupName(index)); ++index) {
        config->deleteGroup(groupName);
    }

    // rewrite current user-set exceptions
    index = 0;
    for (const InternalSettingsPtr &exception : std::as_const(_exceptions)) {
        writeConfig(exception.data(), config.data(), exceptionGroupName(index));
        ++index;
    }
}

//_______________________________________________________________________
QString DecorationExceptionList::exceptionGroupName(int index)
{
    return QString("Windeco Exception %1").arg(index);
}

//_______________________________________________________________________
QString DecorationExceptionList::defaultExceptionGroupName(int index)
{
    return QString("Default Windeco Exception %1").arg(index);
}

//______________________________________________________________
void DecorationExceptionList::writeConfig(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &groupName)
{
    // write all items in windecoExceptionKeys
    for (auto key : windecoExceptionKeys) {
        KConfigSkeletonItem *item(skeleton->findItem(key));
        if (!item)
            continue;

        if (!groupName.isEmpty())
            item->setGroup(groupName);
        KConfigGroup configGroup(config, item->group());
        configGroup.writeEntry(item->key(), item->property());
    }
}

//______________________________________________________________
void DecorationExceptionList::writeDefaultsConfig(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &groupName)
{
    // list of items to be written
    QStringList keys = {"Enabled"};

    // write all items
    for (auto key, keys) {
        KConfigSkeletonItem *item(skeleton->findItem(key));
        if (!item || item->isEqual(true)) // don't write true to file as it's already the default
            continue;

        if (!groupName.isEmpty())
            item->setGroup(groupName);
        KConfigGroup configGroup(config, item->group());
        configGroup.writeEntry(item->key(), item->property());
    }
}

//______________________________________________________________
void DecorationExceptionList::readConfig(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &groupName)
{
    const auto items = skeleton->items();
    for (KConfigSkeletonItem *item : items) {
        if (!groupName.isEmpty())
            item->setGroup(groupName);
        item->readConfig(config);
    }
}
}
