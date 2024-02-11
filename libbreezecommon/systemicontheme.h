#ifndef BREEZE_SYSTEMICONTHEME_H
#define BREEZE_SYSTEMICONTHEME_H

/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "breezecommon_export.h"
#include "breezesettings.h"
#include <KDecoration2/DecorationButton>
#include <KDecoration2/DecorationSettings>
#include <QPainter>
#include <QPalette>

namespace Breeze
{

class BREEZECOMMON_EXPORT SystemIconTheme
{
public:
    /**
     * @brief
     *
     * @param painter A QPainter object already initialised with an 18x18 reference window and pen.
     * @param iconWidth The width and height of the icon (assumed square) - will be converted to int internally at present
     * @param iconName The name of the icon in the system icon m_systemIconFromTheme
     * @param internalSettings Window Decoration internal settings pointer
     * @param devicePixelRatio devicePixelRatio of paint device - set separately here as the value from the device does not work on X11
     */
    SystemIconTheme(QPainter *painter,
                    const qreal iconWidth,
                    const QString &iconName,
                    const QSharedPointer<InternalSettings> internalSettings,
                    const QPalette &palette)
        : m_painter(painter)
        , m_iconWidth(iconWidth)
        , m_systemIconFromTheme(iconName)
        , m_internalSettings(internalSettings)
        , m_palette(palette){};

    void renderIcon();

    //* When "Use system icon theme" is selected for the icons then not all icons are available as a window-*-symbolic icon
    //* ouputs systemIconName and systemIconCheckedName
    static void systemIconNames(KDecoration2::DecorationButtonType type, QString &systemIconName, QString &systemIconCheckedName);
    static QString isSystemIconNameAvailable(const QString &preferredIconName, const QString &backupIconName);

private:
    void paintIconFromSystemTheme(QString iconName);

    QPainter *m_painter;
    const qreal m_iconWidth;
    QString m_systemIconFromTheme;
    const QSharedPointer<InternalSettings> m_internalSettings;
    QPalette m_palette;
};

}

#endif
