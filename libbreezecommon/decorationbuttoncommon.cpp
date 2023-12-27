/*
 * SPDX-FileCopyrightText: 2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "decorationbuttoncommon.h"
#include <KColorUtils>

namespace Breeze
{

using KDecoration2::DecorationButtonType;

DecorationButtonBehaviour::DecorationButtonBehaviour()
{
}

void DecorationButtonBehaviour::reconfigure(InternalSettingsPtr decorationSettings)
{
    _decorationSettings = decorationSettings;

    drawBackgroundNormally = false;
    drawBackgroundOnHover = false;
    drawBackgroundOnPress = false;
    drawCloseBackgroundNormally = false;
    drawCloseBackgroundOnHover = false;
    drawCloseBackgroundOnPress = false;
    drawOutlineNormally = false;
    drawOutlineOnHover = false;
    drawOutlineOnPress = false;
    drawCloseOutlineNormally = false;
    drawCloseOutlineOnHover = false;
    drawCloseOutlineOnPress = false;
    drawIconNormally = false;
    drawIconOnHover = false;
    drawIconOnPress = false;
    drawCloseIconNormally = false;
    drawCloseIconOnHover = false;
    drawCloseIconOnPress = false;

    switch (_decorationSettings->alwaysShow()) {
    case InternalSettings::EnumAlwaysShow::Icons:
        drawIconNormally = true;
        drawIconOnHover = true;
        drawIconOnPress = true;
        drawCloseIconNormally = true;
        drawCloseIconOnHover = true;
        drawCloseIconOnPress = true;
        switch (_decorationSettings->alwaysShowIconHighlightUsing()) {
        case InternalSettings::EnumAlwaysShowIconHighlightUsing::Background:
            drawBackgroundNormally = false;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            drawCloseBackgroundNormally = false;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = false;
            drawOutlineOnPress = false;
            drawCloseOutlineNormally = false;
            drawCloseOutlineOnHover = false;
            drawCloseOutlineOnPress = false;
            break;
        case InternalSettings::EnumAlwaysShowIconHighlightUsing::BackgroundAndOutline:
            drawBackgroundNormally = false;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            drawCloseBackgroundNormally = false;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            drawCloseOutlineNormally = false;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
        case InternalSettings::EnumAlwaysShowIconHighlightUsing::Outline:
            drawBackgroundNormally = false;
            drawBackgroundOnHover = false;
            drawBackgroundOnPress = true;
            drawCloseBackgroundNormally = false;
            drawCloseBackgroundOnHover = false;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            drawCloseOutlineNormally = false;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
        }
        break;
    case InternalSettings::EnumAlwaysShow::IconsAndCloseButtonBackground:
        drawIconNormally = true;
        drawIconOnHover = true;
        drawIconOnPress = true;
        drawCloseIconNormally = true;
        drawCloseIconOnHover = true;
        drawCloseIconOnPress = true;
        switch (_decorationSettings->alwaysShowIconCloseButtonBackgroundHighlightUsing()) {
        case InternalSettings::EnumAlwaysShowIconHighlightUsing::Background:
            drawBackgroundNormally = false;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = false;
            drawOutlineOnPress = false;
            drawCloseOutlineNormally = false;
            drawCloseOutlineOnHover = false;
            drawCloseOutlineOnPress = false;
            break;
        case InternalSettings::EnumAlwaysShowIconHighlightUsing::BackgroundAndOutline:
            drawBackgroundNormally = false;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            drawCloseOutlineNormally = false;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
        case InternalSettings::EnumAlwaysShowIconHighlightUsing::Outline:
            drawBackgroundNormally = false;
            drawBackgroundOnHover = false;
            drawBackgroundOnPress = true;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            drawCloseOutlineNormally = false;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
        }
        break;
    case InternalSettings::EnumAlwaysShow::IconsAndCloseButtonOutline:
        drawIconNormally = true;
        drawIconOnHover = true;
        drawIconOnPress = true;
        drawCloseIconNormally = true;
        drawCloseIconOnHover = true;
        drawCloseIconOnPress = true;
        switch (_decorationSettings->alwaysShowIconCloseButtonOutlineHighlightUsing()) {
        case InternalSettings::EnumAlwaysShowIconHighlightUsing::Background:
            drawBackgroundNormally = false;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            drawCloseBackgroundNormally = false;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = false;
            drawOutlineOnPress = false;
            drawCloseOutlineNormally = true;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
        case InternalSettings::EnumAlwaysShowIconHighlightUsing::BackgroundAndOutline:
            drawBackgroundNormally = false;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            drawCloseBackgroundNormally = false;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            drawCloseOutlineNormally = true;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
        case InternalSettings::EnumAlwaysShowIconHighlightUsing::Outline:
            drawBackgroundNormally = false;
            drawBackgroundOnHover = false;
            drawBackgroundOnPress = true;
            drawCloseBackgroundNormally = false;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            drawCloseOutlineNormally = true;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
        }
        break;
    case InternalSettings::EnumAlwaysShow::IconsOutlines:
        drawIconNormally = true;
        drawIconOnHover = true;
        drawIconOnPress = true;
        drawCloseIconNormally = true;
        drawCloseIconOnHover = true;
        drawCloseIconOnPress = true;
        switch (_decorationSettings->alwaysShowIconOutlineHighlightUsing()) {
        case InternalSettings::EnumAlwaysShowIconOutlineHighlightUsing::Background:
            drawBackgroundNormally = false;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            drawCloseBackgroundNormally = false;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = true;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            drawCloseOutlineNormally = true;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
            /*case InternalSettings::EnumAlwaysShowIconOutlineHighlightUsing::DifferentColoredOutline:
                drawBackgroundNormally = false;
                drawBackgroundOnHover = false;
                drawBackgroundOnPress = false;
                drawCloseBackgroundNormally = false;
                drawCloseBackgroundOnHover = false;
                drawCloseBackgroundOnPress = false;
                drawOutlineNormally = true;
                drawOutlineOnHover = true;
                drawOutlineOnPress = true;
                drawOutlineDifferentColoredHover = true;
                drawCloseOutlineNormally = true;
                drawCloseOutlineOnHover = true;
                drawCloseOutlineOnPress = true;
                break;
            case InternalSettings::EnumAlwaysShowIconOutlineHighlightUsing::BackgroundDifferentColoredOutline:
                drawBackgroundNormally = false;
                drawBackgroundOnHover = true;
                drawBackgroundOnPress = true;
                drawCloseBackgroundNormally = false;
                drawCloseBackgroundOnHover = true;
                drawCloseBackgroundOnPress = true;
                drawOutlineNormally = true;
                drawOutlineOnHover = true;
                drawOutlineOnPress = true;
                drawOutlineDifferentColoredHover = true;
                drawCloseOutlineNormally = true;
                drawCloseOutlineOnHover = true;
                drawCloseOutlineOnPress = true;
                break;*/
        }
        break;
    case InternalSettings::EnumAlwaysShow::IconsOutlinesAndCloseButtonBackground:
        drawIconNormally = true;
        drawIconOnHover = true;
        drawIconOnPress = true;
        drawCloseIconNormally = true;
        drawCloseIconOnHover = true;
        drawCloseIconOnPress = true;
        switch (_decorationSettings->alwaysShowIconOutlineCloseButtonBackgroundHighlightUsing()) {
        case InternalSettings::EnumAlwaysShowIconOutlineCloseButtonBackgroundHighlightUsing::Background:
            drawBackgroundNormally = false;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = true;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            drawCloseOutlineNormally = true;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
            /*case InternalSettings::EnumNormallyShowIconOutlineCloseButtonBackgroundHighlightUsing::DifferentColoredOutline:
                drawBackgroundNormally = false;
                drawBackgroundOnHover = false;
                drawBackgroundOnPress = false;
                drawCloseBackgroundNormally = true;
                drawCloseBackgroundOnHover = true;
                drawCloseBackgroundOnPress = true;
                drawOutlineNormally = true;
                drawOutlineOnHover = true;
                drawOutlineOnPress = true;
                drawOutlineDifferentColoredHover = true;
                drawCloseOutlineNormally = true;
                drawCloseOutlineOnHover = true;
                drawCloseOutlineOnPress = true;
                break;
            case InternalSettings::EnumNormallyShowIconOutlineCloseButtonBackgroundHighlightUsing::BackgroundDifferentColoredOutline:
                drawBackgroundNormally = false;
                drawBackgroundOnHover = true;
                drawBackgroundOnPress = true;
                drawCloseBackgroundNormally = true;
                drawCloseBackgroundOnHover = true;
                drawCloseBackgroundOnPress = true;
                drawOutlineNormally = true;
                drawOutlineOnHover = true;
                drawOutlineOnPress = true;
                drawOutlineDifferentColoredHover = true;
                drawCloseOutlineNormally = true;
                drawCloseOutlineOnHover = true;
                drawCloseOutlineOnPress = true;
                break;*/
        }
        break;
    case InternalSettings::EnumAlwaysShow::IconsAndBackgrounds:
        drawIconNormally = true;
        drawIconOnHover = true;
        drawIconOnPress = true;
        drawCloseIconNormally = true;
        drawCloseIconOnHover = true;
        drawCloseIconOnPress = true;
        switch (_decorationSettings->alwaysShowIconBackgroundHighlightUsing()) {
        /*case InternalSettings::EnumAlwaysShowIconBackgroundHighlightUsing::Outline:
            drawBackgroundNormally = true;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            drawCloseOutlineNormally = false;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;*/
        case InternalSettings::EnumAlwaysShowIconBackgroundHighlightUsing::DifferentColoredBackground:
            drawBackgroundNormally = true;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            // drawBackgroundDifferentColoredHover = true;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = false;
            drawOutlineOnPress = false;
            drawCloseOutlineNormally = false;
            drawCloseOutlineOnHover = false;
            drawCloseOutlineOnPress = false;
            break;
        case InternalSettings::EnumAlwaysShowIconBackgroundHighlightUsing::DifferentColoredBackgroundOutline:
            drawBackgroundNormally = true;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            // drawBackgroundDifferentColoredHover = true;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            drawCloseOutlineNormally = false;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
        }
        break;
    case InternalSettings::EnumAlwaysShow::IconsBackgroundsAndOutlines:
        drawIconNormally = true;
        drawIconOnHover = true;
        drawIconOnPress = true;
        drawCloseIconNormally = true;
        drawCloseIconOnHover = true;
        drawCloseIconOnPress = true;
        switch (_decorationSettings->alwaysShowIconBackgroundOutlineHighlightUsing()) {
        case InternalSettings::EnumAlwaysShowIconBackgroundOutlineHighlightUsing::DifferentColoredBackground:
            drawBackgroundNormally = true;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            // drawBackgroundDifferentColoredHover = true;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = true;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            drawCloseOutlineNormally = true;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
            /*case InternalSettings::EnumAlwaysShowIconBackgroundOutlineHighlightUsing::DifferentColoredOutline:
                drawBackgroundNormally = true;
                drawBackgroundOnHover = true;
                drawBackgroundOnPress = true;
                drawCloseBackgroundNormally = true;
                drawCloseBackgroundOnHover = true;
                drawCloseBackgroundOnPress = true;
                drawOutlineNormally = true;
                drawOutlineOnHover = true;
                drawOutlineOnPress = true;
                drawOutlineDifferentColoredHover = true;
                drawCloseOutlineNormally = true;
                drawCloseOutlineOnHover = true;
                drawCloseOutlineOnPress = true;
                break;*/
            /*case InternalSettings::EnumAlwaysShowIconBackgroundOutlineHighlightUsing::DifferentColoredBackgroundDifferentColoredOutline:
                drawBackgroundNormally = true;
                drawBackgroundOnHover = true;
                drawBackgroundOnPress = true;
                drawBackgroundDifferentColoredHover = true;
                drawCloseBackgroundNormally = true;
                drawCloseBackgroundOnHover = true;
                drawCloseBackgroundOnPress = true;
                drawOutlineNormally = true;
                drawOutlineOnHover = true;
                drawOutlineOnPress = true;
                drawOutlineDifferentColoredHover = true;
                drawCloseOutlineNormally = true;
                drawCloseOutlineOnHover = true;
                drawCloseOutlineOnPress = true;
                break;*/
        }
        break;
    case InternalSettings::EnumAlwaysShow::Backgrounds:
        drawIconNormally = false;
        drawIconOnHover = true;
        drawIconOnPress = true;
        drawCloseIconNormally = false;
        drawCloseIconOnHover = true;
        drawCloseIconOnPress = true;
        switch (_decorationSettings->alwaysShowBackgroundHighlightUsing()) {
        case InternalSettings::EnumAlwaysShowBackgroundHighlightUsing::Icon:
            drawBackgroundNormally = true;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            // drawBackgroundDifferentColoredHover = false;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = false;
            drawOutlineOnPress = false;
            // drawOutlineDifferentColoredHover = false;
            drawCloseOutlineNormally = false;
            drawCloseOutlineOnHover = false;
            drawCloseOutlineOnPress = false;
            break;
        /*case InternalSettings::EnumAlwaysShowBackgroundHighlightUsing::IconOutline:
            drawBackgroundNormally = true;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            //drawBackgroundDifferentColoredHover = false;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            //drawOutlineDifferentColoredHover = false;
            drawCloseOutlineNormally = false;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;*/
        case InternalSettings::EnumAlwaysShowBackgroundHighlightUsing::IconDifferentColoredBackground:
            drawBackgroundNormally = true;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            // drawBackgroundDifferentColoredHover = true;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = false;
            drawOutlineOnPress = false;
            // drawOutlineDifferentColoredHover = false;
            drawCloseOutlineNormally = false;
            drawCloseOutlineOnHover = false;
            drawCloseOutlineOnPress = false;
            break;
        case InternalSettings::EnumAlwaysShowBackgroundHighlightUsing::IconOutlineDifferentColoredBackground:
            drawBackgroundNormally = true;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            // drawBackgroundDifferentColoredHover = true;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            drawOutlineNormally = false;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            // drawOutlineDifferentColoredHover = false;
            drawCloseOutlineNormally = false;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
        }
        break;
    case InternalSettings::EnumAlwaysShow::BackgroundsAndOutlines:
        drawIconNormally = false;
        drawIconOnHover = true;
        drawIconOnPress = true;
        drawCloseIconNormally = false;
        drawCloseIconOnHover = true;
        drawCloseIconOnPress = true;
        switch (_decorationSettings->alwaysShowBackgroundOutlineHighlightUsing()) {
        case InternalSettings::EnumAlwaysShowBackgroundOutlineHighlightUsing::Icon:
            drawBackgroundNormally = true;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;
            // drawBackgroundDifferentColoredHover = false;
            drawOutlineNormally = true;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            // drawOutlineDifferentColoredHover = false;
            drawCloseOutlineNormally = true;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
        case InternalSettings::EnumAlwaysShowBackgroundOutlineHighlightUsing::IconDifferentColoredBackground:
            drawBackgroundNormally = true;
            drawBackgroundOnHover = true;
            drawBackgroundOnPress = true;
            // drawBackgroundDifferentColoredHover = true;
            drawCloseBackgroundNormally = true;
            drawCloseBackgroundOnHover = true;
            drawCloseBackgroundOnPress = true;

            drawOutlineNormally = true;
            drawOutlineOnHover = true;
            drawOutlineOnPress = true;
            // drawOutlineDifferentColoredHover = false;
            drawCloseOutlineNormally = true;
            drawCloseOutlineOnHover = true;
            drawCloseOutlineOnPress = true;
            break;
            /*case InternalSettings::EnumAlwaysShowBackgroundOutlineHighlightUsing::IconDifferentColoredOutline:
                drawBackgroundNormally = true;
                drawBackgroundOnHover = true;
                drawBackgroundOnPress = true;
                drawCloseBackgroundNormally = true;
                drawBackgroundDifferentColoredHover = false;
                drawCloseBackgroundOnHover = true;
                drawCloseBackgroundOnPress = true;

                drawOutlineNormally = true;
                drawOutlineOnHover = true;
                drawOutlineOnPress = true;
                drawOutlineDifferentColoredHover = true;
                drawCloseOutlineNormally = true;
                drawCloseOutlineOnHover = true;
                drawCloseOutlineOnPress = true;
                break;
            case InternalSettings::EnumAlwaysShowBackgroundOutlineHighlightUsing::IconDifferentColoredBackgroundDifferentColoredOutline:
                drawBackgroundNormally = true;
                drawBackgroundOnHover = true;
                drawBackgroundOnPress = true;
                drawBackgroundDifferentColoredHover = true;
                drawCloseBackgroundNormally = true;
                drawCloseBackgroundOnHover = true;
                drawCloseBackgroundOnPress = true;
                drawOutlineNormally = true;
                drawOutlineOnHover = true;
                drawOutlineOnPress = true;
                drawOutlineDifferentColoredHover = true;
                drawCloseOutlineNormally = true;
                drawCloseOutlineOnHover = true;
                drawCloseOutlineOnPress = true;
                break;*/
        }
        break;
    }
}

