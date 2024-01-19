#ifndef BREEZE_DECORATIONBUTTONCOMMON_H
#define BREEZE_DECORATIONBUTTONCOMMON_H

/*
 * SPDX-FileCopyrightText: 2023-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breeze.h"
#include "breezecommon_export.h"
#include "colortools.h"
#include <KDecoration2/DecorationButton>

namespace Breeze
{

enum struct BREEZECOMMON_EXPORT OverridableButtonColorStates {
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

    QColor text;
    QColor base;
};

/**
 *  @brief Class to generate the colour palette used in a decoration button
 */
class BREEZECOMMON_EXPORT DecorationButtonPalette
{
public:
    DecorationButtonPalette(KDecoration2::DecorationButtonType buttonType);

    void reconfigure(InternalSettingsPtr decorationSettings,
                     DecorationPalette *decorationPalette,
                     QColor textActive,
                     QColor baseActive,
                     QColor textInactive,
                     QColor baseInactive,
                     const bool reconfigureOneGroupOnly = false,
                     const bool oneGrouproupActiveState = true);
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

private:
    void decodeButtonOverrideColors();
    void generateButtonBackgroundPalette(const bool active);
    void generateButtonForegroundPalette(const bool active);
    void generateButtonOutlinePalette(const bool active);

    InternalSettingsPtr _decorationSettings;
    KDecoration2::DecorationButtonType _buttonType;
    DecorationPalette *_decorationPalette;

    bool _buttonOverrideColorsPresentActive{false};
    bool _buttonOverrideColorsPresentInactive{false};

    QMap<OverridableButtonColorStates, QColor> _buttonOverrideColorsActive;
    QMap<OverridableButtonColorStates, QColor> _buttonOverrideColorsInactive;

    std::unique_ptr<DecorationButtonPaletteGroup> _active;
    std::unique_ptr<DecorationButtonPaletteGroup> _inactive;
};

}

#endif
