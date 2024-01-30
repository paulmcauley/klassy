/*
 * SPDX-FileCopyrightText: 2023-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "decorationbuttoncolors.h"
#include "colortools.h"
#include <KColorUtils>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace Breeze
{

using KDecoration2::DecorationButtonType;

DecorationButtonPalette::DecorationButtonPalette(KDecoration2::DecorationButtonType buttonType)
    : _buttonType(buttonType)
    , _active(std::make_shared<DecorationButtonPaletteGroup>())
    , _inactive(std::make_shared<DecorationButtonPaletteGroup>())

{
}

void DecorationButtonPalette::generate(InternalSettingsPtr decorationSettings,
                                       DecorationColors *decorationColors,
                                       const bool generateOneGroupOnly,
                                       const bool oneGroupActiveState)
{
    _decorationSettings = decorationSettings;
    _decorationColors = decorationColors;

    if (!(generateOneGroupOnly && !oneGroupActiveState)) { // active
        decodeButtonOverrideColors(true);
        generateButtonBackgroundPalette(true);
        generateButtonForegroundPalette(true);
        generateButtonOutlinePalette(true);
    }

    if (!(generateOneGroupOnly && oneGroupActiveState)) { // inactive
        decodeButtonOverrideColors(false);
        generateButtonBackgroundPalette(false);
        generateButtonForegroundPalette(false);
        generateButtonOutlinePalette(false);
    }
}

void DecorationButtonPalette::decodeButtonOverrideColors(const bool active)
{
    QMap<OverridableButtonColorState, QColor> &buttonOverrideColors = active ? _buttonOverrideColorsActive : _buttonOverrideColorsInactive;
    bool &buttonOverrideColorsPresent = active ? _buttonOverrideColorsPresentActive : _buttonOverrideColorsPresentInactive;

    buttonOverrideColors.clear();
    buttonOverrideColorsPresent = false;

    bool overrideButtonTypeValid = false;
    for (int i = 0; i < InternalSettings::EnumButtonOverrideColorsActiveButtonType::COUNT; i++) {
        if (static_cast<int>(_buttonType) == i) {
            overrideButtonTypeValid = true;
            break;
        }
    }
    if (!overrideButtonTypeValid) {
        return;
    }

    QByteArray overrideColorsSetting = active ? _decorationSettings->buttonOverrideColorsActive(static_cast<int>(_buttonType)).toUtf8()
                                              : _decorationSettings->buttonOverrideColorsInactive(static_cast<int>(_buttonType)).toUtf8();
    if (overrideColorsSetting.isEmpty()) {
        return;
    }

    QJsonDocument document = QJsonDocument::fromJson(overrideColorsSetting);

    QJsonObject buttonStatesObject = document.object();
    bool overrideColorLoaded = false;

    for (auto i = buttonStatesObject.begin(); i < buttonStatesObject.end(); i++) {
        QJsonArray colorArray = i->toArray();
        QColor color;
        double colorOpacity;
        int overridableButtonColorStatesIndex;
        int overrideColorItemsIndex;
        switch (colorArray.count()) {
        case 0:
        default:
            break;
        case 1:
            overridableButtonColorStatesIndex = overridableButtonColorStatesJsonStrings.indexOf(i.key());
            if (overridableButtonColorStatesIndex < 0)
                break;

            overrideColorItemsIndex = overrideColorItems.indexOf(colorArray[0].toString());
            if (overrideColorItemsIndex == 0)
                break;

            color = overrideColorItemsIndexToColor(_decorationColors, overrideColorItemsIndex, active);
            if (!color.isValid())
                break;

            buttonOverrideColors.insert(static_cast<OverridableButtonColorState>(overridableButtonColorStatesIndex), color);
            overrideColorLoaded = true;
            break;
        case 2:
            overridableButtonColorStatesIndex = overridableButtonColorStatesJsonStrings.indexOf(i.key());
            if (overridableButtonColorStatesIndex < 0)
                break;

            overrideColorItemsIndex = overrideColorItems.indexOf(colorArray[0].toString());
            if (overrideColorItemsIndex == 0)
                break;

            color = overrideColorItemsIndexToColor(_decorationColors, overrideColorItemsIndex, active);
            if (!color.isValid())
                break;

            colorOpacity = colorArray[1].toDouble(-1.0);
            if (colorOpacity >= 0 && colorOpacity <= 1.0) {
                color.setAlphaF(colorOpacity);
            } else {
                break;
            }

            buttonOverrideColors.insert(static_cast<OverridableButtonColorState>(overridableButtonColorStatesIndex), color);
            overrideColorLoaded = true;
            break;
        case 3:
            overridableButtonColorStatesIndex = overridableButtonColorStatesJsonStrings.indexOf(i.key());
            if (overridableButtonColorStatesIndex < 0)
                break;

            color.setRed(colorArray[0].toInt());
            color.setGreen(colorArray[1].toInt());
            color.setBlue(colorArray[2].toInt());
            if (!color.isValid())
                break;

            buttonOverrideColors.insert(static_cast<OverridableButtonColorState>(overridableButtonColorStatesIndex), color);
            overrideColorLoaded = true;
            break;
        case 4:
            overridableButtonColorStatesIndex = overridableButtonColorStatesJsonStrings.indexOf(i.key());
            if (overridableButtonColorStatesIndex < 0)
                break;

            color.setRed(colorArray[1].toInt());
            color.setGreen(colorArray[2].toInt());
            color.setBlue(colorArray[3].toInt());
            if (!color.isValid())
                break;

            colorOpacity = colorArray[0].toDouble(-1.0);
            if (colorOpacity >= 0 && colorOpacity <= 1.0) {
                color.setAlphaF(colorOpacity);
            } else {
                break;
            }

            buttonOverrideColors.insert(static_cast<OverridableButtonColorState>(overridableButtonColorStatesIndex), color);
            overrideColorLoaded = true;
            break;
        }
    }

    buttonOverrideColorsPresent = overrideColorLoaded;
}

QColor DecorationButtonPalette::overrideColorItemsIndexToColor(const DecorationColors *decorationColors, const int overrideColorItemsIndex, const bool active)
{
    switch (overrideColorItemsIndex) {
    default:
        return QColor();
    case 0:
        return Qt::transparent;
    case 1:
        return active ? decorationColors->active()->titleBarText : decorationColors->inactive()->titleBarText;
    case 2:
        return decorationColors->active()->titleBarText;
    case 3:
        return decorationColors->inactive()->titleBarText;
    case 4:
        return active ? decorationColors->active()->titleBarBase : decorationColors->inactive()->titleBarBase;
    case 5:
        return decorationColors->active()->titleBarBase;
    case 6:
        return decorationColors->inactive()->titleBarBase;
    case 7:
        return decorationColors->active()->buttonHover;
    case 8:
        return decorationColors->active()->buttonFocus;
    case 9:
        return decorationColors->active()->highlight;
    case 10:
        return decorationColors->active()->highlightLessSaturated;
    case 11:
        return decorationColors->active()->negative;
    case 12:
        return decorationColors->active()->negativeLessSaturated;
    case 13:
        return decorationColors->active()->negativeSaturated;
    case 14:
        return decorationColors->active()->fullySaturatedNegative;
    case 15:
        return decorationColors->active()->neutral;
    case 16:
        return decorationColors->active()->neutralLessSaturated;
    case 17:
        return decorationColors->active()->neutralSaturated;
    case 18:
        return decorationColors->active()->positive;
    case 19:
        return decorationColors->active()->positiveLessSaturated;
    case 20:
        return decorationColors->active()->positiveSaturated;
    case 21:
        return Qt::white;
    case 22:
        return active ? decorationColors->active()->windowOutline : decorationColors->inactive()->windowOutline;
    case 23:
        return decorationColors->active()->windowOutline;
    case 24:
        return decorationColors->inactive()->windowOutline;
    case 25:
        return active ? decorationColors->active()->shadow : decorationColors->inactive()->shadow;
    case 26:
        return decorationColors->active()->shadow;
    case 27:
        return decorationColors->inactive()->shadow;
    }
}

void DecorationButtonPalette::generateButtonBackgroundPalette(const bool active)
{
    DecorationButtonPaletteGroup *group = active ? this->_active.get() : this->_inactive.get();
    QColor &backgroundNormal = group->backgroundNormal;
    QColor &backgroundHover = group->backgroundHover;
    QColor &backgroundPress = group->backgroundPress;
    DecorationPaletteGroup *decorationColors = active ? _decorationColors->active() : _decorationColors->inactive();
    QColor &text = decorationColors->titleBarText;
    QColor &base = decorationColors->titleBarBase;

    bool &negativeNormalCloseBackground = group->negativeNormalCloseBackground;
    bool &negativeHoverCloseBackground = group->negativeHoverCloseBackground;
    bool &negativePressCloseBackground = group->negativePressCloseBackground;

    const int buttonBackgroundColors = _decorationSettings->buttonBackgroundColors(active);
    const bool translucentBackgrounds = _decorationSettings->translucentButtonBackgrounds(active);
    const qreal translucentButtonBackgroundsOpacity = _decorationSettings->translucentButtonBackgroundsOpacity(active);
    const bool negativeCloseBackgroundHoverPress = _decorationSettings->negativeCloseBackgroundHoverPress(active);
    const bool adjustBackgroundColorOnPoorContrast = _decorationSettings->adjustBackgroundColorOnPoorContrast(active);

    backgroundNormal = QColor();
    backgroundHover = QColor();
    backgroundPress = QColor();

    const bool negativeCloseCategory( // whether the button background colour is in the general negative category as selected in the "Background & Outline
                                      // Colours" combobox
        buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
        || buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
        || buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights);

    negativeNormalCloseBackground = false;
    negativeHoverCloseBackground = false;
    negativePressCloseBackground = false;

    bool defaultButton =
        true; // flag indicates the button has standard colours for the behaviour and selected colour (i.e. is not a close/max/min with special colours)
    const bool &drawBackgroundNormally = (_buttonType == DecorationButtonType::Close) ? _decorationSettings->showCloseBackgroundNormally(active)
                                                                                      : _decorationSettings->showBackgroundNormally(active);
    const bool &drawBackgroundOnHover = (_buttonType == DecorationButtonType::Close) ? _decorationSettings->showCloseBackgroundOnHover(active)
                                                                                     : _decorationSettings->showBackgroundOnHover(active);
    const bool &drawBackgroundOnPress = (_buttonType == DecorationButtonType::Close) ? _decorationSettings->showCloseBackgroundOnPress(active)
                                                                                     : _decorationSettings->showBackgroundOnPress(active);

    // set normal, hover and press colours
    if (buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::Accent
        || buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
        || buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
        if (translucentBackgrounds) {
            if (_buttonType == DecorationButtonType::Close && negativeCloseCategory) {
                defaultButton = false;
                if (drawBackgroundNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        negativeNormalCloseBackground = true;
                        backgroundNormal = decorationColors->negativeReducedOpacityBackground;
                        if (drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = decorationColors->negativeReducedOpacityOutline;
                        }
                        if (drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = decorationColors->fullySaturatedNegative;
                        }
                    } else {
                        backgroundNormal = decorationColors->buttonReducedOpacityBackground;
                        if (drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = decorationColors->negativeReducedOpacityOutline;
                        }
                        if (drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = decorationColors->fullySaturatedNegative;
                        }
                    }
                } else {
                    if (drawBackgroundOnHover) {
                        negativeHoverCloseBackground = true;
                        backgroundHover = decorationColors->negativeReducedOpacityBackground;
                    }
                    if (drawBackgroundOnPress) {
                        negativePressCloseBackground = true;
                        backgroundPress = decorationColors->negativeReducedOpacityOutline;
                    }
                }
            } else if (_buttonType == DecorationButtonType::Minimize
                       && buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (drawBackgroundNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        backgroundNormal = decorationColors->neutralReducedOpacityBackground;
                        if (drawBackgroundOnHover)
                            backgroundHover = decorationColors->neutralReducedOpacityOutline;
                        if (drawBackgroundOnPress)
                            backgroundPress = decorationColors->neutral;
                    } else {
                        backgroundNormal = decorationColors->buttonReducedOpacityBackground;
                        if (drawBackgroundOnHover)
                            backgroundHover = decorationColors->neutralReducedOpacityOutline;
                        if (drawBackgroundOnPress)
                            backgroundPress = decorationColors->neutral;
                    }
                } else {
                    if (drawBackgroundOnHover)
                        backgroundHover = decorationColors->neutralReducedOpacityBackground;
                    if (drawBackgroundOnPress)
                        backgroundPress = decorationColors->neutralReducedOpacityOutline;
                }
            } else if (_buttonType == DecorationButtonType::Maximize
                       && buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (drawBackgroundNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        backgroundNormal = decorationColors->positiveReducedOpacityBackground;
                        if (drawBackgroundOnHover)
                            backgroundHover = decorationColors->positiveReducedOpacityOutline;
                        if (drawBackgroundOnPress)
                            backgroundPress = decorationColors->positive;
                    } else {
                        backgroundNormal = decorationColors->buttonReducedOpacityBackground;
                        if (drawBackgroundOnHover)
                            backgroundHover = decorationColors->positiveReducedOpacityOutline;
                        if (drawBackgroundOnPress)
                            backgroundPress = decorationColors->positive;
                    }
                } else {
                    if (drawBackgroundOnHover)
                        backgroundHover = decorationColors->positiveReducedOpacityBackground;
                    if (drawBackgroundOnPress)
                        backgroundPress = decorationColors->positiveReducedOpacityOutline;
                }
            }

            if (defaultButton) {
                if (drawBackgroundNormally) {
                    backgroundNormal = decorationColors->buttonReducedOpacityBackground;
                    if (drawBackgroundOnHover)
                        backgroundHover = decorationColors->buttonReducedOpacityOutline;
                    if (drawBackgroundOnPress)
                        backgroundPress = decorationColors->buttonFocus;
                } else {
                    if (drawBackgroundOnHover)
                        backgroundHover = decorationColors->buttonReducedOpacityBackground;
                    if (drawBackgroundOnPress)
                        backgroundPress = decorationColors->buttonReducedOpacityOutline;
                }
            }
        } else { // accent but not translucent
            if (_buttonType == DecorationButtonType::Close && negativeCloseCategory) {
                defaultButton = false;
                if (drawBackgroundNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        negativeNormalCloseBackground = true;
                        backgroundNormal = decorationColors->negative;
                        if (drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = decorationColors->negativeSaturated;
                        }
                        if (drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = decorationColors->negativeLessSaturated;
                        }
                    } else {
                        backgroundNormal = drawBackgroundNormally ? KColorUtils::mix(base, decorationColors->buttonHover, 0.8) : decorationColors->buttonHover;
                        if (drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = decorationColors->negativeSaturated;
                        }
                        if (drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = decorationColors->negativeLessSaturated;
                        }
                    }
                } else {
                    if (drawBackgroundOnHover) {
                        negativeHoverCloseBackground = true;
                        backgroundHover = decorationColors->negative;
                    }
                    if (drawBackgroundOnPress) {
                        negativePressCloseBackground = true;
                        backgroundPress = decorationColors->negativeSaturated;
                    }
                }
            } else if (_buttonType == DecorationButtonType::Minimize
                       && buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (drawBackgroundNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        backgroundNormal = decorationColors->neutralLessSaturated;
                        if (drawBackgroundOnHover)
                            backgroundHover = decorationColors->neutral;
                        if (drawBackgroundOnPress)
                            backgroundPress = decorationColors->neutralSaturated;
                    } else {
                        backgroundNormal = KColorUtils::mix(base, decorationColors->buttonHover, 0.8);
                        if (drawBackgroundOnHover)
                            backgroundHover = decorationColors->neutral;
                        if (drawBackgroundOnPress)
                            backgroundPress = decorationColors->neutralSaturated;
                    }
                } else {
                    if (drawBackgroundOnHover)
                        backgroundHover = decorationColors->neutralLessSaturated;
                    if (drawBackgroundOnPress)
                        backgroundPress = decorationColors->neutral;
                }
            } else if (_buttonType == DecorationButtonType::Maximize
                       && buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (drawBackgroundNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        backgroundNormal = decorationColors->positiveLessSaturated;
                        if (drawBackgroundOnHover)
                            backgroundHover = decorationColors->positive;
                        if (drawBackgroundOnPress)
                            backgroundPress = decorationColors->positiveSaturated;
                    } else {
                        backgroundNormal = KColorUtils::mix(base, decorationColors->buttonHover, 0.8);
                        if (drawBackgroundOnHover)
                            backgroundHover = decorationColors->positive;
                        if (drawBackgroundOnPress)
                            backgroundPress = decorationColors->positiveSaturated;
                    }
                } else {
                    if (drawBackgroundOnHover)
                        backgroundHover = decorationColors->positiveLessSaturated;
                    if (drawBackgroundOnPress)
                        backgroundPress = decorationColors->positive;
                }
            }

            if (defaultButton) {
                if (drawBackgroundNormally) {
                    backgroundNormal = KColorUtils::mix(base, decorationColors->buttonHover, 0.8);
                    if (drawBackgroundOnHover)
                        backgroundHover = decorationColors->buttonHover;
                    if (drawBackgroundOnPress)
                        backgroundPress = decorationColors->buttonFocus;
                } else {
                    if (drawBackgroundOnHover)
                        backgroundHover = decorationColors->buttonHover;
                    if (drawBackgroundOnPress)
                        backgroundPress = decorationColors->buttonFocus;
                }
            }
        }

    } else {
        if (translucentBackgrounds) { // titlebar text color, translucent
            if (_buttonType == DecorationButtonType::Close && negativeCloseCategory) {
                defaultButton = false;
                if (drawBackgroundNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        negativeNormalCloseBackground = true;
                        backgroundNormal = decorationColors->negativeReducedOpacityBackground;
                        if (drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = decorationColors->negativeReducedOpacityOutline;
                        }
                        if (drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = decorationColors->negativeReducedOpacityLessSaturatedBackground;
                        }
                    } else {
                        backgroundNormal = ColorTools::alphaMix(text, qMin(translucentButtonBackgroundsOpacity * 0.3, 1.0));
                        if (drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = decorationColors->negativeReducedOpacityOutline;
                        }
                        if (drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = decorationColors->negativeReducedOpacityBackground;
                        }
                    }
                } else {
                    if (drawBackgroundOnHover) {
                        negativeHoverCloseBackground = true;
                        backgroundHover = decorationColors->negativeReducedOpacityBackground;
                    }
                    if (drawBackgroundOnPress) {
                        negativePressCloseBackground = true;
                        backgroundPress = decorationColors->negativeReducedOpacityOutline;
                    }
                }
            }

            if (defaultButton) {
                if (drawBackgroundNormally) {
                    backgroundNormal = ColorTools::alphaMix(text, qMin(translucentButtonBackgroundsOpacity * 0.3, 1.0));
                    if (drawBackgroundOnHover)
                        backgroundHover = ColorTools::alphaMix(text, qMin(translucentButtonBackgroundsOpacity * 0.5, 1.0));
                    if (drawBackgroundOnPress)
                        backgroundPress = ColorTools::alphaMix(text, qMin(translucentButtonBackgroundsOpacity * 0.7, 1.0));
                } else {
                    if (drawBackgroundOnHover)
                        backgroundHover = ColorTools::alphaMix(text, qMin(translucentButtonBackgroundsOpacity * 0.3, 1.0));
                    if (drawBackgroundOnPress)
                        backgroundPress = ColorTools::alphaMix(text, qMin(translucentButtonBackgroundsOpacity * 0.5, 1.0));
                }
            }
        } else { // titlebar text color, not translucent
            if (_buttonType == DecorationButtonType::Close && negativeCloseCategory) {
                defaultButton = false;
                if (_decorationSettings->showBackgroundNormally(active)) {
                    if (!negativeCloseBackgroundHoverPress) {
                        if (drawBackgroundNormally) {
                            negativeNormalCloseBackground = true;
                            backgroundNormal = decorationColors->negative;
                        }
                        if (drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = decorationColors->negativeSaturated;
                        }
                        if (drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = decorationColors->negativeLessSaturated;
                        }
                    } else {
                        backgroundNormal = KColorUtils::mix(base, text, 0.3);
                        if (drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = decorationColors->negativeSaturated;
                        }
                        if (drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = decorationColors->negativeLessSaturated;
                        }
                    }
                } else if (_decorationSettings->showCloseBackgroundNormally(active)) {
                    if (!negativeCloseBackgroundHoverPress) {
                        if (drawBackgroundNormally) {
                            negativeNormalCloseBackground = true;
                            backgroundNormal = decorationColors->negative;
                        }
                        if (drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = decorationColors->negativeSaturated;
                        }
                        if (drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = decorationColors->negativeLessSaturated;
                        }
                    } else {
                        if (drawBackgroundNormally)
                            backgroundNormal = text;
                        if (drawBackgroundOnHover) {
                            negativeHoverCloseBackground = true;
                            backgroundHover = decorationColors->negativeSaturated;
                        }
                        if (drawBackgroundOnPress) {
                            negativePressCloseBackground = true;
                            backgroundPress = decorationColors->negativeLessSaturated;
                        }
                    }
                } else {
                    if (drawBackgroundOnHover) {
                        negativeHoverCloseBackground = true;
                        backgroundHover = decorationColors->negative;
                    }
                    if (drawBackgroundOnPress) {
                        negativePressCloseBackground = true;
                        backgroundPress = decorationColors->negativeSaturated;
                    }
                }
            }

            if (defaultButton) {
                if (_decorationSettings->showBackgroundNormally(active)) {
                    if (drawBackgroundNormally)
                        backgroundNormal = KColorUtils::mix(base, text, 0.3);
                    if (drawBackgroundOnHover)
                        backgroundHover = KColorUtils::mix(base, text, 0.6);
                    if (drawBackgroundOnPress)
                        backgroundPress = text;
                } else if (_decorationSettings->showCloseBackgroundNormally(active) && _buttonType == DecorationButtonType::Close) {
                    if (drawBackgroundNormally)
                        backgroundNormal = text;
                    if (drawBackgroundOnHover)
                        backgroundHover = KColorUtils::mix(base, text, 0.6);
                    if (drawBackgroundOnPress)
                        backgroundPress = KColorUtils::mix(base, text, 0.3);
                } else {
                    if (drawBackgroundOnHover)
                        backgroundHover = text;
                    if (drawBackgroundOnPress)
                        backgroundPress = KColorUtils::mix(base, text, 0.3);
                }
            }
        }
    }

    const bool buttonOverrideColorsPresent = active ? _buttonOverrideColorsPresentActive : _buttonOverrideColorsPresentInactive;
    if (buttonOverrideColorsPresent) {
        auto &buttonOverrideColors = active ? _buttonOverrideColorsActive : _buttonOverrideColorsInactive;

        if (buttonOverrideColors.value(OverridableButtonColorState::BackgroundNormal).isValid() && drawBackgroundNormally) {
            backgroundNormal = buttonOverrideColors.value(OverridableButtonColorState::BackgroundNormal);
        }
        if (buttonOverrideColors.value(OverridableButtonColorState::BackgroundHover).isValid() && drawBackgroundOnHover) {
            backgroundHover = buttonOverrideColors.value(OverridableButtonColorState::BackgroundHover);
        }
        if (buttonOverrideColors.value(OverridableButtonColorState::BackgroundPress).isValid() && drawBackgroundOnPress) {
            backgroundPress = buttonOverrideColors.value(OverridableButtonColorState::BackgroundPress);
        }
    }

    // low contrast correction between background and titlebar
    if (adjustBackgroundColorOnPoorContrast) {
        if (backgroundNormal.isValid() && KColorUtils::contrastRatio(backgroundNormal, base) < 1.3) {
            backgroundNormal = KColorUtils::mix(backgroundNormal, text, 0.3);
        }

        if (backgroundHover.isValid() && KColorUtils::contrastRatio(backgroundHover, base) < 1.3) {
            backgroundHover = KColorUtils::mix(backgroundHover, text, 0.3);
        }
        if (backgroundPress.isValid() && KColorUtils::contrastRatio(backgroundPress, base) < 1.3) {
            backgroundPress = KColorUtils::mix(backgroundPress, text, 0.3);
        }
    }
}

void DecorationButtonPalette::generateButtonForegroundPalette(const bool active)
{
    DecorationButtonPaletteGroup *group = active ? this->_active.get() : this->_inactive.get();
    QColor &foregroundNormal = group->foregroundNormal;
    QColor &foregroundHover = group->foregroundHover;
    QColor &foregroundPress = group->foregroundPress;
    DecorationPaletteGroup *decorationColors = active ? _decorationColors->active() : _decorationColors->inactive();
    QColor &text = decorationColors->titleBarText;
    QColor &base = decorationColors->titleBarBase;

    foregroundNormal = QColor();
    foregroundHover = QColor();
    foregroundPress = QColor();

    const int buttonBackgroundColors = _decorationSettings->buttonBackgroundColors(active);
    const bool translucentBackgrounds = _decorationSettings->translucentButtonBackgrounds(active);
    const int buttonIconColors = _decorationSettings->buttonIconColors(active);
    const int closeButtonIconColor = _decorationSettings->closeButtonIconColor(active);
    const bool blackWhiteIconOnPoorContrast = _decorationSettings->blackWhiteIconOnPoorContrast(active);

    bool defaultButton =
        true; // flag indicates the button has standard colours for the behaviour and selected colour (i.e. is not a close/max/min with special colours)
    bool closeForegroundIsDefault = false;

    const bool &drawBackgroundNormally = (_buttonType == DecorationButtonType::Close) ? _decorationSettings->showCloseBackgroundNormally(active)
                                                                                      : _decorationSettings->showBackgroundNormally(active);
    const bool &drawBackgroundOnHover = (_buttonType == DecorationButtonType::Close) ? _decorationSettings->showCloseBackgroundOnHover(active)
                                                                                     : _decorationSettings->showBackgroundOnHover(active);
    const bool &drawBackgroundOnPress = (_buttonType == DecorationButtonType::Close) ? _decorationSettings->showCloseBackgroundOnPress(active)
                                                                                     : _decorationSettings->showBackgroundOnPress(active);
    ;

    const bool &drawIconNormally =
        (_buttonType == DecorationButtonType::Close) ? _decorationSettings->showCloseIconNormally(active) : _decorationSettings->showIconNormally(active);
    const bool &drawIconOnHover =
        (_buttonType == DecorationButtonType::Close) ? _decorationSettings->showCloseIconOnHover(active) : _decorationSettings->showIconOnHover(active);
    const bool &drawIconOnPress =
        (_buttonType == DecorationButtonType::Close) ? _decorationSettings->showCloseIconOnPress(active) : _decorationSettings->showIconOnPress(active);

    const bool negativeCloseBackground = (group->negativeNormalCloseBackground || group->negativeHoverCloseBackground || group->negativePressCloseBackground);

    const bool negativeWhenHoverPress = closeButtonIconColor == InternalSettings::EnumCloseButtonIconColor::NegativeWhenHoverPress
        && (buttonIconColors == InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose
            || buttonIconColors == InternalSettings::EnumButtonIconColors::AccentNegativeClose
            || buttonIconColors == InternalSettings::EnumButtonIconColors::AccentTrafficLights);

    if (closeButtonIconColor != InternalSettings::EnumCloseButtonIconColor::AsSelected
        && ((_buttonType == DecorationButtonType::Close && negativeCloseBackground)
            || (negativeWhenHoverPress
                && (_buttonType == DecorationButtonType::Close
                    || (buttonIconColors == InternalSettings::EnumButtonIconColors::AccentTrafficLights
                        && (_buttonType == DecorationButtonType::Maximize || _buttonType == DecorationButtonType::Minimize)))))) {
        defaultButton = false;

        if (closeButtonIconColor == InternalSettings::EnumCloseButtonIconColor::White) {
            if (drawIconNormally)
                foregroundNormal = Qt::GlobalColor::white;
            if (drawIconOnHover)
                foregroundHover = Qt::GlobalColor::white;
            if (drawIconOnPress)
                foregroundPress = Qt::GlobalColor::white;
        } else {
            if (closeButtonIconColor == InternalSettings::EnumCloseButtonIconColor::WhiteWhenHoverPress) {
                if (drawIconOnHover)
                    foregroundHover = Qt::GlobalColor::white;
                if (drawIconOnPress)
                    foregroundPress = Qt::GlobalColor::white;
            } else if (negativeWhenHoverPress) {
                if (_buttonType == DecorationButtonType::Close) {
                    if (drawIconOnHover)
                        foregroundHover = decorationColors->negativeSaturated;
                    if (drawIconOnPress)
                        foregroundPress = decorationColors->negativeSaturated;
                } else if (_buttonType == DecorationButtonType::Maximize) {
                    if (drawIconOnHover)
                        foregroundHover = decorationColors->positiveSaturated;
                    if (drawIconOnPress)
                        foregroundPress = decorationColors->positiveSaturated;
                } else if (_buttonType == DecorationButtonType::Minimize) {
                    if (drawIconOnHover)
                        foregroundHover = decorationColors->neutral;
                    if (drawIconOnPress)
                        foregroundPress = decorationColors->neutral;
                }
            }

            // get foregroundNormal
            if (buttonIconColors == InternalSettings::EnumButtonIconColors::TitlebarText) {
                if ((buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
                     || buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::TitlebarText)
                    && !translucentBackgrounds) {
                    if (drawBackgroundNormally) {
                        if (drawIconNormally)
                            foregroundNormal = base;
                    } else {
                        if (drawIconNormally)
                            foregroundNormal = text;
                    }
                } else {
                    if (drawIconNormally)
                        foregroundNormal = text;
                }
            } else if (!negativeWhenHoverPress
                       && (buttonIconColors == InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose
                           || buttonIconColors == InternalSettings::EnumButtonIconColors::AccentNegativeClose
                           || buttonIconColors == InternalSettings::EnumButtonIconColors::AccentTrafficLights)) {
                if (drawIconNormally)
                    foregroundNormal = decorationColors->negativeSaturated;
            } else { // Normal icon colour is selected default colour - treat as default button
                closeForegroundIsDefault = true;
            }
        }
    }

    if (!negativeWhenHoverPress) {
        if (_buttonType == DecorationButtonType::Close
            && (buttonIconColors == InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose
                || buttonIconColors == InternalSettings::EnumButtonIconColors::AccentNegativeClose
                || buttonIconColors == InternalSettings::EnumButtonIconColors::AccentTrafficLights)) {
            defaultButton = false;
            if (drawIconNormally && !foregroundNormal.isValid())
                foregroundNormal = decorationColors->negativeSaturated;
            if (drawIconOnHover && !foregroundHover.isValid())
                foregroundHover = decorationColors->negativeSaturated;
            if (drawIconOnPress && !foregroundPress.isValid())
                foregroundPress = decorationColors->negativeSaturated;
        } else if (_buttonType == DecorationButtonType::Maximize && (buttonIconColors == InternalSettings::EnumButtonIconColors::AccentTrafficLights)) {
            defaultButton = false;
            if (drawIconNormally && !foregroundNormal.isValid())
                foregroundNormal = decorationColors->positiveSaturated;
            if (drawIconOnHover && !foregroundHover.isValid())
                foregroundHover = decorationColors->positiveSaturated;
            if (drawIconOnPress && !foregroundPress.isValid())
                foregroundPress = decorationColors->positiveSaturated;
        } else if (_buttonType == DecorationButtonType::Minimize && (buttonIconColors == InternalSettings::EnumButtonIconColors::AccentTrafficLights)) {
            defaultButton = false;
            if (drawIconNormally && !foregroundNormal.isValid())
                foregroundNormal = decorationColors->neutral;
            if (drawIconOnHover && !foregroundHover.isValid())
                foregroundHover = decorationColors->neutral;
            if (drawIconOnPress && !foregroundPress.isValid())
                foregroundPress = decorationColors->neutral;
        }
    }

    if (defaultButton || closeForegroundIsDefault) {
        if ((buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
             || buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::TitlebarText)
            && !translucentBackgrounds
            && (buttonIconColors == InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose
                || buttonIconColors == InternalSettings::EnumButtonIconColors::TitlebarText)) {
            if (drawBackgroundNormally) {
                if (drawIconNormally && !foregroundNormal.isValid())
                    foregroundNormal = base;
                if (drawIconOnHover && !foregroundHover.isValid())
                    foregroundHover = base;
                if (drawIconOnPress && !foregroundPress.isValid())
                    foregroundPress = base;
            } else {
                if (drawIconNormally && !foregroundNormal.isValid())
                    foregroundNormal = text;
                if (drawIconOnHover && !foregroundHover.isValid())
                    foregroundHover = drawBackgroundOnHover ? base : text;
                if (drawIconOnPress && !foregroundPress.isValid())
                    foregroundPress = drawBackgroundOnPress ? base : text;
            }
        } else {
            if (buttonIconColors == InternalSettings::EnumButtonIconColors::TitlebarTextNegativeClose
                || buttonIconColors == InternalSettings::EnumButtonIconColors::TitlebarText) {
                if (drawIconNormally && !foregroundNormal.isValid())
                    foregroundNormal = text;
                if (drawIconOnHover && !foregroundHover.isValid())
                    foregroundHover = text;
                if (drawIconOnPress && !foregroundPress.isValid())
                    foregroundPress = text;
            } else if (buttonIconColors == InternalSettings::EnumButtonIconColors::AccentNegativeClose
                       || buttonIconColors == InternalSettings::EnumButtonIconColors::Accent
                       || buttonIconColors == InternalSettings::EnumButtonIconColors::AccentTrafficLights) {
                if (drawIconNormally && !foregroundNormal.isValid())
                    foregroundNormal = decorationColors->highlight;
                if (drawIconOnHover && !foregroundHover.isValid())
                    foregroundHover = decorationColors->highlight;
                if (drawIconOnPress && !foregroundPress.isValid())
                    foregroundPress = decorationColors->highlight;
            }
        }
    }

    const bool buttonOverrideColorsPresent = active ? _buttonOverrideColorsPresentActive : _buttonOverrideColorsPresentInactive;
    if (buttonOverrideColorsPresent) {
        auto &buttonOverrideColors = active ? _buttonOverrideColorsActive : _buttonOverrideColorsInactive;
        if (buttonOverrideColors.value(OverridableButtonColorState::IconNormal).isValid() && drawIconNormally) {
            foregroundNormal = buttonOverrideColors.value(OverridableButtonColorState::IconNormal);
        }
        if (buttonOverrideColors.value(OverridableButtonColorState::IconHover).isValid() && drawIconOnHover) {
            foregroundHover = buttonOverrideColors.value(OverridableButtonColorState::IconHover);
        }
        if (buttonOverrideColors.value(OverridableButtonColorState::IconPress).isValid() && drawIconOnPress) {
            foregroundPress = buttonOverrideColors.value(OverridableButtonColorState::IconPress);
        }
    }

    if (blackWhiteIconOnPoorContrast) {
        if (foregroundNormal.isValid() && group->backgroundNormal.isValid())
            ColorTools::getHigherContrastForegroundColor(foregroundNormal, group->backgroundNormal, 2.3, foregroundNormal);

        if (foregroundHover.isValid() && group->backgroundHover.isValid())
            ColorTools::getHigherContrastForegroundColor(foregroundHover, group->backgroundHover, 2.3, foregroundHover);

        if (foregroundPress.isValid() && group->backgroundPress.isValid())
            ColorTools::getHigherContrastForegroundColor(foregroundPress, group->backgroundPress, 2.3, foregroundPress);
    }
}

void DecorationButtonPalette::generateButtonOutlinePalette(const bool active)
{
    DecorationButtonPaletteGroup *group = active ? this->_active.get() : this->_inactive.get();
    QColor &outlineNormal = group->outlineNormal;
    QColor &outlineHover = group->outlineHover;
    QColor &outlinePress = group->outlinePress;
    DecorationPaletteGroup *decorationColors = active ? _decorationColors->active() : _decorationColors->inactive();
    QColor &text = decorationColors->titleBarText;
    QColor &base = decorationColors->titleBarBase;

    const int buttonBackgroundColors = _decorationSettings->buttonBackgroundColors(active);
    const bool translucentBackgrounds = _decorationSettings->translucentButtonBackgrounds(active);
    const qreal translucentButtonBackgroundsOpacity = _decorationSettings->translucentButtonBackgroundsOpacity(active);
    const bool negativeCloseBackgroundHoverPress = _decorationSettings->negativeCloseBackgroundHoverPress(active);
    const bool adjustBackgroundColorOnPoorContrast = _decorationSettings->adjustBackgroundColorOnPoorContrast(active);

    outlineNormal = QColor();
    outlineHover = QColor();
    outlinePress = QColor();

    const bool negativeClose(buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::TitlebarTextNegativeClose
                             || buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
                             || buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights);

    bool defaultButton =
        true; // flag indicates the button has standard colours for the behaviour and selected colour (i.e. is not a close/max/min with special colours)
    const bool &drawOutlineNormally =
        (_buttonType == DecorationButtonType::Close) ? _decorationSettings->showCloseOutlineNormally(active) : _decorationSettings->showOutlineNormally(active);
    const bool &drawOutlineOnHover =
        (_buttonType == DecorationButtonType::Close) ? _decorationSettings->showCloseOutlineOnHover(active) : _decorationSettings->showOutlineOnHover(active);
    const bool &drawOutlineOnPress =
        (_buttonType == DecorationButtonType::Close) ? _decorationSettings->showCloseOutlineOnPress(active) : _decorationSettings->showOutlineOnPress(active);

    // set normal, hover and press colours
    if (buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::Accent
        || buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentNegativeClose
        || buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
        if (translucentBackgrounds) {
            if (_buttonType == DecorationButtonType::Close && negativeClose) {
                defaultButton = false;
                if (drawOutlineNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        outlineNormal = decorationColors->negativeReducedOpacityOutline; // may want to change these to be distinct colours in the future
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->negativeReducedOpacityOutline;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->negativeReducedOpacityOutline;
                    } else {
                        outlineNormal = decorationColors->buttonReducedOpacityOutline;
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->negativeReducedOpacityOutline;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->negativeReducedOpacityOutline;
                    }
                } else {
                    if (drawOutlineOnHover)
                        outlineHover = decorationColors->negativeReducedOpacityOutline;
                    if (drawOutlineOnPress)
                        outlinePress = decorationColors->negativeReducedOpacityOutline;
                }
            } else if (_buttonType == DecorationButtonType::Minimize
                       && buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (drawOutlineNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        outlineNormal = decorationColors->neutralReducedOpacityOutline;
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->neutralReducedOpacityOutline;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->neutralReducedOpacityOutline;
                    } else {
                        outlineNormal = decorationColors->buttonReducedOpacityOutline;
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->neutralReducedOpacityOutline;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->neutralReducedOpacityOutline;
                    }
                } else {
                    if (drawOutlineOnHover)
                        outlineHover = decorationColors->neutralReducedOpacityOutline;
                    if (drawOutlineOnPress)
                        outlinePress = decorationColors->neutralReducedOpacityOutline;
                }
            } else if (_buttonType == DecorationButtonType::Maximize
                       && buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (drawOutlineNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        outlineNormal = decorationColors->positiveReducedOpacityOutline;
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->positiveReducedOpacityOutline;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->positiveReducedOpacityOutline;
                    } else {
                        outlineNormal = decorationColors->buttonReducedOpacityOutline;
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->positiveReducedOpacityOutline;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->positiveReducedOpacityOutline;
                    }
                } else {
                    if (drawOutlineOnHover)
                        outlineHover = decorationColors->positiveReducedOpacityOutline;
                    if (drawOutlineOnPress)
                        outlinePress = decorationColors->positiveReducedOpacityOutline;
                }
            }

            if (defaultButton) {
                if (drawOutlineNormally) {
                    outlineNormal = decorationColors->buttonReducedOpacityOutline;
                    if (drawOutlineOnHover)
                        outlineHover = decorationColors->buttonReducedOpacityOutline;
                    if (drawOutlineOnPress)
                        outlinePress = decorationColors->buttonReducedOpacityOutline;
                } else {
                    if (drawOutlineOnHover)
                        outlineHover = decorationColors->buttonReducedOpacityOutline;
                    if (drawOutlineOnPress)
                        outlinePress = decorationColors->buttonReducedOpacityOutline;
                }
            }
        } else { // non-translucent accent colours
            if (_buttonType == DecorationButtonType::Close && negativeClose) {
                defaultButton = false;
                if (drawOutlineNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        outlineNormal = decorationColors->negativeSaturated;
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->negativeSaturated;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->negativeSaturated;
                    } else {
                        outlineNormal = decorationColors->buttonFocus;
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->negativeSaturated;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->negativeSaturated;
                    }
                } else {
                    if (drawOutlineOnHover)
                        outlineHover = decorationColors->negativeSaturated;
                    if (drawOutlineOnPress)
                        outlinePress = decorationColors->negativeSaturated;
                }
            } else if (_buttonType == DecorationButtonType::Minimize
                       && buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (drawOutlineNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        outlineNormal = decorationColors->neutral;
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->neutral;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->neutral;
                    } else {
                        outlineNormal = decorationColors->buttonFocus;
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->neutral;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->neutral;
                    }
                } else {
                    if (drawOutlineOnHover)
                        outlineHover = decorationColors->neutral;
                    if (drawOutlineOnPress)
                        outlinePress = decorationColors->neutral;
                }
            } else if (_buttonType == DecorationButtonType::Maximize
                       && buttonBackgroundColors == InternalSettings::EnumButtonBackgroundColors::AccentTrafficLights) {
                defaultButton = false;
                if (drawOutlineNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        outlineNormal = decorationColors->positive;
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->positive;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->positive;
                    } else {
                        outlineNormal = decorationColors->buttonFocus;
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->positive;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->positive;
                    }
                } else {
                    if (drawOutlineOnHover)
                        outlineHover = decorationColors->positive;
                    if (drawOutlineOnPress)
                        outlinePress = decorationColors->positive;
                }
            }

            if (defaultButton) {
                if (drawOutlineNormally) {
                    outlineNormal = decorationColors->buttonFocus;
                    if (drawOutlineOnHover)
                        outlineHover = decorationColors->buttonFocus;
                    if (drawOutlineOnPress)
                        outlinePress = decorationColors->buttonFocus;
                } else {
                    if (drawOutlineOnHover)
                        outlineHover = decorationColors->buttonFocus;
                    if (drawOutlineOnPress)
                        outlinePress = decorationColors->buttonFocus;
                }
            }
        }

    } else { // titlebar text colour, translucent
        if (translucentBackgrounds) {
            if (_buttonType == DecorationButtonType::Close && negativeClose) {
                defaultButton = false;
                if (drawOutlineNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        outlineNormal = decorationColors->negativeReducedOpacityOutline;
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->negativeReducedOpacityOutline;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->negativeReducedOpacityOutline;
                    } else {
                        outlineNormal = ColorTools::alphaMix(text, qMin(translucentButtonBackgroundsOpacity * 0.5, 1.0));
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->negativeReducedOpacityOutline;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->negativeReducedOpacityOutline;
                    }
                } else {
                    if (drawOutlineOnHover)
                        outlineHover = decorationColors->negativeReducedOpacityOutline;
                    if (drawOutlineOnPress)
                        outlinePress = decorationColors->negativeReducedOpacityOutline;
                }
            }
            if (defaultButton) {
                if (drawOutlineNormally) {
                    outlineNormal = ColorTools::alphaMix(text, qMin(translucentButtonBackgroundsOpacity * 0.5, 1.0));
                    if (drawOutlineOnHover)
                        outlineHover = ColorTools::alphaMix(text, qMin(translucentButtonBackgroundsOpacity * 0.5, 1.0));
                    if (drawOutlineOnPress)
                        outlinePress = ColorTools::alphaMix(text, qMin(translucentButtonBackgroundsOpacity * 0.5, 1.0));
                } else {
                    if (drawOutlineOnHover)
                        outlineHover = ColorTools::alphaMix(text, qMin(translucentButtonBackgroundsOpacity * 0.5, 1.0));
                    if (drawOutlineOnPress)
                        outlinePress = ColorTools::alphaMix(text, qMin(translucentButtonBackgroundsOpacity * 0.5, 1.0));
                }
            }
        } else { // titlebar text colour, non-translucent
            if (_buttonType == DecorationButtonType::Close && negativeClose) {
                defaultButton = false;
                if (drawOutlineNormally) {
                    if (!negativeCloseBackgroundHoverPress) {
                        outlineNormal = decorationColors->negativeSaturated;
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->negativeSaturated;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->negativeSaturated;
                    } else {
                        outlineNormal = KColorUtils::mix(base, text, 0.3);
                        if (drawOutlineOnHover)
                            outlineHover = decorationColors->negativeSaturated;
                        if (drawOutlineOnPress)
                            outlinePress = decorationColors->negativeSaturated;
                    }
                } else {
                    if (drawOutlineOnHover)
                        outlineHover = decorationColors->negativeSaturated;
                    if (drawOutlineOnPress)
                        outlinePress = decorationColors->negativeSaturated;
                }
            }
            if (defaultButton) {
                if (drawOutlineNormally) {
                    outlineNormal = KColorUtils::mix(base, text, 0.3);
                    if (drawOutlineOnHover)
                        outlineHover = KColorUtils::mix(base, text, 0.3);
                    if (drawOutlineOnPress)
                        outlinePress = KColorUtils::mix(base, text, 0.3);
                } else {
                    if (drawOutlineOnHover)
                        outlineHover = KColorUtils::mix(base, text, 0.3);
                    if (drawOutlineOnHover)
                        outlinePress = KColorUtils::mix(base, text, 0.3);
                }
            }
        }
    }

    const bool buttonOverrideColorsPresent = active ? _buttonOverrideColorsPresentActive : _buttonOverrideColorsPresentInactive;
    if (buttonOverrideColorsPresent) {
        auto &buttonOverrideColors = active ? _buttonOverrideColorsActive : _buttonOverrideColorsInactive;
        if (buttonOverrideColors.value(OverridableButtonColorState::OutlineNormal).isValid() && drawOutlineNormally) {
            outlineNormal = buttonOverrideColors.value(OverridableButtonColorState::OutlineNormal);
        }
        if (buttonOverrideColors.value(OverridableButtonColorState::OutlineHover).isValid() && drawOutlineOnHover) {
            outlineHover = buttonOverrideColors.value(OverridableButtonColorState::OutlineHover);
        }
        if (buttonOverrideColors.value(OverridableButtonColorState::OutlinePress).isValid() && drawOutlineOnPress) {
            outlinePress = buttonOverrideColors.value(OverridableButtonColorState::OutlinePress);
        }
    }

    // low contrast correction between outline and titlebar
    if (adjustBackgroundColorOnPoorContrast) {
        if (outlineNormal.isValid() && KColorUtils::contrastRatio(outlineNormal, base) < 1.3) {
            outlineNormal = KColorUtils::mix(outlineNormal, text, 0.4);
        }

        if (outlineHover.isValid() && KColorUtils::contrastRatio(outlineHover, base) < 1.3) {
            outlineHover = KColorUtils::mix(outlineHover, text, 0.4);
        }
        if (outlinePress.isValid() && KColorUtils::contrastRatio(outlinePress, base) < 1.3) {
            outlinePress = KColorUtils::mix(outlinePress, text, 0.4);
        }
    }
}

}
