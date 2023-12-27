/*
 * SPDX-FileCopyrightText: 2022-2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "colortools.h"

#include <KColorUtils>

namespace Breeze
{

// cached values needed for base window decoration colour generation
std::unique_ptr<DecorationColorPalette> g_decorationPalette = nullptr;
qreal g_translucentButtonBackgroundsOpacity = -1.0; // is -1 so gets updated on first iteration

DecorationColors::DecorationColors(const QPalette &systemBasepalette,
                                   const QSharedPointer<InternalSettings> decorationSettings,
                                   const bool useCachedGlobalPalette,
                                   const bool regenerate)
    : m_useCachedGlobalPalette(useCachedGlobalPalette)
{
    if (!useCachedGlobalPalette || (useCachedGlobalPalette && (!g_decorationPalette || regenerate))) {
        generateDecorationColors(systemBasepalette, decorationSettings);
    }
    m_decorationPalette = useCachedGlobalPalette ? &g_decorationPalette : &m_nonGlobalDecorationPalette;
}

void DecorationColors::generateDecorationColors(const QPalette &palette, const QSharedPointer<InternalSettings> decorationSettings)
{
    DecorationColorPalette *decorationColors = new DecorationColorPalette();

    KStatefulBrush buttonFocusStatefulBrush;
    KStatefulBrush buttonHoverStatefulBrush;

    buttonFocusStatefulBrush = KStatefulBrush(KColorScheme::Button, KColorScheme::NegativeText);
    decorationColors->negative = buttonFocusStatefulBrush.brush(palette).color();
    // this was too pale
    // buttonHoverStatefulBrush = KStatefulBrush( KColorScheme::Button, KColorScheme::NegativeBackground );
    // colors.negativeBackground = buttonHoverStatefulBrush.brush( palette ).color();
    decorationColors->negativeLessSaturated = ColorTools::getDifferentiatedLessSaturatedColor(decorationColors->negative);
    decorationColors->negativeSaturated = ColorTools::getDifferentiatedSaturatedColor(decorationColors->negative);

    buttonFocusStatefulBrush = KStatefulBrush(KColorScheme::Button, KColorScheme::NeutralText);
    decorationColors->neutral = buttonFocusStatefulBrush.brush(palette).color();
    // this was too pale
    // buttonHoverStatefulBrush = KStatefulBrush( KColorScheme::Button, KColorScheme::NeutralBackground );
    // colors.neutralLessSaturated = buttonHoverStatefulBrush.brush( palette ).color();
    decorationColors->neutralLessSaturated = ColorTools::getDifferentiatedLessSaturatedColor(decorationColors->neutral);
    decorationColors->neutralSaturated = ColorTools::getDifferentiatedSaturatedColor(decorationColors->neutral);

    buttonFocusStatefulBrush = KStatefulBrush(KColorScheme::Button, KColorScheme::PositiveText);
    decorationColors->positive = buttonFocusStatefulBrush.brush(palette).color();
    // this was too pale
    // buttonHoverStatefulBrush = KStatefulBrush( KColorScheme::Button, KColorScheme::PositiveBackground );
    // colors.positiveLessSaturated = buttonHoverStatefulBrush.brush( palette ).color();
    decorationColors->positiveLessSaturated = ColorTools::getDifferentiatedLessSaturatedColor(decorationColors->positive);
    decorationColors->positiveSaturated = ColorTools::getDifferentiatedSaturatedColor(decorationColors->positive);

    buttonFocusStatefulBrush = KStatefulBrush(KColorScheme::Button, KColorScheme::FocusColor);
    decorationColors->buttonFocus = buttonFocusStatefulBrush.brush(palette).color();
    buttonHoverStatefulBrush = KStatefulBrush(KColorScheme::Button, KColorScheme::HoverColor);
    decorationColors->buttonHover = buttonHoverStatefulBrush.brush(palette).color();

    // this is required as the accent colours feature sets these the same
    if (decorationColors->buttonFocus == decorationColors->buttonHover)
        decorationColors->buttonHover = ColorTools::getDifferentiatedLessSaturatedColor(decorationColors->buttonFocus);

    //"Blue Ocean" style reduced opacity outlined buttons
    decorationColors->buttonReducedOpacityBackground =
        ColorTools::alphaMix(decorationColors->buttonFocus, qMin((decorationSettings->translucentButtonBackgroundsOpacity() * 0.8), 1.0));

    decorationColors->buttonReducedOpacityOutline =
        ColorTools::alphaMix(decorationColors->buttonFocus, qMin((decorationSettings->translucentButtonBackgroundsOpacity() * 1.2), 1.0));

    decorationColors->fullySaturatedNegative = ColorTools::getDifferentiatedSaturatedColor(decorationColors->negative, true);
    decorationColors->negativeReducedOpacityBackground =
        ColorTools::alphaMix(decorationColors->fullySaturatedNegative, decorationSettings->translucentButtonBackgroundsOpacity());

    decorationColors->negativeReducedOpacityOutline =
        ColorTools::alphaMix(decorationColors->fullySaturatedNegative, qMin((decorationSettings->translucentButtonBackgroundsOpacity() * 1.4), 1.0));

    decorationColors->negativeReducedOpacityLessSaturatedBackground =
        ColorTools::alphaMix(ColorTools::getDifferentiatedLessSaturatedColor(decorationColors->negative),
                             qMin((decorationSettings->translucentButtonBackgroundsOpacity() * 1.2), 1.0));

    decorationColors->neutralReducedOpacityBackground =
        ColorTools::alphaMix(decorationColors->neutral, qMin((decorationSettings->translucentButtonBackgroundsOpacity() * 0.8), 1.0));

    decorationColors->neutralReducedOpacityOutline =
        ColorTools::alphaMix(decorationColors->neutral, qMin((decorationSettings->translucentButtonBackgroundsOpacity() * 1.2), 1.0));

    decorationColors->positiveReducedOpacityBackground =
        ColorTools::alphaMix(decorationColors->positive, qMin((decorationSettings->translucentButtonBackgroundsOpacity() * 0.8), 1.0));

    decorationColors->positiveReducedOpacityOutline =
        ColorTools::alphaMix(decorationColors->positive, qMin((decorationSettings->translucentButtonBackgroundsOpacity() * 1.2), 1.0));

    decorationColors->highlight = palette.color(QPalette::Highlight);
    decorationColors->highlightLessSaturated = ColorTools::getLessSaturatedColorForWindowHighlight(decorationColors->highlight, true);

    if (m_useCachedGlobalPalette)
        g_decorationPalette = std::unique_ptr<DecorationColorPalette>(decorationColors);
    else
        m_nonGlobalDecorationPalette = std::unique_ptr<DecorationColorPalette>(decorationColors);
}

QColor ColorTools::getDifferentiatedSaturatedColor(const QColor &inputColor, bool noMandatoryDifferentiate)
{
    int colorHsv[3];
    inputColor.getHsv(&colorHsv[0], &colorHsv[1], &colorHsv[2]);
    if (colorHsv[1] < 240)
        colorHsv[1] = 255; // increase saturation to max if not max
    else if (!noMandatoryDifferentiate)
        colorHsv[1] -= 80; // else reduce saturation if already high to provide differentiation/contrast
    QColor redColorSaturated;
    redColorSaturated.setHsv(colorHsv[0], colorHsv[1], colorHsv[2]);
    return redColorSaturated;
}

QColor ColorTools::getDifferentiatedLessSaturatedColor(const QColor &inputColor, bool noMandatoryDifferentiate)
{
    int colorHsv[3];
    inputColor.getHsv(&colorHsv[0], &colorHsv[1], &colorHsv[2]);

    if (colorHsv[1] >= 100)
        colorHsv[1] -= 80; // decrease saturation if not already low
    else if (!noMandatoryDifferentiate)
        colorHsv[1] += 80; // else increase saturation if very low to provide differentiation/contrast
    QColor outputColor;
    outputColor.setHsv(colorHsv[0], colorHsv[1], colorHsv[2]);
    return outputColor;
}

QColor ColorTools::getLessSaturatedColorForWindowHighlight(const QColor &inputColor, bool noMandatoryDifferentiate)
{
    int colorHsv[3];
    inputColor.getHsv(&colorHsv[0], &colorHsv[1], &colorHsv[2]);

    if (colorHsv[1] >= 100)
        colorHsv[1] -= 30; // decrease saturation if not already low
    else if (!noMandatoryDifferentiate)
        colorHsv[1] += 30; // else increase saturation if very low to provide differentiation/contrast
    QColor outputColor;
    outputColor.setHsv(colorHsv[0], colorHsv[1], colorHsv[2]);
    return outputColor;
}

bool ColorTools::getHigherContrastForegroundColor(const QColor &foregroundColor,
                                                  const QColor &backgroundColor,
                                                  const qreal blackWhiteContrastThreshold,
                                                  QColor &outputColor)
{
    qreal contrastRatio = KColorUtils::contrastRatio(foregroundColor, backgroundColor);

    if (contrastRatio < blackWhiteContrastThreshold) {
        outputColor = getBlackOrWhiteForegroundForHighContrast(backgroundColor);
        return true;
    } else {
        outputColor = foregroundColor;
        return false;
    }
}

QColor ColorTools::getBlackOrWhiteForegroundForHighContrast(const QColor &backgroundColor)
{
    // based on http://www.w3.org/TR/AERT#color-contrast

    if (!backgroundColor.isValid())
        return QColor();

    int rgbBackground[3];

    backgroundColor.getRgb(&rgbBackground[0], &rgbBackground[1], &rgbBackground[2]);

    double brightness = qRound(static_cast<double>(((rgbBackground[0] * 299) + (rgbBackground[1] * 587) + (rgbBackground[2] * 114)) / 1000));

    return (brightness > 125) ? QColor(Qt::GlobalColor::black) : QColor(Qt::GlobalColor::white);
}

QColor ColorTools::alphaMix(const QColor &inputColor, const qreal &alphaMixFactor)
{
    QColor outputColor(inputColor);
    outputColor.setAlphaF(outputColor.alphaF() * alphaMixFactor);
    return outputColor;
}
}
