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
void DecorationExceptionList::readConfig(KSharedConfig::Ptr config)
{
    _exceptions.clear();

    QString groupName;
    for (int index = 0; config->hasGroup(groupName = exceptionGroupName(index)); ++index) {
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
        _exceptions.append(configuration);
    }
}

//______________________________________________________________
void DecorationExceptionList::writeConfig(KSharedConfig::Ptr config)
{
    // remove all existing exceptions
    QString groupName;
    for (int index = 0; config->hasGroup(groupName = exceptionGroupName(index)); ++index) {
        config->deleteGroup(groupName);
    }

    // rewrite current exceptions
    int index = 0;
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
