#ifndef BREEZE_DECORATIONBUTTONCOMMON_H
#define BREEZE_DECORATIONBUTTONCOMMON_H

/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breeze.h"
#include "breezecommon_export.h"
#include "colortools.h"
#include <KDecoration2/DecorationButton>

namespace Breeze
{

enum struct BREEZECOMMON_EXPORT ColorOverridableButtonTypes {
    Close,
    Maximize,
    Minimize,
    Help,
    Shade,
    AllDesktops,
    KeepBelow,
    KeepAbove,
    ApplicationMenu,
    Menu,
    Count,
};

enum struct BREEZECOMMON_EXPORT OverridableButtonColorStates {
    IconNormal,
    IconHover,
    IconPress,
    BackgroundNormal,
    BackgroundHover,
    BackgroundPress,
    OutlineNormal,
    OutlineHover,
    OutlinePress,
    Count,
};

/**
 *  @brief Class to generate the desired behaviour of a decoration button
 */
class BREEZECOMMON_EXPORT DecorationButtonBehaviour
{
public:
    DecorationButtonBehaviour();
    void reconfigure(InternalSettingsPtr decorationSettings);
    bool drawBackgroundNormally;
    bool drawBackgroundOnHover;
    bool drawBackgroundOnPress;
    // bool drawBackgroundDifferentColoredHover;
    bool drawCloseBackgroundNormally;
    bool drawCloseBackgroundOnHover;
    bool drawCloseBackgroundOnPress;
    bool drawOutlineNormally;
    bool drawOutlineOnHover;
    bool drawOutlineOnPress;
    // bool drawOutlineDifferentColoredHover;
    bool drawCloseOutlineNormally;
    bool drawCloseOutlineOnHover;
    bool drawCloseOutlineOnPress;
    bool drawIconNormally;
    bool drawIconOnHover;
    bool drawIconOnPress;
    bool drawCloseIconNormally;
    bool drawCloseIconOnHover;
    bool drawCloseIconOnPress;

private:
    InternalSettingsPtr _decorationSettings;
};

/**
 *  @brief Class to generate the colour palette used in a decoration button
 */
class BREEZECOMMON_EXPORT DecorationButtonPalette
{
public:
    DecorationButtonPalette(KDecoration2::DecorationButtonType buttonType);

    void reconfigure(InternalSettingsPtr decorationSettings,
                     DecorationButtonBehaviour *buttonBehaviour,
                     DecorationColors *decorationColors,
                     QColor baseForeground,
                     QColor baseBackground);

    KDecoration2::DecorationButtonType buttonType()
    {
        return _buttonType;
    }

    QColor foregroundNormal;
    QColor foregroundHover;
    QColor foregroundPress;

    QColor backgroundNormal;
    bool negativeNormalCloseBackground;
    QColor backgroundHover;
    bool negativeHoverCloseBackground;
    QColor backgroundPress;
    bool negativePressCloseBackground;

    QColor outlineNormal;
    QColor outlineHover;
    QColor outlinePress;

    QColor baseForeground;
    QColor baseBackground;

private:
    bool decodeButtonOverrideColors();
    void generateButtonBackgroundPalette();
    void generateButtonForegroundPalette();
    void generateButtonOutlinePalette();

    InternalSettingsPtr _decorationSettings;
    KDecoration2::DecorationButtonType _buttonType;
    DecorationButtonBehaviour *_buttonBehaviour;
    DecorationColors *_decorationColors;

    bool _buttonOverrideColorsPresent{false};

    QMap<OverridableButtonColorStates, QColor> _buttonOverrideColorsActive;
    QMap<OverridableButtonColorStates, QColor> _buttonOverrideColorsInactive;
};

}

#endif