DecorationButtonPalette::DecorationButtonPalette(KDecoration2::DecorationButtonType buttonType)
    : _buttonType(buttonType)
{
}

void DecorationButtonPalette::reconfigure(InternalSettingsPtr decorationSettings,
                                          DecorationButtonBehaviour *buttonBehaviour,
                                          DecorationColors *decorationColors,
                                          QColor baseForeground,
                                          QColor baseBackground)
{
    _decorationSettings = decorationSettings;
    _buttonBehaviour = buttonBehaviour;
    _decorationColors = decorationColors;
    this->baseForeground = baseForeground;
    this->baseBackground = baseBackground;

    _buttonOverrideColorsPresent = decodeButtonOverrideColors();
    generateButtonBackgroundPalette();
    generateButtonForegroundPalette();
    generateButtonOutlinePalette();
}

bool DecorationButtonPalette::decodeButtonOverrideColors()
{
    uint32_t _buttonOverrideColorsFlags = 0;
    QList<int> buttonOverrideColorsList;
    switch (_buttonType) {
    case DecorationButtonType::Close:
        _buttonOverrideColorsFlags = _decorationSettings->buttonOverrideColorsCloseFlags();
        buttonOverrideColorsList = _decorationSettings->buttonOverrideColorsClose();
        break;
    case DecorationButtonType::Maximize:
        _buttonOverrideColorsFlags = _decorationSettings->buttonOverrideColorsMaximizeFlags();
        buttonOverrideColorsList = _decorationSettings->buttonOverrideColorsMaximize();
        break;
    case DecorationButtonType::Minimize:
        _buttonOverrideColorsFlags = _decorationSettings->buttonOverrideColorsMinimizeFlags();
        buttonOverrideColorsList = _decorationSettings->buttonOverrideColorsMinimize();
        break;
    case DecorationButtonType::ContextHelp:
        _buttonOverrideColorsFlags = _decorationSettings->buttonOverrideColorsHelpFlags();
        buttonOverrideColorsList = _decorationSettings->buttonOverrideColorsHelp();
        break;
    case DecorationButtonType::Shade:
        _buttonOverrideColorsFlags = _decorationSettings->buttonOverrideColorsShadeFlags();
        buttonOverrideColorsList = _decorationSettings->buttonOverrideColorsShade();
        break;
    case DecorationButtonType::OnAllDesktops:
        _buttonOverrideColorsFlags = _decorationSettings->buttonOverrideColorsAllDesktopsFlags();
        buttonOverrideColorsList = _decorationSettings->buttonOverrideColorsAllDesktops();
        break;
    case DecorationButtonType::KeepBelow:
        _buttonOverrideColorsFlags = _decorationSettings->buttonOverrideColorsKeepBelowFlags();
        buttonOverrideColorsList = _decorationSettings->buttonOverrideColorsKeepBelow();
        break;
    case DecorationButtonType::KeepAbove:
        _buttonOverrideColorsFlags = _decorationSettings->buttonOverrideColorsKeepAboveFlags();
        buttonOverrideColorsList = _decorationSettings->buttonOverrideColorsKeepAbove();
        break;
    case DecorationButtonType::ApplicationMenu:
        _buttonOverrideColorsFlags = _decorationSettings->buttonOverrideColorsApplicationMenuFlags();
        buttonOverrideColorsList = _decorationSettings->buttonOverrideColorsApplicationMenu();
        break;
    case DecorationButtonType::Menu:
        _buttonOverrideColorsFlags = _decorationSettings->buttonOverrideColorsMenuFlags();
        buttonOverrideColorsList = _decorationSettings->buttonOverrideColorsMenu();
        break;
    default:
        break;
    }

    _buttonOverrideColorsActive.clear();
    _buttonOverrideColorsInactive.clear();

    if (!_buttonOverrideColorsFlags)
        return false;

    uint32_t bitMask = 0x00000001;
    uint32_t validColorsFlagsBits = 0x0EEE0EEE;
    uint32_t validColorsFlagsBitsActive = 0x0000FFFF;
    QMap<OverridableButtonColorStates, QColor> *output;
    int colorsListIndex = 0;
    int outputIndex = 0;
    bool buttonHasOverrideColors = false;
    for (uint32_t i = 0; i < (sizeof(validColorsFlagsBits) * CHAR_BIT); i++, bitMask = bitMask << 1) {
        if (validColorsFlagsBits & bitMask) {
            if (validColorsFlagsBitsActive & bitMask) {
                output = &_buttonOverrideColorsActive;
            } else {
                output = &_buttonOverrideColorsInactive;
            }

            if (_buttonOverrideColorsFlags & bitMask) { // if the current bit in colorsFlags is 1
                buttonHasOverrideColors = true;
                if (buttonOverrideColorsList.count()
                    && colorsListIndex < buttonOverrideColorsList.count()) { // this if is to prevent against an unlikely corruption situation when
                    // colorsSet and colorsist are out of sync
                    QRgb color = QRgb(static_cast<QRgb>(buttonOverrideColorsList[colorsListIndex++]));
                    QColor qcolor(color);
                    qcolor.setAlpha(qAlpha(color));
                    output->insert(static_cast<OverridableButtonColorStates>(outputIndex), qcolor);
                }
            } else {
                output->insert(static_cast<OverridableButtonColorStates>(outputIndex), QColor());
            }

            outputIndex++;
        }
    }
    return buttonHasOverrideColors;
}

