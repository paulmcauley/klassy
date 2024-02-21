#ifndef BREEZE_SYSTEMICONGENERATOR_H
#define BREEZE_SYSTEMICONGENERATOR_H

/*
 * SPDX-FileCopyrightText: 2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "breeze.h"
#include "breezecommon_export.h"
#include "decorationcolors.h"
#include <KDecoration2/DecorationButton>
#include <KDecoration2/DecorationSettings>

namespace Breeze
{

class BREEZECOMMON_EXPORT SystemIconGenerator
{
public:
    SystemIconGenerator(InternalSettingsPtr internalSettings);

    void generate();

private:
    void addSystemScales();
    void generateIconThemeDir(const QString themeDirPath, const QString themeName, const QString inherits, const DecorationColors &decorationColors);

    InternalSettingsPtr m_internalSettings;

    QList<qreal> m_scales = {1, 1.25, 1.5, 1.75, 2, 2.25, 2.5, 2.75, 3};
    const QMap<InternalSettings::EnumIconSize::type, int> m_iconSizes{
        {InternalSettings::EnumIconSize::IconSmallMedium, 16},
        {InternalSettings::EnumIconSize::IconMedium, 18},
        {InternalSettings::EnumIconSize::IconLargeMedium, 20},
        {InternalSettings::EnumIconSize::IconLarge, 22},
        {InternalSettings::EnumIconSize::IconVeryLarge, 24},
        {InternalSettings::EnumIconSize::IconGiant, 32},
        {InternalSettings::EnumIconSize::IconHumongous, 48},
    };

    struct iconType {
        KDecoration2::DecorationButtonType type;
        bool checked;
        QString name;
    };

    const QList<iconType> m_iconTypes{
        {KDecoration2::DecorationButtonType::ApplicationMenu, false, QStringLiteral("application-menu-symbolic")},
        {KDecoration2::DecorationButtonType::ApplicationMenu, false, QStringLiteral("application-menu")},
        {KDecoration2::DecorationButtonType::OnAllDesktops, false, QStringLiteral("window-pin-symbolic")},
        //{KDecoration2::DecorationButtonType::OnAllDesktops, false, QStringLiteral("window-pin")},
        {KDecoration2::DecorationButtonType::OnAllDesktops, true, QStringLiteral("window-unpin-symbolic")},
        //{KDecoration2::DecorationButtonType::OnAllDesktops, true, QStringLiteral("window-unpin")},
        {KDecoration2::DecorationButtonType::Minimize, false, QStringLiteral("window-minimize-symbolic")},
        {KDecoration2::DecorationButtonType::Minimize, false, QStringLiteral("window-minimize")},
        {KDecoration2::DecorationButtonType::Maximize, false, QStringLiteral("window-maximize-symbolic")},
        {KDecoration2::DecorationButtonType::Maximize, false, QStringLiteral("window-maximize")},
        {KDecoration2::DecorationButtonType::Maximize, true, QStringLiteral("window-restore-symbolic")},
        {KDecoration2::DecorationButtonType::Maximize, true, QStringLiteral("window-restore")},
        {KDecoration2::DecorationButtonType::Close, false, QStringLiteral("window-close-symbolic")},
        {KDecoration2::DecorationButtonType::Close, false, QStringLiteral("window-close")},
        {KDecoration2::DecorationButtonType::Close, false, QStringLiteral("dialog-close")},
        {KDecoration2::DecorationButtonType::Close, false, QStringLiteral("geany-close-all")},
        {KDecoration2::DecorationButtonType::Close, false, QStringLiteral("gtk-close")},
        {KDecoration2::DecorationButtonType::Close, false, QStringLiteral("gtk-no")},
        {KDecoration2::DecorationButtonType::Close, false, QStringLiteral("kontes-close")},
        {KDecoration2::DecorationButtonType::Close, false, QStringLiteral("tab-close-other")},
        {KDecoration2::DecorationButtonType::Close, false, QStringLiteral("tab-close")},
        {KDecoration2::DecorationButtonType::ContextHelp, false, QStringLiteral("help-contextual-symbolic")},
        //{KDecoration2::DecorationButtonType::ContextHelp, false, QStringLiteral("help-contextual")},
        {KDecoration2::DecorationButtonType::Shade, false, QStringLiteral("window-shade-symbolic")},
        {KDecoration2::DecorationButtonType::Shade, false, QStringLiteral("window-shade")},
        {KDecoration2::DecorationButtonType::Shade, true, QStringLiteral("window-unshade-symbolic")},
        {KDecoration2::DecorationButtonType::Shade, true, QStringLiteral("window-unshade")},
        {KDecoration2::DecorationButtonType::KeepBelow, false, QStringLiteral("window-keep-below-symbolic")},
        {KDecoration2::DecorationButtonType::KeepBelow, false, QStringLiteral("window-keep-below")},
        {KDecoration2::DecorationButtonType::KeepAbove, false, QStringLiteral("window-keep-above-symbolic")},
        {KDecoration2::DecorationButtonType::KeepAbove, false, QStringLiteral("window-keep-above")},
    };
};

}

#endif
