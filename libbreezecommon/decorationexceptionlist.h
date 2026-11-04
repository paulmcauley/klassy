//////////////////////////////////////////////////////////////////////////////
// decorationexceptionlist.h
// window decoration exceptions
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
// SPDX-FileCopyrightText: 2022-2023 Paul A McAuley <kde@paulmcauley.com>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "klassycommon_export.h"

#include "breeze.h"
#include "breezesettings.h"

#include <KSharedConfig>

namespace Klassy
{

//! breeze exceptions list
class KLASSYCOMMON_EXPORT DecorationExceptionList
{
public:
    //! constructor from list
    explicit DecorationExceptionList(const InternalSettingsList &exceptions = InternalSettingsList(),
                                     const InternalSettingsList &bundledExceptions = InternalSettingsList())
        : _exceptions(exceptions)
        , _bundledExceptions(bundledExceptions)
    {
    }

    //! exceptions
    const InternalSettingsList &get(void) const
    {
        return _exceptions;
    }

    //! bundled exceptions
    const InternalSettingsList &getBundled(void) const
    {
        return _bundledExceptions;
    }

    //! read from KConfig
    void readConfig(KSharedConfig::Ptr, const bool readDefaults = false);

    //! return the number of bundled exceptions (call afer calling readConfig)
    int numberBundled();

    //! write to kconfig
    void writeConfig(KSharedConfig::Ptr);

protected:
    //! generate exception group name for given exception index
    static QString exceptionGroupName(int index);

    //! generate exception group name for given bundled exception index
    static QString bundledExceptionGroupName(int index);

    //! read configuration
    static void readConfig(KCoreConfigSkeleton *, KConfig *, const QString &);

    //! write configuration
    static void writeConfig(KCoreConfigSkeleton *, KConfig *, const QString &);

    //! write configuration for defaults (enabled key only)
    void writeBundledConfig(KCoreConfigSkeleton *skeleton, KConfig *config, const QString &groupName);

private:
    void readIndividualExceptionFromConfig(KSharedConfig::Ptr config, QString &groupName, InternalSettingsList &appendTo);
    void readExceptionEnabledFromConfig(KSharedConfig::Ptr config, QString groupName, InternalSettingsList &settingsList, int index);

    //! exceptions
    InternalSettingsList _exceptions;

    //! bundled exceptions
    InternalSettingsList _bundledExceptions;
};
}