void DecorationButtonPalette::generateButtonBackgroundPalette()
{
    backgroundNormal = QColor();
    backgroundHover = QColor();
    backgroundPress = QColor();

    const bool negativeCloseCategory( // whether the button background colour is in the general negative category as selected in the "Background & Outline
                                      // Colours" combobox
        _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
        || _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
        || _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights);

    negativeNormalCloseBackground = false;
    negativeHoverCloseBackground = false;
    negativePressCloseBackground = false;

    bool defaultButton =
        true; // flag indicates the button has standard colours for the behaviour and selected colour (i.e. is not a close/max/min with special colours)
    bool *drawBackgroundNormally =
        (_buttonType == DecorationButtonType::Close) ? &_buttonBehaviour->drawCloseBackgroundNormally : &_buttonBehaviour->drawBackgroundNormally;
    bool *drawBackgroundOnHover =
        (_buttonType == DecorationButtonType::Close) ? &_buttonBehaviour->drawCloseBackgroundOnHover : &_buttonBehaviour->drawBackgroundOnHover;
    bool *drawBackgroundOnPress =
        (_buttonType == DecorationButtonType::Close) ? &_buttonBehaviour->drawCloseBackgroundOnPress : &_buttonBehaviour->drawBackgroundOnPress;

    // set normal, hover and press colours
    if (_decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::Accent
        || _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
        || _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
        if (_decorationSettings->translucentButtonBackgrounds()) {
            if (_buttonType == DecorationButtonType::Close && negativeCloseCategory) {
                defaultButton = false;
                if (*drawBackgroundNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        negativeNormalCloseBackground = true;
                        backgroundNormal = _decorationColors->negativeReducedOpacityBackground();
                        if (*drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = _decorationColors->negativeReducedOpacityOutline();
                        }
                        if (*drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = _decorationColors->fullySaturatedNegative();
                        }
                    } else {
                        backgroundNormal = _decorationColors->buttonReducedOpacityBackground();
                        if (*drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = _decorationColors->negativeReducedOpacityOutline();
                        }
                        if (*drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = _decorationColors->fullySaturatedNegative();
                        }
                    }
                } else {
                    if (*drawBackgroundOnHover) {
                        negativeHoverCloseBackground = true;
                        backgroundHover = _decorationColors->negativeReducedOpacityBackground();
                    }
                    if (*drawBackgroundOnPress) {
                        negativePressCloseBackground = true;
                        backgroundPress = _decorationColors->negativeReducedOpacityOutline();
                    }
                }
            } else if (_buttonType == DecorationButtonType::Minimize
                       && _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (*drawBackgroundNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        backgroundNormal = _decorationColors->neutralReducedOpacityBackground();
                        if (*drawBackgroundOnHover)
                            backgroundHover = _decorationColors->neutralReducedOpacityOutline();
                        if (*drawBackgroundOnPress)
                            backgroundPress = _decorationColors->neutral();
                    } else {
                        backgroundNormal = _decorationColors->buttonReducedOpacityBackground();
                        if (*drawBackgroundOnHover)
                            backgroundHover = _decorationColors->neutralReducedOpacityOutline();
                        if (*drawBackgroundOnPress)
                            backgroundPress = _decorationColors->neutral();
                    }
                } else {
                    if (*drawBackgroundOnHover)
                        backgroundHover = _decorationColors->neutralReducedOpacityBackground();
                    if (*drawBackgroundOnPress)
                        backgroundPress = _decorationColors->neutralReducedOpacityOutline();
                }
            } else if (_buttonType == DecorationButtonType::Maximize
                       && _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (*drawBackgroundNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        backgroundNormal = _decorationColors->positiveReducedOpacityBackground();
                        if (*drawBackgroundOnHover)
                            backgroundHover = _decorationColors->positiveReducedOpacityOutline();
                        if (*drawBackgroundOnPress)
                            backgroundPress = _decorationColors->positive();
                    } else {
                        backgroundNormal = _decorationColors->buttonReducedOpacityBackground();
                        if (*drawBackgroundOnHover)
                            backgroundHover = _decorationColors->positiveReducedOpacityOutline();
                        if (*drawBackgroundOnPress)
                            backgroundPress = _decorationColors->positive();
                    }
                } else {
                    if (*drawBackgroundOnHover)
                        backgroundHover = _decorationColors->positiveReducedOpacityBackground();
                    if (*drawBackgroundOnPress)
                        backgroundPress = _decorationColors->positiveReducedOpacityOutline();
                }
            }

            if (defaultButton) {
                if (*drawBackgroundNormally) {
                    backgroundNormal = _decorationColors->buttonReducedOpacityBackground();
                    if (*drawBackgroundOnHover)
                        backgroundHover = _decorationColors->buttonReducedOpacityOutline();
                    if (*drawBackgroundOnPress)
                        backgroundPress = _decorationColors->buttonFocus();
                } else {
                    if (*drawBackgroundOnHover)
                        backgroundHover = _decorationColors->buttonReducedOpacityBackground();
                    if (*drawBackgroundOnPress)
                        backgroundPress = _decorationColors->buttonReducedOpacityOutline();
                }
            }
        } else { // accent but not translucent
            if (_buttonType == DecorationButtonType::Close && negativeCloseCategory) {
                defaultButton = false;
                if (*drawBackgroundNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        negativeNormalCloseBackground = true;
                        backgroundNormal = _decorationColors->negative();
                        if (*drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = _decorationColors->negativeSaturated();
                        }
                        if (*drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = _decorationColors->negativeLessSaturated();
                        }
                    } else {
                        backgroundNormal = *drawBackgroundNormally ? KColorUtils::mix(baseBackground, _decorationColors->buttonHover(), 0.8)
                                                                   : _decorationColors->buttonHover();
                        if (*drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = _decorationColors->negativeSaturated();
                        }
                        if (*drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = _decorationColors->negativeLessSaturated();
                        }
                    }
                } else {
                    if (*drawBackgroundOnHover) {
                        negativeHoverCloseBackground = true;
                        backgroundHover = _decorationColors->negative();
                    }
                    if (*drawBackgroundOnPress) {
                        negativePressCloseBackground = true;
                        backgroundPress = _decorationColors->negativeSaturated();
                    }
                }
            } else if (_buttonType == DecorationButtonType::Minimize
                       && _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (*drawBackgroundNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        backgroundNormal = _decorationColors->neutralLessSaturated();
                        if (*drawBackgroundOnHover)
                            backgroundHover = _decorationColors->neutral();
                        if (*drawBackgroundOnPress)
                            backgroundPress = _decorationColors->neutralSaturated();
                    } else {
                        backgroundNormal = KColorUtils::mix(baseBackground, _decorationColors->buttonHover(), 0.8);
                        if (*drawBackgroundOnHover)
                            backgroundHover = _decorationColors->neutral();
                        if (*drawBackgroundOnPress)
                            backgroundPress = _decorationColors->neutralSaturated();
                    }
                } else {
                    if (*drawBackgroundOnHover)
                        backgroundHover = _decorationColors->neutralLessSaturated();
                    if (*drawBackgroundOnPress)
                        backgroundPress = _decorationColors->neutral();
                }
            } else if (_buttonType == DecorationButtonType::Maximize
                       && _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (*drawBackgroundNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        backgroundNormal = _decorationColors->positiveLessSaturated();
                        if (*drawBackgroundOnHover)
                            backgroundHover = _decorationColors->positive();
                        if (*drawBackgroundOnPress)
                            backgroundPress = _decorationColors->positiveSaturated();
                    } else {
                        backgroundNormal = KColorUtils::mix(baseBackground, _decorationColors->buttonHover(), 0.8);
                        if (*drawBackgroundOnHover)
                            backgroundHover = _decorationColors->positive();
                        if (*drawBackgroundOnPress)
                            backgroundPress = _decorationColors->positiveSaturated();
                    }
                } else {
                    if (*drawBackgroundOnHover)
                        backgroundHover = _decorationColors->positiveLessSaturated();
                    if (*drawBackgroundOnPress)
                        backgroundPress = _decorationColors->positive();
                }
            }

            if (defaultButton) {
                if (*drawBackgroundNormally) {
                    backgroundNormal = KColorUtils::mix(baseBackground, _decorationColors->buttonHover(), 0.8);
                    if (*drawBackgroundOnHover)
                        backgroundHover = _decorationColors->buttonHover();
                    if (*drawBackgroundOnPress)
                        backgroundPress = _decorationColors->buttonFocus();
                } else {
                    if (*drawBackgroundOnHover)
                        backgroundHover = _decorationColors->buttonHover();
                    if (*drawBackgroundOnPress)
                        backgroundPress = _decorationColors->buttonFocus();
                }
            }
        }

    } else {
        if (_decorationSettings->translucentButtonBackgrounds()) { // titlebar text color, translucent
            if (_buttonType == DecorationButtonType::Close && negativeCloseCategory) {
                defaultButton = false;
                if (*drawBackgroundNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        negativeNormalCloseBackground = true;
                        backgroundNormal = _decorationColors->negativeReducedOpacityBackground();
                        if (*drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = _decorationColors->negativeReducedOpacityOutline();
                        }
                        if (*drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = _decorationColors->negativeReducedOpacityLessSaturatedBackground();
                        }
                    } else {
                        backgroundNormal = ColorTools::alphaMix(baseForeground, qMin(_decorationSettings->translucentButtonBackgroundsOpacity() * 0.3, 1.0));
                        if (*drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = _decorationColors->negativeReducedOpacityOutline();
                        }
                        if (*drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = _decorationColors->negativeReducedOpacityBackground();
                        }
                    }
                } else {
                    if (*drawBackgroundOnHover) {
                        negativeHoverCloseBackground = true;
                        backgroundHover = _decorationColors->negativeReducedOpacityBackground();
                    }
                    if (*drawBackgroundOnPress) {
                        negativePressCloseBackground = true;
                        backgroundPress = _decorationColors->negativeReducedOpacityOutline();
                    }
                }
            }

            if (defaultButton) {
                if (*drawBackgroundNormally) {
                    backgroundNormal = ColorTools::alphaMix(baseForeground, qMin(_decorationSettings->translucentButtonBackgroundsOpacity() * 0.3, 1.0));
                    if (*drawBackgroundOnHover)
                        backgroundHover = ColorTools::alphaMix(baseForeground, qMin(_decorationSettings->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                    if (*drawBackgroundOnPress)
                        backgroundPress = ColorTools::alphaMix(baseForeground, qMin(_decorationSettings->translucentButtonBackgroundsOpacity() * 0.7, 1.0));
                } else {
                    if (*drawBackgroundOnHover)
                        backgroundHover = ColorTools::alphaMix(baseForeground, qMin(_decorationSettings->translucentButtonBackgroundsOpacity() * 0.3, 1.0));
                    if (*drawBackgroundOnPress)
                        backgroundPress = ColorTools::alphaMix(baseForeground, qMin(_decorationSettings->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                }
            }
        } else { // titlebar text color, not translucent
            if (_buttonType == DecorationButtonType::Close && negativeCloseCategory) {
                defaultButton = false;
                if (_buttonBehaviour->drawBackgroundNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        if (*drawBackgroundNormally) {
                            negativeNormalCloseBackground = true;
                            backgroundNormal = _decorationColors->negative();
                        }
                        if (*drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = _decorationColors->negativeSaturated();
                        }
                        if (*drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = _decorationColors->negativeLessSaturated();
                        }
                    } else {
                        backgroundNormal = KColorUtils::mix(baseBackground, baseForeground, 0.3);
                        if (*drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = _decorationColors->negativeSaturated();
                        }
                        if (*drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = _decorationColors->negativeLessSaturated();
                        }
                    }
                } else if (_buttonBehaviour->drawCloseBackgroundNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        if (*drawBackgroundNormally) {
                            negativeNormalCloseBackground = true;
                            backgroundNormal = _decorationColors->negative();
                        }
                        if (*drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = _decorationColors->negativeSaturated();
                        }
                        if (*drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = _decorationColors->negativeLessSaturated();
                        }
                    } else {
                        if (*drawBackgroundNormally)
                            backgroundNormal = baseForeground;
                        if (*drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = _decorationColors->negativeSaturated();
                        }
                        if (*drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = _decorationColors->negativeLessSaturated();
                        }
                    }
                } else {
                    if (*drawBackgroundOnHover) {
                        negativeHoverCloseBackground = true;
                        backgroundHover = _decorationColors->negative();
                    }
                    if (*drawBackgroundOnPress) {
                        negativePressCloseBackground = true;
                        backgroundPress = _decorationColors->negativeSaturated();
                    }
                }
            }

            if (defaultButton) {
                if (_buttonBehaviour->drawBackgroundNormally) {
                    if (*drawBackgroundNormally)
                        backgroundNormal = KColorUtils::mix(baseBackground, baseForeground, 0.3);
                    if (*drawBackgroundOnHover)
                        backgroundHover = KColorUtils::mix(baseBackground, baseForeground, 0.6);
                    if (*drawBackgroundOnPress)
                        backgroundPress = baseForeground;
                } else if (_buttonBehaviour->drawCloseBackgroundNormally && _buttonType == DecorationButtonType::Close) {
                    if (*drawBackgroundNormally)
                        backgroundNormal = baseForeground;
                    if (*drawBackgroundOnHover)
                        backgroundHover = KColorUtils::mix(baseBackground, baseForeground, 0.6);
                    if (*drawBackgroundOnPress)
                        backgroundPress = KColorUtils::mix(baseBackground, baseForeground, 0.3);
                } else {
                    if (*drawBackgroundOnHover)
                        backgroundHover = baseForeground;
                    if (*drawBackgroundOnPress)
                        backgroundPress = KColorUtils::mix(baseBackground, baseForeground, 0.3);
                }
            }
        }
    }

    if (_buttonOverrideColorsPresent) {
        if (_buttonOverrideColorsActive.value(OverridableButtonColorStates::BackgroundNormal).isValid() && *drawBackgroundNormally) {
            backgroundNormal = _buttonOverrideColorsActive.value(OverridableButtonColorStates::BackgroundNormal);
        }
        if (_buttonOverrideColorsActive.value(OverridableButtonColorStates::BackgroundHover).isValid() && *drawBackgroundOnHover) {
            backgroundHover = _buttonOverrideColorsActive.value(OverridableButtonColorStates::BackgroundHover);
        }
        if (_buttonOverrideColorsActive.value(OverridableButtonColorStates::BackgroundPress).isValid() && *drawBackgroundOnPress) {
            backgroundPress = _buttonOverrideColorsActive.value(OverridableButtonColorStates::BackgroundPress);
        }
    }

    // low contrast correction between background and titlebar
    if (_decorationSettings->adjustBackgroundColorOnPoorContrast()) {
        if (backgroundNormal.isValid() && KColorUtils::contrastRatio(backgroundNormal, baseBackground) < 1.3) {
            backgroundNormal = KColorUtils::mix(backgroundNormal, baseForeground, 0.3);
        }

        if (backgroundHover.isValid() && KColorUtils::contrastRatio(backgroundHover, baseBackground) < 1.3) {
            backgroundHover = KColorUtils::mix(backgroundHover, baseForeground, 0.3);
        }
        if (backgroundPress.isValid() && KColorUtils::contrastRatio(backgroundPress, baseBackground) < 1.3) {
            backgroundPress = KColorUtils::mix(backgroundPress, baseForeground, 0.3);
        }
    }
}

