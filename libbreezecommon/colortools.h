#ifndef BREEZE_COLORTOOLS_H
#define BREEZE_COLORTOOLS_H

/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breezecommon_export.h"

#include <KColorScheme>
#include <QColor>
#include <memory>

namespace Breeze
{

struct BREEZECOMMON_EXPORT DecorationColors {
    QColor buttonFocus = QColor();
    QColor buttonHover = QColor();
    QColor buttonReducedOpacityBackground = QColor();
    QColor buttonReducedOpacityOutline = QColor();
    QColor highlight = QColor();
    QColor highlightLessSaturated = QColor();
    QColor negative = QColor();
    QColor negativeLessSaturated = QColor();
    QColor negativeSaturated = QColor();
    QColor negativeReducedOpacityBackground = QColor();
    QColor negativeReducedOpacityOutline = QColor();
    QColor negativeReducedOpacityLessSaturatedBackground = QColor();
    QColor neutral = QColor();
    QColor neutralLessSaturated = QColor();
    QColor neutralReducedOpacityBackground = QColor();
    QColor neutralReducedOpacityOutline = QColor();
    QColor positive = QColor();
    QColor positiveLessSaturated = QColor();
    QColor positiveReducedOpacityBackground = QColor();
    QColor positiveReducedOpacityOutline = QColor();
};

extern std::shared_ptr<DecorationColors> BREEZECOMMON_EXPORT g_decorationColors;

/**
 * @brief Functions to manipulate colours within Klassy
 *        To be used as common code base across both kdecoration and kstyle.
 */
class BREEZECOMMON_EXPORT ColorTools
{
public:
    /**
     * @brief Returns a DecorationColors struct containing the colours set in the KDE color scheme
     */
    static std::shared_ptr<DecorationColors> generateDecorationColors(const QPalette &palette, const bool setGlobal = false);

    static void systemPaletteUpdated(const QPalette &palette);

    static QColor getDifferentiatedSaturatedColor(const QColor &inputColor, bool noMandatoryDifferentiate = false);

    static QColor getDifferentiatedLessSaturatedColor(const QColor &inputColor, bool noMandatoryDifferentiate = false);

    static QColor getLessSaturatedColorForWindowHighlight(const QColor &inputColor, bool noMandatoryDifferentiate = false);

    /**
     * @brief Checks the contrast ratio of the two given colours, and if is below the given threshold returns a higher contrast black or white foreground
     * @param foregroundColor The foreground colour to potentially replace
     * @param backgroundColor The background colour to compare with
     * @param blackWhiteContrastThreshold The contrast threshold, below which a black or white foreground colour will be returned
     * @return the higher contrast QColor
     */
    static QColor getHigherContrastForegroundColor(const QColor &foregroundColor, const QColor &backgroundColor, double blackWhiteContrastThreshold);

    /**
     * @brief Given a background colour, will return either a black or white foregreound colour, depending upon which gives the best contrast
     */
    static QColor getBlackOrWhiteForegroundForHighContrast(const QColor &backgroundColor);

    /**
     * @brief Mulitplies the existing aplhaF value of the inputColor by aplhaMixFactor
     * @param inputColor The input QColor
     * @param alphaMixFactor The factor to multiply inputColor by
     * @return the output QColor with modified alpha
     */
    static QColor alphaMix(const QColor &inputColor, const qreal &alphaMixFactor);
};
}

#endif
