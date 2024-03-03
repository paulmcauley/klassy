/*
 * SPDX-FileCopyrightText: 2023-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "breeze.h"
#include "breezecommon_export.h"
#include "decorationbuttoncolors.h"
#include <KColorScheme>
#include <QColor>
#include <QObject>
#include <QPalette>
#include <map>
#include <memory>

namespace Breeze
{

class DecorationButtonPalette;

struct BREEZECOMMON_EXPORT DecorationPaletteGroup {
    QColor titleBarBase;
    QColor titleBarText;
    QColor windowOutline;
    QColor shadow;
    QColor buttonFocus;
    QColor buttonHover;
    QColor highlight;
    QColor highlightLessSaturated;
    QColor negative;
    QColor negativeLessSaturated;
    QColor negativeSaturated;
    QColor fullySaturatedNegative;
    QColor neutral;
    QColor neutralLessSaturated;
    QColor neutralSaturated;
    QColor positive;
    QColor positiveLessSaturated;
    QColor positiveSaturated;
};

extern qreal BREEZECOMMON_EXPORT g_translucentButtonBackgroundsOpacityActive;
extern qreal BREEZECOMMON_EXPORT g_translucentButtonBackgroundsOpacityInactive;

class BREEZECOMMON_EXPORT DecorationColors
{
public:
    /**
     * @brief Class for generating and accessing the decoration colour palette modified from the system colour scheme colours
     *        Includes a caching mechanism
     *        Global decoration colours should only be accessed via this class for caching to work properly
     *
     * @param useCachedGlobalPalette If an object is created with this flag true it will use the caching mechanism, else it will not cache and generate its own
     * copy of the decoration palette used if available
     * @param forAppStyle If true, only generates separately cached colours for the application style
     */
    DecorationColors(const bool useCachedPalette, const bool forAppStyle = false);

    //* color return methods return either the static or local colour, depending on whether useCachedPalette was set in the constructor
    DecorationPaletteGroup *active() const
    {
        return (m_decorationPaletteGroupActive->get());
    }

    DecorationPaletteGroup *inactive() const
    {
        return (m_decorationPaletteGroupInactive->get());
    }

    DecorationButtonPalette *buttonPalette(DecorationButtonType type) const
    {
        return (&m_buttonPalettes->at(type));
    }

    bool isCachedPalette()
    {
        return m_useCachedPalette;
    }

    bool forAppStyle()
    {
        return m_forAppStyle;
    }

    bool areColorsGenerated()
    {
        return *m_colorsGenerated;
    }

    QPalette *basePalette()
    {
        return m_basePalette;
    }

    QByteArray settingsUpdateUuid()
    {
        if (m_useCachedPalette) {
            return *static_cast<QByteArray *>(m_settingsUpdateUuid);
        } else {
            return QByteArray("");
        }
    }

    /**
     * @brief Regenerates the decorationColors, including button colours
     * @param decorationSettings an InternalSettings pointer
     * @param textActive Active titlebar/window text colour
     * @param baseActive Active titlebar/window background colour
     * @param textInactive Inactive titlebar/window text colour
     * @param baseInactive Inactive titlebar/window background colour
     * @param settingsUpdateUuid UUID of a decoration colour cache update request -- to be used to ensure a cached update request only gets procesed once
     * @param generateOneGroupOnly Only generate either active or inactive colours
     * @param oneGroupActiveState Whether to generate active (true) or inactive (false) colours if \p generateOneGroupOnly is set
     *
     */
    void generateDecorationAndButtonColors(const QPalette &palette,
                                           const QSharedPointer<InternalSettings> decorationSettings,
                                           QColor titleBarTextActive,
                                           QColor titleBarBaseActive,
                                           QColor titleBarTextInactive,
                                           QColor titleBarBaseInactive,
                                           QByteArray settingsUpdateUuid = "",
                                           const bool generateOneGroupOnly = false,
                                           const bool oneGroupActiveState = false

    );

    /**
     * @brief Regenerates the decorationColors, excluding button colours
     * @param decorationSettings an InternalSettings pointer
     * @param textActive Active titlebar/window text colour
     * @param baseActive Active titlebar/window background colour
     * @param textInactive Inactive titlebar/window text colour
     * @param baseInactive Inactive titlebar/window background colour
     * @param settingsUpdateUuid UUID of a decoration colour cache update request -- to be used to ensure a cached update request only gets procesed once
     * @param generateOneGroupOnly Only generate either active or inactive colours
     * @param oneGroupActiveState Whether to generate active (true) or inactive (false) colours if \p generateOneGroupOnly is set
     */
    void generateDecorationColors(const QPalette &palette,
                                  const QSharedPointer<InternalSettings> decorationSettings,
                                  QColor titleBarTextActive,
                                  QColor titleBarBaseActive,
                                  QColor titleBarTextInactive,
                                  QColor titleBarBaseInactive,
                                  QByteArray settingsUpdateUuid = "",
                                  const bool generateOneGroupOnly = false,
                                  const bool oneGroupActiveState = false);

    static void readSystemTitleBarColors(KSharedConfig::Ptr kdeGlobalConfig,
                                         QColor &systemBaseActive,
                                         QColor &systemBaseInactive,
                                         QColor &systemTextActive,
                                         QColor &systemTextInactive,
                                         QString colorSchemePath = QString());

private:
    void generateDecorationPaletteGroup(const QPalette &palette,
                                        const QSharedPointer<InternalSettings> decorationSettings,
                                        const bool active,
                                        QColor &titleBarTextActive,
                                        QColor &titleBarBaseActive,
                                        QColor &titleBarTextInactive,
                                        QColor &titleBarBaseInactive);
    QColor accentedWindowOutlineColor(DecorationPaletteGroup *decorationPaletteGroup,
                                      const QSharedPointer<InternalSettings> decorationSettings,
                                      bool active,
                                      QColor customColor = QColor()) const;
    QColor fontMixedAccentWindowOutlineColor(DecorationPaletteGroup *decorationPaletteGroup,
                                             const QSharedPointer<InternalSettings> decorationSettings,
                                             bool active,
                                             QColor customColor = QColor()) const;

    bool m_useCachedPalette;
    bool m_forAppStyle;

    //* pointers to whether to return the static cached palette data or non-cached class member data
    QPalette *m_basePalette;
    std::unique_ptr<DecorationPaletteGroup> *m_decorationPaletteGroupActive;
    std::unique_ptr<DecorationPaletteGroup> *m_decorationPaletteGroupInactive;
    std::map<DecorationButtonType, DecorationButtonPalette> *m_buttonPalettes;
    bool *m_colorsGenerated;
    void *m_settingsUpdateUuid;

    //* non-cached data only used when m_useCachedPalette is false
    QPalette m_nonCachedClientPalette;
    std::unique_ptr<DecorationPaletteGroup> m_nonCachedDecorationPaletteGroupActive;
    std::unique_ptr<DecorationPaletteGroup> m_nonCachedDecorationPaletteGroupInactive;
    std::map<DecorationButtonType, DecorationButtonPalette> m_nonCachedButtonPalettes;
    bool m_nonCachedColorsGenerated = false;

    //* cached data used for window decorations
    static QPalette s_cachedKdeGlobalPalette;
    static std::unique_ptr<DecorationPaletteGroup> s_cachedDecorationPaletteGroupActive;
    static std::unique_ptr<DecorationPaletteGroup> s_cachedDecorationPaletteGroupInactive;
    static std::map<DecorationButtonType, DecorationButtonPalette> s_cachedButtonPalettes;
    static QByteArray s_settingsUpdateUuid;
    static bool s_cachedColorsGenerated;
};
}