void DecorationButtonPalette::generateButtonForegroundPalette()
{
    foregroundNormal = QColor();
    foregroundHover = QColor();
    foregroundPress = QColor();

    bool defaultButton =
        true; // flag indicates the button has standard colours for the behaviour and selected colour (i.e. is not a close/max/min with special colours)
    bool closeForegroundNormalOnNegativeBackgroundIsDefault = false;

    bool *drawBackgroundNormally =
        (_buttonType == DecorationButtonType::Close) ? &_buttonBehaviour->drawCloseBackgroundNormally : &_buttonBehaviour->drawBackgroundNormally;
    bool *drawBackgroundOnHover =
        (_buttonType == DecorationButtonType::Close) ? &_buttonBehaviour->drawCloseBackgroundOnHover : &_buttonBehaviour->drawBackgroundOnHover;
    bool *drawBackgroundOnPress =
        (_buttonType == DecorationButtonType::Close) ? &_buttonBehaviour->drawCloseBackgroundOnPress : &_buttonBehaviour->drawBackgroundOnPress;

    bool *drawIconNormally = (_buttonType == DecorationButtonType::Close) ? &_buttonBehaviour->drawCloseIconNormally : &_buttonBehaviour->drawIconNormally;
    bool *drawIconOnHover = (_buttonType == DecorationButtonType::Close) ? &_buttonBehaviour->drawCloseIconOnHover : &_buttonBehaviour->drawIconOnHover;
    bool *drawIconOnPress = (_buttonType == DecorationButtonType::Close) ? &_buttonBehaviour->drawCloseIconOnPress : &_buttonBehaviour->drawIconOnPress;

    if (_buttonType == DecorationButtonType::Close && (negativeNormalCloseBackground || negativeHoverCloseBackground || negativePressCloseBackground)
        && _decorationSettings->closeIconNegativeBackground() != InternalSettings::EnumCloseIconNegativeBackground::AsSelected) {
        defaultButton = false;

        if (_decorationSettings->closeIconNegativeBackground() == InternalSettings::EnumCloseIconNegativeBackground::White) {
            if (*drawIconNormally)
                foregroundNormal = Qt::GlobalColor::white;
            if (*drawIconOnHover)
                foregroundHover = Qt::GlobalColor::white;
            if (*drawIconOnPress)
                foregroundPress = Qt::GlobalColor::white;
        } else { // White on hovered pressed
            if (*drawIconOnHover)
                foregroundHover = Qt::GlobalColor::white;
            if (*drawIconOnPress)
                foregroundPress = Qt::GlobalColor::white;

            // get foregroundNormal
            if (_decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::TitlebarText) {
                if ((_decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
                     || _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarText)
                    && !_decorationSettings->translucentButtonBackgrounds()) {
                    if (*drawBackgroundNormally) {
                        if (*drawIconNormally)
                            foregroundNormal = baseBackground;
                    } else {
                        if (*drawIconNormally)
                            foregroundNormal = baseForeground;
                    }
                } else {
                    if (*drawIconNormally)
                        foregroundNormal = baseForeground;
                }
            } else if (_decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose
                       || _decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::AccentNegativeClose
                       || _decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
                if (*drawIconNormally)
                    foregroundNormal = _decorationColors->negativeSaturated();
            } else { // Normal icon colour is accent colour - treat as default button
                closeForegroundNormalOnNegativeBackgroundIsDefault = true;
            }
        }
    }

    if (_buttonType == DecorationButtonType::Close
        && (_decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose
            || _decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::AccentNegativeClose
            || _decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::AccentTrafficLights)) {
        defaultButton = false;
        if (*drawIconNormally && !foregroundNormal.isValid())
            foregroundNormal = _decorationColors->negativeSaturated();
        if (*drawIconOnHover && !foregroundHover.isValid())
            foregroundHover = _decorationColors->negativeSaturated();
        if (*drawIconOnPress && !foregroundPress.isValid())
            foregroundPress = _decorationColors->negativeSaturated();
    } else if (_buttonType == DecorationButtonType::Maximize
               && (_decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::AccentTrafficLights)) {
        defaultButton = false;
        if (*drawIconNormally)
            foregroundNormal = _decorationColors->positiveSaturated();
        if (*drawIconOnHover)
            foregroundHover = _decorationColors->positiveSaturated();
        if (*drawIconOnPress)
            foregroundPress = _decorationColors->positiveSaturated();
    } else if (_buttonType == DecorationButtonType::Minimize
               && (_decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::AccentTrafficLights)) {
        defaultButton = false;
        if (*drawIconNormally)
            foregroundNormal = _decorationColors->neutral();
        if (*drawIconOnHover)
            foregroundHover = _decorationColors->neutral();
        if (*drawIconOnPress)
            foregroundPress = _decorationColors->neutral();
    }

    if (defaultButton || closeForegroundNormalOnNegativeBackgroundIsDefault) {
        if ((_decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
             || _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarText)
            && !_decorationSettings->translucentButtonBackgrounds()
            && (_decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose
                || _decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::TitlebarText)) {
            if (*drawBackgroundNormally) {
                if (*drawIconNormally && !foregroundNormal.isValid())
                    foregroundNormal = baseBackground;
                if (*drawIconOnHover && !foregroundHover.isValid())
                    foregroundHover = baseBackground;
                if (*drawIconOnPress && !foregroundPress.isValid())
                    foregroundPress = baseBackground;
            } else {
                if (*drawIconNormally && !foregroundNormal.isValid())
                    foregroundNormal = baseForeground;
                if (*drawIconOnHover && !foregroundHover.isValid())
                    foregroundHover = *drawBackgroundOnHover ? baseBackground : baseForeground;
                if (*drawIconOnPress && !foregroundPress.isValid())
                    foregroundPress = *drawBackgroundOnPress ? baseBackground : baseForeground;
            }
        } else {
            if (_decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose
                || _decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::TitlebarText) {
                if (*drawIconNormally && !foregroundNormal.isValid())
                    foregroundNormal = baseForeground;
                if (*drawIconOnHover && !foregroundHover.isValid())
                    foregroundHover = baseForeground;
                if (*drawIconOnPress && !foregroundPress.isValid())
                    foregroundPress = baseForeground;
            } else if (_decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::AccentNegativeClose
                       || _decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::Accent
                       || _decorationSettings->buttonIconColors() == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
                if (*drawIconNormally && !foregroundNormal.isValid())
                    foregroundNormal = _decorationColors->highlight();
                if (*drawIconOnHover && !foregroundHover.isValid())
                    foregroundHover = _decorationColors->highlight();
                if (*drawIconOnPress && !foregroundPress.isValid())
                    foregroundPress = _decorationColors->highlight();
            }
        }
    }

    if (_buttonOverrideColorsPresent) {
        if (_buttonOverrideColorsActive.value(OverridableButtonColorStates::IconNormal).isValid() && *drawIconNormally) {
            foregroundNormal = _buttonOverrideColorsActive.value(OverridableButtonColorStates::IconNormal);
        }
        if (_buttonOverrideColorsActive.value(OverridableButtonColorStates::IconHover).isValid() && *drawIconOnHover) {
            foregroundHover = _buttonOverrideColorsActive.value(OverridableButtonColorStates::IconHover);
        }
        if (_buttonOverrideColorsActive.value(OverridableButtonColorStates::IconPress).isValid() && *drawIconOnPress) {
            foregroundPress = _buttonOverrideColorsActive.value(OverridableButtonColorStates::IconPress);
        }
    }

    if (_decorationSettings->blackWhiteIconOnPoorContrast()) {
        if (foregroundNormal.isValid() && backgroundNormal.isValid())
            ColorTools::getHigherContrastForegroundColor(foregroundNormal, backgroundNormal, 2.3, foregroundNormal);

        if (foregroundHover.isValid() && backgroundHover.isValid())
            ColorTools::getHigherContrastForegroundColor(foregroundHover, backgroundHover, 2.3, foregroundHover);

        if (foregroundPress.isValid() && backgroundPress.isValid())
            ColorTools::getHigherContrastForegroundColor(foregroundPress, backgroundPress, 2.3, foregroundPress);
    }
}

