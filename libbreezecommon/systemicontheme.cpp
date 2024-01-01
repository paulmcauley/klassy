/*
 * SPDX-FileCopyrightText: 2022-2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "systemicontheme.h"
#include "colortools.h"
#include <QIcon>

namespace Breeze
{

using KDecoration2::DecorationButtonType;

void SystemIconTheme::paintIconFromSystemTheme(QString iconName)
{
    // QIcon::setThemeName(QIcon::themeName()); //doing this hack allows Adwaita icon theme to be partially loaded
    QIcon icon(QIcon::fromTheme(iconName));
    QSize pixmapSize(m_iconWidth, m_iconWidth);
    QRect rect(QPoint(0, 0), pixmapSize); // Why not rectF, QSizeF??? experiment

    if (m_internalSettings->colorizeSystemIcons()) {
        // convert the alpha of the icon into tinted colour on transparent
        // TODO: use pixmap(const QSize &size, qreal devicePixelRatio, QIcon::Mode mode = Normal, QIcon::State state = Off) const
        //      this allows setting devicePixelRatioF in Qt6, fixing display of icons on multimonitor setups
        QImage iconImage(icon.pixmap(pixmapSize).toImage());
        ColorTools::convertAlphaToColor(iconImage, m_painter->pen().color());

        m_painter->drawImage(rect, iconImage);
    } else
        icon.paint(m_painter, rect);
}

void SystemIconTheme::renderIcon()
{
    paintIconFromSystemTheme(m_systemIconFromTheme);
}

void SystemIconTheme::systemIconNames(KDecoration2::DecorationButtonType type, QString &systemIconName, QString &systemIconCheckedName)
{
    switch (type) {
    case DecorationButtonType::Close:
        systemIconName = isSystemIconNameAvailable(QStringLiteral("window-close-symbolic"), QStringLiteral("window-close"));
        systemIconCheckedName = systemIconName;
        break;

    case DecorationButtonType::Maximize:
        systemIconCheckedName = isSystemIconNameAvailable(QStringLiteral("window-restore-symbolic"), QStringLiteral("window-restore"));
        systemIconName = isSystemIconNameAvailable(QStringLiteral("window-maximize-symbolic"), QStringLiteral("window-maximize"));
        break;

    case DecorationButtonType::Minimize:
        systemIconName = isSystemIconNameAvailable(QStringLiteral("window-minimize-symbolic"), QStringLiteral("window-minimize"));
        systemIconCheckedName = systemIconName;
        break;

    case DecorationButtonType::OnAllDesktops:
        systemIconCheckedName = isSystemIconNameAvailable(QStringLiteral("window-unpin-symbolic"), QStringLiteral("window-unpin"));
        systemIconName = isSystemIconNameAvailable(QStringLiteral("window-pin-symbolic"), QStringLiteral("window-pin"));
        break;

    case DecorationButtonType::Shade:
        systemIconCheckedName = isSystemIconNameAvailable(QStringLiteral("window-unshade-symbolic"), QStringLiteral("window-unshade"));
        systemIconName = isSystemIconNameAvailable(QStringLiteral("window-shade-symbolic"), QStringLiteral("window-shade"));
        break;

    case DecorationButtonType::KeepBelow:
        systemIconName = isSystemIconNameAvailable(QStringLiteral("window-keep-below-symbolic"), QStringLiteral("window-keep-below"));
        systemIconCheckedName = systemIconName;
        break;

    case DecorationButtonType::KeepAbove:
        systemIconName = isSystemIconNameAvailable(QStringLiteral("window-keep-above-symbolic"), QStringLiteral("window-keep-above"));
        systemIconCheckedName = systemIconName;
        break;

    case DecorationButtonType::ApplicationMenu:
        systemIconName = isSystemIconNameAvailable(QStringLiteral("application-menu-symbolic"), QStringLiteral("application-menu"));
        systemIconCheckedName = systemIconName;
        break;

    case DecorationButtonType::ContextHelp:
        systemIconName = isSystemIconNameAvailable(QStringLiteral("help-contextual-symbolic"), QStringLiteral("help-contextual"));
        systemIconCheckedName = systemIconName;
        break;

    default:
        break;
    }
}

QString SystemIconTheme::isSystemIconNameAvailable(const QString &preferredIconName, const QString &backupIconName)
{
    if (QIcon::hasThemeIcon(preferredIconName))
        return preferredIconName;
    else if (QIcon::hasThemeIcon(backupIconName))
        return backupIconName;
    else
        return QString();
}

}
