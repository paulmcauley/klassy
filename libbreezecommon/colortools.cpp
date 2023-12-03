/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "colortools.h"

#include <KColorUtils>
#include <QTimer>

namespace Breeze
{

std::shared_ptr<DecorationColors> g_decorationColors = nullptr;
static std::unique_ptr<QTimer> g_systemPaletteSingleUpdateTimer = std::unique_ptr<QTimer>(new QTimer());

std::shared_ptr<DecorationColors> ColorTools::generateDecorationColors(const QPalette &palette, const bool setGlobal)
{
    std::shared_ptr<DecorationColors> decorationColors = std::make_shared<DecorationColors>();

    KStatefulBrush buttonFocusStatefulBrush;
    KStatefulBrush buttonHoverStatefulBrush;

    buttonFocusStatefulBrush = KStatefulBrush(KColorScheme::Button, KColorScheme::NegativeText);
    decorationColors->negative = buttonFocusStatefulBrush.brush(palette).color();
    // this was too pale
    // buttonHoverStatefulBrush = KStatefulBrush( KColorScheme::Button, KColorScheme::NegativeBackground );
    // colors.negativeBackground = buttonHoverStatefulBrush.brush( palette ).color();
    decorationColors->negativeLessSaturated = getDifferentiatedLessSaturatedColor(decorationColors->negative);
    decorationColors->negativeSaturated = getDifferentiatedSaturatedColor(decorationColors->negative);

    buttonFocusStatefulBrush = KStatefulBrush(KColorScheme::Button, KColorScheme::NeutralText);
    decorationColors->neutral = buttonFocusStatefulBrush.brush(palette).color();
    // this was too pale
    // buttonHoverStatefulBrush = KStatefulBrush( KColorScheme::Button, KColorScheme::NeutralBackground );
    // colors.neutralLessSaturated = buttonHoverStatefulBrush.brush( palette ).color();
    decorationColors->neutralLessSaturated = getDifferentiatedLessSaturatedColor(decorationColors->neutral);
    decorationColors->neutralSaturated = getDifferentiatedSaturatedColor(decorationColors->neutral);

    buttonFocusStatefulBrush = KStatefulBrush(KColorScheme::Button, KColorScheme::PositiveText);
    decorationColors->positive = buttonFocusStatefulBrush.brush(palette).color();
    // this was too pale
    // buttonHoverStatefulBrush = KStatefulBrush( KColorScheme::Button, KColorScheme::PositiveBackground );
    // colors.positiveLessSaturated = buttonHoverStatefulBrush.brush( palette ).color();
    decorationColors->positiveLessSaturated = getDifferentiatedLessSaturatedColor(decorationColors->positive);
    decorationColors->positiveSaturated = getDifferentiatedSaturatedColor(decorationColors->positive);

    buttonFocusStatefulBrush = KStatefulBrush(KColorScheme::Button, KColorScheme::FocusColor);
    decorationColors->buttonFocus = buttonFocusStatefulBrush.brush(palette).color();
    buttonHoverStatefulBrush = KStatefulBrush(KColorScheme::Button, KColorScheme::HoverColor);
    decorationColors->buttonHover = buttonHoverStatefulBrush.brush(palette).color();

    // this is required as the accent colours feature sets these the same
    if (decorationColors->buttonFocus == decorationColors->buttonHover)
        decorationColors->buttonHover = getDifferentiatedLessSaturatedColor(decorationColors->buttonFocus);

    //"Blue Ocean" style reduced opacity outlined buttons
    decorationColors->buttonReducedOpacityBackground = decorationColors->buttonFocus;
    decorationColors->buttonReducedOpacityBackground.setAlphaF(decorationColors->buttonReducedOpacityBackground.alphaF() * 0.4);

    decorationColors->buttonReducedOpacityOutline = decorationColors->buttonFocus;
    decorationColors->buttonReducedOpacityOutline.setAlphaF(decorationColors->buttonReducedOpacityOutline.alphaF() * 0.6);

    decorationColors->fullySaturatedNegative = getDifferentiatedSaturatedColor(decorationColors->negative, true);
    decorationColors->negativeReducedOpacityBackground = decorationColors->fullySaturatedNegative;
    decorationColors->negativeReducedOpacityBackground.setAlphaF(decorationColors->negativeReducedOpacityBackground.alphaF() * 0.5);

    decorationColors->negativeReducedOpacityOutline = decorationColors->fullySaturatedNegative;
    decorationColors->negativeReducedOpacityOutline.setAlphaF(decorationColors->negativeReducedOpacityOutline.alphaF() * 0.7);

    decorationColors->negativeReducedOpacityLessSaturatedBackground = getDifferentiatedLessSaturatedColor(decorationColors->negative);
    decorationColors->negativeReducedOpacityLessSaturatedBackground.setAlphaF(decorationColors->negativeReducedOpacityLessSaturatedBackground.alphaF() * 0.6);

    decorationColors->neutralReducedOpacityBackground = decorationColors->neutral;
    decorationColors->neutralReducedOpacityBackground.setAlphaF(decorationColors->neutralReducedOpacityBackground.alphaF() * 0.4);

    decorationColors->neutralReducedOpacityOutline = decorationColors->neutral;
    decorationColors->neutralReducedOpacityOutline.setAlphaF(decorationColors->neutralReducedOpacityOutline.alphaF() * 0.6);

    decorationColors->positiveReducedOpacityBackground = decorationColors->positive;
    decorationColors->positiveReducedOpacityBackground.setAlphaF(decorationColors->positiveReducedOpacityBackground.alphaF() * 0.4);

    decorationColors->positiveReducedOpacityOutline = decorationColors->positive;
    decorationColors->positiveReducedOpacityOutline.setAlphaF(decorationColors->positiveReducedOpacityOutline.alphaF() * 0.6);

    decorationColors->highlight = palette.color(QPalette::Highlight);
    decorationColors->highlightLessSaturated = getLessSaturatedColorForWindowHighlight(decorationColors->highlight, true);

    if (setGlobal)
        g_decorationColors = decorationColors;

    return decorationColors;
}

void ColorTools::systemPaletteUpdated(const QPalette &palette)
{
    // the timer ensures that generateDecorationColors(palette) will only be called once if more than one updated signal arrives at the same time
    if (!g_systemPaletteSingleUpdateTimer->isActive()) {
        g_systemPaletteSingleUpdateTimer->setSingleShot(true);
        g_systemPaletteSingleUpdateTimer->start(10);
        generateDecorationColors(palette, true);
    }
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

QColor ColorTools::getHigherContrastForegroundColor(const QColor &foregroundColor, const QColor &backgroundColor, double blackWhiteContrastThreshold)
{
    double contrastRatio = KColorUtils::contrastRatio(foregroundColor, backgroundColor);

    if (contrastRatio < blackWhiteContrastThreshold)
        return getBlackOrWhiteForegroundForHighContrast(backgroundColor);
    else
        return foregroundColor;
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