void DecorationButtonPalette::generateButtonOutlinePalette()
{
    outlineNormal = QColor();
    outlineHover = QColor();
    outlinePress = QColor();

    const bool negativeClose(_decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
                             || _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
                             || _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights);

    bool defaultButton =
        true; // flag indicates the button has standard colours for the behaviour and selected colour (i.e. is not a close/max/min with special colours)
    bool *drawOutlineNormally =
        (_buttonType == DecorationButtonType::Close) ? &_buttonBehaviour->drawCloseOutlineNormally : &_buttonBehaviour->drawOutlineNormally;
    bool *drawOutlineOnHover =
        (_buttonType == DecorationButtonType::Close) ? &_buttonBehaviour->drawCloseOutlineOnHover : &_buttonBehaviour->drawOutlineOnHover;
    bool *drawOutlineOnPress =
        (_buttonType == DecorationButtonType::Close) ? &_buttonBehaviour->drawCloseOutlineOnPress : &_buttonBehaviour->drawOutlineOnPress;

    // set normal, hover and press colours
    if (_decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::Accent
        || _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
        || _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
        if (_decorationSettings->translucentButtonBackgrounds()) {
            if (_buttonType == DecorationButtonType::Close && negativeClose) {
                defaultButton = false;
                if (*drawOutlineNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        outlineNormal = _decorationColors->negativeReducedOpacityOutline(); // may want to change these to be distinct colours in the future
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->negativeReducedOpacityOutline();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->negativeReducedOpacityOutline();
                    } else {
                        outlineNormal = _decorationColors->buttonReducedOpacityOutline();
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->negativeReducedOpacityOutline();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->negativeReducedOpacityOutline();
                    }
                } else {
                    if (*drawOutlineOnHover)
                        outlineHover = _decorationColors->negativeReducedOpacityOutline();
                    if (*drawOutlineOnPress)
                        outlinePress = _decorationColors->negativeReducedOpacityOutline();
                }
            } else if (_buttonType == DecorationButtonType::Minimize
                       && _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (*drawOutlineNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        outlineNormal = _decorationColors->neutralReducedOpacityOutline();
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->neutralReducedOpacityOutline();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->neutralReducedOpacityOutline();
                    } else {
                        outlineNormal = _decorationColors->buttonReducedOpacityOutline();
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->neutralReducedOpacityOutline();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->neutralReducedOpacityOutline();
                    }
                } else {
                    if (*drawOutlineOnHover)
                        outlineHover = _decorationColors->neutralReducedOpacityOutline();
                    if (*drawOutlineOnPress)
                        outlinePress = _decorationColors->neutralReducedOpacityOutline();
                }
            } else if (_buttonType == DecorationButtonType::Maximize
                       && _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (*drawOutlineNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        outlineNormal = _decorationColors->positiveReducedOpacityOutline();
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->positiveReducedOpacityOutline();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->positiveReducedOpacityOutline();
                    } else {
                        outlineNormal = _decorationColors->buttonReducedOpacityOutline();
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->positiveReducedOpacityOutline();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->positiveReducedOpacityOutline();
                    }
                } else {
                    if (*drawOutlineOnHover)
                        outlineHover = _decorationColors->positiveReducedOpacityOutline();
                    if (*drawOutlineOnPress)
                        outlinePress = _decorationColors->positiveReducedOpacityOutline();
                }
            }

            if (defaultButton) {
                if (*drawOutlineNormally) {
                    outlineNormal = _decorationColors->buttonReducedOpacityOutline();
                    if (*drawOutlineOnHover)
                        outlineHover = _decorationColors->buttonReducedOpacityOutline();
                    if (*drawOutlineOnPress)
                        outlinePress = _decorationColors->buttonReducedOpacityOutline();
                } else {
                    if (*drawOutlineOnHover)
                        outlineHover = _decorationColors->buttonReducedOpacityOutline();
                    if (*drawOutlineOnPress)
                        outlinePress = _decorationColors->buttonReducedOpacityOutline();
                }
            }
        } else { // non-translucent accent colours
            if (_buttonType == DecorationButtonType::Close && negativeClose) {
                defaultButton = false;
                if (*drawOutlineNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        outlineNormal = _decorationColors->negativeSaturated();
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->negativeSaturated();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->negativeSaturated();
                    } else {
                        outlineNormal = _decorationColors->buttonFocus();
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->negativeSaturated();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->negativeSaturated();
                    }
                } else {
                    if (*drawOutlineOnHover)
                        outlineHover = _decorationColors->negativeSaturated();
                    if (*drawOutlineOnPress)
                        outlinePress = _decorationColors->negativeSaturated();
                }
            } else if (_buttonType == DecorationButtonType::Minimize
                       && _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (*drawOutlineNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        outlineNormal = _decorationColors->neutral();
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->neutral();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->neutral();
                    } else {
                        outlineNormal = _decorationColors->buttonFocus();
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->neutral();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->neutral();
                    }
                } else {
                    if (*drawOutlineOnHover)
                        outlineHover = _decorationColors->neutral();
                    if (*drawOutlineOnPress)
                        outlinePress = _decorationColors->neutral();
                }
            } else if (_buttonType == DecorationButtonType::Maximize
                       && _decorationSettings->buttonBackgroundColors() == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (*drawOutlineNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        outlineNormal = _decorationColors->positive();
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->positive();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->positive();
                    } else {
                        outlineNormal = _decorationColors->buttonFocus();
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->positive();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->positive();
                    }
                } else {
                    if (*drawOutlineOnHover)
                        outlineHover = _decorationColors->positive();
                    if (*drawOutlineOnPress)
                        outlinePress = _decorationColors->positive();
                }
            }

            if (defaultButton) {
                if (*drawOutlineNormally) {
                    outlineNormal = _decorationColors->buttonFocus();
                    if (*drawOutlineOnHover)
                        outlineHover = _decorationColors->buttonFocus();
                    if (*drawOutlineOnPress)
                        outlinePress = _decorationColors->buttonFocus();
                } else {
                    if (*drawOutlineOnHover)
                        outlineHover = _decorationColors->buttonFocus();
                    if (*drawOutlineOnPress)
                        outlinePress = _decorationColors->buttonFocus();
                }
            }
        }

    } else { // titlebar text colour, translucent
        if (_decorationSettings->translucentButtonBackgrounds()) {
            if (_buttonType == DecorationButtonType::Close && negativeClose) {
                defaultButton = false;
                if (*drawOutlineNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        outlineNormal = _decorationColors->negativeReducedOpacityOutline();
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->negativeReducedOpacityOutline();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->negativeReducedOpacityOutline();
                    } else {
                        outlineNormal = ColorTools::alphaMix(baseForeground, qMin(_decorationSettings->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->negativeReducedOpacityOutline();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->negativeReducedOpacityOutline();
                    }
                } else {
                    if (*drawOutlineOnHover)
                        outlineHover = _decorationColors->negativeReducedOpacityOutline();
                    if (*drawOutlineOnPress)
                        outlinePress = _decorationColors->negativeReducedOpacityOutline();
                }
            }
            if (defaultButton) {
                if (*drawOutlineNormally) {
                    outlineNormal = ColorTools::alphaMix(baseForeground, qMin(_decorationSettings->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                    if (*drawOutlineOnHover)
                        outlineHover = ColorTools::alphaMix(baseForeground, qMin(_decorationSettings->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                    if (*drawOutlineOnPress)
                        outlinePress = ColorTools::alphaMix(baseForeground, qMin(_decorationSettings->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                } else {
                    if (*drawOutlineOnHover)
                        outlineHover = ColorTools::alphaMix(baseForeground, qMin(_decorationSettings->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                    if (*drawOutlineOnPress)
                        outlinePress = ColorTools::alphaMix(baseForeground, qMin(_decorationSettings->translucentButtonBackgroundsOpacity() * 0.5, 1.0));
                }
            }
        } else { // titlebar text colour, non-translucent
            if (_buttonType == DecorationButtonType::Close && negativeClose) {
                defaultButton = false;
                if (*drawOutlineNormally) {
                    if (!_decorationSettings->negativeCloseBackgroundHoverPress()) {
                        outlineNormal = _decorationColors->negativeSaturated();
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->negativeSaturated();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->negativeSaturated();
                    } else {
                        outlineNormal = KColorUtils::mix(baseBackground, baseForeground, 0.3);
                        if (*drawOutlineOnHover)
                            outlineHover = _decorationColors->negativeSaturated();
                        if (*drawOutlineOnPress)
                            outlinePress = _decorationColors->negativeSaturated();
                    }
                } else {
                    if (*drawOutlineOnHover)
                        outlineHover = _decorationColors->negativeSaturated();
                    if (*drawOutlineOnPress)
                        outlinePress = _decorationColors->negativeSaturated();
                }
            }
            if (defaultButton) {
                if (*drawOutlineNormally) {
                    outlineNormal = KColorUtils::mix(baseBackground, baseForeground, 0.3);
                    if (*drawOutlineOnHover)
                        outlineHover = KColorUtils::mix(baseBackground, baseForeground, 0.3);
                    if (*drawOutlineOnPress)
                        outlinePress = KColorUtils::mix(baseBackground, baseForeground, 0.3);
                } else {
                    if (*drawOutlineOnHover)
                        outlineHover = KColorUtils::mix(baseBackground, baseForeground, 0.3);
                    if (*drawOutlineOnHover)
                        outlinePress = KColorUtils::mix(baseBackground, baseForeground, 0.3);
                }
            }
        }
    }

    if (_buttonOverrideColorsPresent) {
        if (_buttonOverrideColorsActive.value(OverridableButtonColorStates::OutlineNormal).isValid() && *drawOutlineNormally) {
            outlineNormal = _buttonOverrideColorsActive.value(OverridableButtonColorStates::OutlineNormal);
        }
        if (_buttonOverrideColorsActive.value(OverridableButtonColorStates::OutlineHover).isValid() && *drawOutlineOnHover) {
            outlineHover = _buttonOverrideColorsActive.value(OverridableButtonColorStates::OutlineHover);
        }
        if (_buttonOverrideColorsActive.value(OverridableButtonColorStates::OutlinePress).isValid() && *drawOutlineOnPress) {
            outlinePress = _buttonOverrideColorsActive.value(OverridableButtonColorStates::OutlinePress);
        }
    }

    // low contrast correction between outline and titlebar
    if (_decorationSettings->adjustBackgroundColorOnPoorContrast()) {
        if (outlineNormal.isValid() && KColorUtils::contrastRatio(outlineNormal, baseBackground) < 1.3) {
            outlineNormal = KColorUtils::mix(outlineNormal, baseForeground, 0.4);
        }

        if (outlineHover.isValid() && KColorUtils::contrastRatio(outlineHover, baseBackground) < 1.3) {
            outlineHover = KColorUtils::mix(outlineHover, baseForeground, 0.4);
        }
        if (outlinePress.isValid() && KColorUtils::contrastRatio(outlinePress, baseBackground) < 1.3) {
            outlinePress = KColorUtils::mix(outlinePress, baseForeground, 0.4);
        }
    }
}

}
