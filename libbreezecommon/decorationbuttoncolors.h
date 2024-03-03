/*
 * SPDX-FileCopyrightText: 2023-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "breeze.h"
#include "breezecommon_export.h"
#include "decorationcolors.h"
#include <KColorScheme>
#include <QColor>
#include <memory>

namespace Breeze
{

struct DecorationPaletteGroup;
class DecorationColors;

const QList<DecorationButtonType> coloredWindowDecorationButtonTypes{
    DecorationButtonType::Menu,
    DecorationButtonType::ApplicationMenu,
    DecorationButtonType::OnAllDesktops,
    DecorationButtonType::Minimize,
    DecorationButtonType::Maximize,
    DecorationButtonType::Close,
    DecorationButtonType::ContextHelp,
    DecorationButtonType::Shade,
    DecorationButtonType::KeepBelow,
    DecorationButtonType::KeepAbove,
    DecorationButtonType::Custom,
};

const QList<DecorationButtonType> coloredAppStyleDecorationButtonTypes{
    DecorationButtonType::Minimize,
    DecorationButtonType::Maximize,
    DecorationButtonType::Close,
};

enum struct BREEZECOMMON_EXPORT OverridableButtonColorState {
    IconNormal,
    IconHover,
    IconPress,
    BackgroundNormal,
    BackgroundHover,
    BackgroundPress,
    OutlineNormal,
    OutlineHover,
    OutlinePress,
    COUNT,
};

enum struct BREEZECOMMON_EXPORT ButtonComponent { Icon, Background, Outline, COUNT };

const QStringList overridableButtonColorStatesJsonStrings{
    QStringLiteral("IconNormal"),
    QStringLiteral("IconHover"),
    QStringLiteral("IconPress"),
    QStringLiteral("BackgroundNormal"),
    QStringLiteral("BackgroundHover"),
    QStringLiteral("BackgroundPress"),
    QStringLiteral("OutlineNormal"),
    QStringLiteral("OutlineHover"),
    QStringLiteral("OutlinePress"),
};

const QStringList overrideColorItems{
    QStringLiteral("Custom"),
    QStringLiteral("TitleBarTextAuto"),
    QStringLiteral("TitleBarTextActive"),
    QStringLiteral("TitleBarTextInactive"),
    QStringLiteral("TitleBarBackgroundAuto"),
    QStringLiteral("TitleBarBackgroundActive"),
    QStringLiteral("TitleBarBackgroundInactive"),
    QStringLiteral("AccentButtonFocus"),
    QStringLiteral("AccentButtonHover"),
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

    //* These cutOut parameters signify that the corresponding colour has been set to the titlebar colour, and that it could be painted with black and
    // CompositionMode_DestinationOut to give a cut-out effect.
    bool cutOutForegroundPress = false;
    bool cutOutForegroundHover = false;
    bool cutOutForegroundNormal = false;

    QColor backgroundPress;
    QColor backgroundHover;
    QColor backgroundNormal;

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
    DecorationButtonPalette(DecorationButtonType buttonType);

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

    DecorationButtonType buttonType()
    {
        return _buttonType;
    }

    static QColor overrideColorItemsIndexToColor(const DecorationColors *decorationColors, const int overrideColorItemsIndex, const bool active);

private:
    void decodeButtonOverrideColors(const bool active);
    void generateBistateColors(ButtonComponent component,
                               const bool active,
                               QColor baseColor,
                               QColor &bistate1,
                               QColor &bistate2,
                               QColor accentHoverBase = QColor());
    void generateTristateColors(ButtonComponent component,
                                const bool active,
                                QColor baseColor,
                                QColor &tristate1,
                                QColor &tristate2,
                                QColor &tristate3,
                                QColor accentHoverBase = QColor());
    void generateButtonBackgroundPalette(const bool active);
    void generateButtonForegroundPalette(const bool active);
    void adjustPoorForegroundContrast(QColor &baseForegroundColor,
                                      const QColor &baseBackgroundColor,
                                      bool &cutOutParameter,
                                      const bool active,
                                      const DecorationPaletteGroup *decorationColorGroup);
    void generateButtonOutlinePalette(const bool active);

    InternalSettingsPtr _decorationSettings;
    DecorationButtonType _buttonType;
    DecorationColors *_decorationColors;

    bool _buttonOverrideColorsPresentActive{false};
    bool _buttonOverrideColorsPresentInactive{false};

    QMap<OverridableButtonColorState, QColor> _buttonOverrideColorsActive;
    QMap<OverridableButtonColorState, QColor> _buttonOverrideColorsInactive;

    std::shared_ptr<DecorationButtonPaletteGroup> _active;
    std::shared_ptr<DecorationButtonPaletteGroup> _inactive;
};

}
