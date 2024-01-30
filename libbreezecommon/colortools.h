#ifndef BREEZE_COLORTOOLS_H
#define BREEZE_COLORTOOLS_H

/*
 * SPDX-FileCopyrightText: 2021-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breezecommon_export.h"

#include <QColor>
#include <QImage>

namespace Breeze
{

/**
 * @brief Functions to manipulate colours within Klassy
 *        To be used as common code base across both kdecoration and kstyle.
 */
class BREEZECOMMON_EXPORT ColorTools
{
public:
    static QColor getDifferentiatedSaturatedColor(const QColor &inputColor, bool noMandatoryDifferentiate = false);

    static QColor getDifferentiatedLessSaturatedColor(const QColor &inputColor, bool noMandatoryDifferentiate = false);

    static QColor getLessSaturatedColorForWindowHighlight(const QColor &inputColor, bool noMandatoryDifferentiate = false);

    /**
     * @brief Checks the contrast ratio of the two given colours, and if is below the given threshold returns a higher contrast black or white foreground
     * @param foregroundColor The foreground colour to potentially replace
     * @param backgroundColor The background colour to compare with
     * @param blackWhiteContrastThreshold The contrast threshold, below which a black or white foreground colour will be returned
     * @param outputColor The potentially-adjusted output colour
     * @return returns true if a higher contrast colour was generated, false if the colour remained the same
     */
    static bool getHigherContrastForegroundColor(const QColor &foregroundColor,
                                                 const QColor &backgroundColor,
                                                 const qreal blackWhiteContrastThreshold,
                                                 QColor &outputColor);

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

    static void convertAlphaToColor(QImage &image, const QColor tintColor);

    static void convertAlphaToColor(QIcon &icon, QSize iconSize, const QColor tintColor);
};
}

#endif
