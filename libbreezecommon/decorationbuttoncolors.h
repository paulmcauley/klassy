#ifndef BREEZE_DECORATIONBUTTONCOLORS_H
#define BREEZE_DECORATIONBUTTONCOLORS_H

/*
 * SPDX-FileCopyrightText: 2023-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breeze.h"
#include "breezecommon_export.h"
#include "decorationcolors.h"
#include <KColorScheme>
#include <KDecoration2/DecorationButton>
#include <KDecoration2/DecorationSettings>
#include <QColor>
#include <memory>

namespace Breeze
{

class DecorationColors;

const QList<KDecoration2::DecorationButtonType> coloredWindowDecorationButtonTypes{
    KDecoration2::DecorationButtonType::Menu,
    KDecoration2::DecorationButtonType::ApplicationMenu,
    KDecoration2::DecorationButtonType::OnAllDesktops,
    KDecoration2::DecorationButtonType::Minimize,
    KDecoration2::DecorationButtonType::Maximize,
    KDecoration2::DecorationButtonType::Close,
    KDecoration2::DecorationButtonType::ContextHelp,
    KDecoration2::DecorationButtonType::Shade,
    KDecoration2::DecorationButtonType::KeepBelow,
    KDecoration2::DecorationButtonType::KeepAbove,
    KDecoration2::DecorationButtonType::Custom,
};

const QList<KDecoration2::DecorationButtonType> coloredAppStyleDecorationButtonTypes{
    KDecoration2::DecorationButtonType::Minimize,
    KDecoration2::DecorationButtonType::Maximize,
    KDecoration2::DecorationButtonType::Close,
};

enum struct BREEZECOMMON_EXPORT OverridableButtonColorState {
    IconPress,
    IconHover,
    IconNormal,
    BackgroundPress,
    BackgroundHover,
    BackgroundNormal,
    OutlinePress,
    OutlineHover,
    OutlineNormal,
    COUNT,
};

const QStringList overridableButtonColorStatesJsonStrings{
    QStringLiteral("IconPress"),
    QStringLiteral("IconHover"),
    QStringLiteral("IconNormal"),
    QStringLiteral("BackgroundPress"),
    QStringLiteral("BackgroundHover"),
    QStringLiteral("BackgroundNormal"),
    QStringLiteral("OutlinePress"),
    QStringLiteral("OutlineHover"),
    QStringLiteral("OutlineNormal"),
};

const QStringList overrideColorItems{
    QStringLiteral("Custom"),
    QStringLiteral("TitleBarTextAuto"),
    QStringLiteral("TitleBarTextActive"),
    QStringLiteral("TitleBarTextInactive"),
    QStringLiteral("TitleBarBackgroundAuto"),
    QStringLiteral("TitleBarBackgroundActive"),
    QStringLiteral("TitleBarBackgroundInactive"),
    QStringLiteral("AccentButtonHover"),
    QStringLiteral("AccentButtonFocus"),
    QStringLiteral("AccentHighlight"),
    QStringLiteral("AccentHighlightLessSaturated"),
    QStringLiteral("NegativeText"),
    QStringLiteral("NegativeLessSaturated"),
    QStringLiteral("NegativeSaturated"),
    QStringLiteral("NegativeFullySaturated"),
    QStringLiteral("NeutralText"),
    QStringLiteral("NeutralLessSaturated"),
    QStringLiteral("NeutralSaturated"),
    QStringLiteral("PositiveText"),
    QStringLiteral("PositiveLessSaturated"),
    QStringLiteral("PositiveSaturated"),
    QStringLiteral("White"),
    QStringLiteral("WindowOutlineAuto"),
    QStringLiteral("WindowOutlineActive"),
    QStringLiteral("WindowOutlineInactive"),
    QStringLiteral("WindowShadowAuto"),
    QStringLiteral("WindowShadowActive"),
    QStringLiteral("WindowShadowInactive"),
};

struct BREEZECOMMON_EXPORT DecorationButtonPaletteGroup {
    QColor foregroundPress;
    QColor foregroundHover;
    QColor foregroundNormal;

    QColor backgroundPress;
    bool negativePressCloseBackground;
    QColor backgroundHover;
    bool negativeHoverCloseBackground;
    QColor backgroundNormal;
    bool negativeNormalCloseBackground;

    QColor outlinePress;
    QColor outlineHover;
    QColor outlineNormal;
};

/**
 *  @brief Class to generate the colour palette used in a decoration button
 */
class BREEZECOMMON_EXPORT DecorationButtonPalette
{
public:
    DecorationButtonPalette(KDecoration2::DecorationButtonType buttonType);

    // DecorationButtonPalette(const DecorationButtonPalette &other);
    // DecorationButtonPalette& operator=(const DecorationButtonPalette& other);

    void generate(InternalSettingsPtr decorationSettings,
                  DecorationColors *decorationColors,
                  const bool generateOneGroupOnly = false,
                  const bool oneGroupActiveState = true);
    DecorationButtonPaletteGroup *active() const
    {
        return _active.get();
    }
    DecorationButtonPaletteGroup *inactive() const
    {
        return _inactive.get();
    }

    KDecoration2::DecorationButtonType buttonType()
    {
        return _buttonType;
    }

    static QColor overrideColorItemsIndexToColor(const DecorationColors *decorationColors, const int overrideColorItemsIndex, const bool active);

private:
    void decodeButtonOverrideColors(const bool active);
    void generateButtonBackgroundPalette(const bool active);
    void generateButtonForegroundPalette(const bool active);
    void generateButtonOutlinePalette(const bool active);

    InternalSettingsPtr _decorationSettings;
    KDecoration2::DecorationButtonType _buttonType;
    DecorationColors *_decorationColors;

    bool _buttonOverrideColorsPresentActive{false};
    bool _buttonOverrideColorsPresentInactive{false};

    QMap<OverridableButtonColorState, QColor> _buttonOverrideColorsActive;
    QMap<OverridableButtonColorState, QColor> _buttonOverrideColorsInactive;

    std::shared_ptr<DecorationButtonPaletteGroup> _active;
    std::shared_ptr<DecorationButtonPaletteGroup> _inactive;
};

}

#endif
