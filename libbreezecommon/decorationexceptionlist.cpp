//////////////////////////////////////////////////////////////////////////////
// breezeexceptionlist.cpp
// window decoration exceptions
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
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
    if (!config->hasGroup(defaultExceptionGroupName(0)) || readDefaults) {
        InternalSettingsPtr defaultException(new InternalSettings());
        defaultException->setEnabled(true);
        defaultException->setExceptionWindowPropertyType(1);
        defaultException->setExceptionWindowPropertyPattern(".*Kdenlive");
        defaultException->setOpaqueTitleBar(true);
        defaultException->setExceptionProgramNamePattern("kdenlive");
        defaultException->setPreventApplyOpacityToHeader(true);
        _defaultExceptions.append(defaultException);
    }

    QString groupName;

    if (!readDefaults) {
        // load default-set exceptions modified from the config file (with user's changes such as unticking enable)
        for (int index = 0; config->hasGroup(groupName = defaultExceptionGroupName(index)); ++index) {
            readIndividualExceptionFromConfig(config, groupName, _defaultExceptions);
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
    configuration->setMask(exception.mask());

    // propagate all features found in mask to the output configuration
    if (exception.mask() & BorderSize)
        configuration->setBorderSize(exception.borderSize());
    configuration->setHideTitleBar(exception.hideTitleBar());
    configuration->setOpaqueTitleBar(exception.opaqueTitleBar());
    configuration->setPreventApplyOpacityToHeader(exception.preventApplyOpacityToHeader());

    // append to exceptions
    appendTo.append(configuration);
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
    foreach (const InternalSettingsPtr &exception, _defaultExceptions) {
        writeConfig(exception.data(), config.data(), defaultExceptionGroupName(index));
        ++index;
    }

    // remove all existing user-set exceptions
    for (int index = 0; config->hasGroup(groupName = exceptionGroupName(index)); ++index) {
        config->deleteGroup(groupName);
    }

    // rewrite current user-set exceptions
    index = 0;
    foreach (const InternalSettingsPtr &exception, _exceptions) {
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
    // list of items to be written
    QStringList keys = {"Enabled",
                        "ExceptionProgramNamePattern",
                        "ExceptionWindowPropertyPattern",
                        "ExceptionWindowPropertyType",
                        "HideTitleBar",
                        "OpaqueTitleBar",
                        "PreventApplyOpacityToHeader",
                        "Mask",
                        "BorderSize"};

    // write all items
    foreach (auto key, keys) {
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
void DecorationExceptionList::readConfig(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &groupName)
{
    foreach (KConfigSkeletonItem *item, skeleton->items()) {
        if (!groupName.isEmpty())
            item->setGroup(groupName);
        item->readConfig(config);
    }
}
}
