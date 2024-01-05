#ifndef BREEZE_COLORTOOLS_H
#define BREEZE_COLORTOOLS_H

/*
 * SPDX-FileCopyrightText: 2021-2024 Paul A McAuley <kde@paulmcauley.com>
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

struct BREEZECOMMON_EXPORT DecorationPaletteGroup {
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

extern qreal BREEZECOMMON_EXPORT g_translucentButtonBackgroundsOpacityActive;
extern qreal BREEZECOMMON_EXPORT g_translucentButtonBackgroundsOpacityInactive;

class BREEZECOMMON_EXPORT DecorationPalette : public QObject
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
    DecorationPalette(const QPalette &systemBasepalette,
                      const QSharedPointer<InternalSettings> decorationSettings,
                      const bool useCachedPalette,
                      const bool regenerate = true);
    virtual ~DecorationPalette() = default;

    //* color return methods return either the static or local colour, depending on whether useCachedPalette was set in the constructor
    DecorationPaletteGroup *active() const
    {
        return (m_decorationPaletteActive->get());
    }
    DecorationPaletteGroup *inactive() const
    {
        return (m_decorationPaletteInactive->get());
    }

public Q_SLOTS:

    /**
     * @brief Regenerates the decorationColors
     */
    void generateDecorationColors(const QPalette &palette, const QSharedPointer<InternalSettings> decorationSettings);

private:
    void generateDecorationColors(const QPalette &palette, const QSharedPointer<InternalSettings> decorationSettings, const bool active);
    const bool m_useCachedPalette;
    std::unique_ptr<DecorationPaletteGroup> *m_decorationPaletteActive; // pointer to whether to return the global palette data or class member data
    std::unique_ptr<DecorationPaletteGroup> *m_decorationPaletteInactive;

    // non-cached palette group only used when m_useCachedPalette is false
    std::unique_ptr<DecorationPaletteGroup> m_nonCachedDecorationPaletteActive;
    std::unique_ptr<DecorationPaletteGroup> m_nonCachedDecorationPaletteInactive;
    static std::unique_ptr<DecorationPaletteGroup> s_CachedDecorationPaletteActive;
    static std::unique_ptr<DecorationPaletteGroup> s_CachedDecorationPaletteInactive;
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
