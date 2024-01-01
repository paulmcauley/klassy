#ifndef BREEZE_COLORTOOLS_H
#define BREEZE_COLORTOOLS_H

/*
 * SPDX-FileCopyrightText: 2021-2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breeze.h"
#include "breezecommon_export.h"

#include <KColorScheme>
#include <QColor>
#include <memory>

namespace Breeze
{

struct BREEZECOMMON_EXPORT DecorationColorPalette {
    QColor buttonFocus;
    QColor buttonHover;
    QColor buttonReducedOpacityBackground;
    QColor buttonReducedOpacityOutline;
    QColor highlight;
    QColor highlightLessSaturated;
    QColor negative;
    QColor negativeLessSaturated;
    QColor negativeSaturated;
    QColor negativeReducedOpacityBackground;
    QColor negativeReducedOpacityOutline;
    QColor negativeReducedOpacityLessSaturatedBackground;
    QColor fullySaturatedNegative;
    QColor neutral;
    QColor neutralLessSaturated;
    QColor neutralSaturated;
    QColor neutralReducedOpacityBackground;
    QColor neutralReducedOpacityOutline;
    QColor positive;
    QColor positiveLessSaturated;
    QColor positiveSaturated;
    QColor positiveReducedOpacityBackground;
    QColor positiveReducedOpacityOutline;
};

extern std::unique_ptr<DecorationColorPalette> BREEZECOMMON_EXPORT g_decorationPalette;
extern qreal BREEZECOMMON_EXPORT g_translucentButtonBackgroundsOpacity;

class BREEZECOMMON_EXPORT DecorationColors : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Class for generating and accessing the decoration colour palette modified from the system colour scheme colours
     *        Includes a caching mechanism
     *        Global decoration colours should only be accessed via this class and not directly via g_decorationPalette for caching to work properly
     *
     * @param systemBasepalette The sytem palette to use as a base for generating the decoration palette
     * @param decorationSettings An InternalSettings pointer neededed to get the translucent button opacity values
     * @param useCachedGlobalPalette If an object is created with this flag true it will use the caching mechanism, else it will not cache and generate its own
     * copy of the decoration palette
     * @param regenerate Used when useCachedGlobalPalette is true. If set true the colours are regenerated in the constructor, else an existing cached value is
     * used if available
     */
    DecorationColors(const QPalette &systemBasepalette,
                     const QSharedPointer<InternalSettings> decorationSettings,
                     const bool useCachedGlobalPalette,
                     const bool regenerate = true);
    virtual ~DecorationColors() = default;

    //* color return methods return either the global or local colour, depending on whether useCachedGlobalPalette was set in the constructor
    QColor &buttonFocus()
    {
        return (*m_decorationPalette)->buttonFocus;
    }
    QColor &buttonHover()
    {
        return (*m_decorationPalette)->buttonHover;
    }
    QColor &buttonReducedOpacityBackground()
    {
        return (*m_decorationPalette)->buttonReducedOpacityBackground;
    }
    QColor &buttonReducedOpacityOutline()
    {
        return (*m_decorationPalette)->buttonReducedOpacityOutline;
    }
    QColor &highlight()
    {
        return (*m_decorationPalette)->highlight;
    }
    QColor &highlightLessSaturated()
    {
        return (*m_decorationPalette)->highlightLessSaturated;
    }
    QColor &negative()
    {
        return (*m_decorationPalette)->negative;
    }
    QColor &negativeLessSaturated()
    {
        return (*m_decorationPalette)->negativeLessSaturated;
    }
    QColor &negativeSaturated()
    {
        return (*m_decorationPalette)->negativeSaturated;
    }
    QColor &negativeReducedOpacityBackground()
    {
        return (*m_decorationPalette)->negativeReducedOpacityBackground;
    }
    QColor &negativeReducedOpacityOutline()
    {
        return (*m_decorationPalette)->negativeReducedOpacityOutline;
    }
    QColor &negativeReducedOpacityLessSaturatedBackground()
    {
        return (*m_decorationPalette)->negativeReducedOpacityLessSaturatedBackground;
    }
    QColor &fullySaturatedNegative()
    {
        return (*m_decorationPalette)->fullySaturatedNegative;
    }
    QColor &neutral()
    {
        return (*m_decorationPalette)->neutral;
    }
    QColor &neutralLessSaturated()
    {
        return (*m_decorationPalette)->neutralLessSaturated;
    }
    QColor &neutralSaturated()
    {
        return (*m_decorationPalette)->neutralSaturated;
    }
    QColor &neutralReducedOpacityBackground()
    {
        return (*m_decorationPalette)->neutralReducedOpacityBackground;
    }
    QColor &neutralReducedOpacityOutline()
    {
        return (*m_decorationPalette)->neutralReducedOpacityOutline;
    }
    QColor &positive()
    {
        return (*m_decorationPalette)->positive;
    }
    QColor &positiveLessSaturated()
    {
        return (*m_decorationPalette)->positiveLessSaturated;
    }
    QColor &positiveSaturated()
    {
        return (*m_decorationPalette)->positiveSaturated;
    }
    QColor &positiveReducedOpacityBackground()
    {
        return (*m_decorationPalette)->positiveReducedOpacityBackground;
    }
    QColor &positiveReducedOpacityOutline()
    {
        return (*m_decorationPalette)->positiveReducedOpacityOutline;
    }

public Q_SLOTS:

    /**
     * @brief Regenerates the decorationColors
     */
    void generateDecorationColors(const QPalette &palette, const QSharedPointer<InternalSettings> decorationSettings);

private:
    const bool m_useCachedGlobalPalette;
    std::unique_ptr<DecorationColorPalette> *m_decorationPalette; // pointer to whether to return the global palette data or class member data
    // nonGlobalPalette only used when m_useCachedGlobalPalette is false
    std::unique_ptr<DecorationColorPalette> m_nonGlobalDecorationPalette;
};

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
