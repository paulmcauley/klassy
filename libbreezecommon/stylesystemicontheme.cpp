/*
 * SPDX-FileCopyrightText: 2022-2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylesystemicontheme.h"
#include <QIcon>

namespace Breeze
{

void RenderStyleSystemIconTheme::paintIconFromSystemTheme(QString iconName)
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
        convertAlphaToColorOnTransparent(iconImage, m_pen.color());

        m_painter->drawImage(rect, iconImage);
    } else
        icon.paint(m_painter, rect);
}

void RenderStyleSystemIconTheme::convertAlphaToColorOnTransparent(QImage &image, const QColor &tintColor)
{
    if (image.isNull())
        return;
    image.convertTo(QImage::Format_ARGB32);

    QColor outputColor(tintColor);
    int alpha;

    for (int y = 0; y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            // line[x] is a pixel
            alpha = qAlpha(line[x]);
            if (alpha > 0) {
                outputColor.setAlphaF((qreal(alpha) / 255) * tintColor.alphaF());
                line[x] = outputColor.rgba();
            }
        }
    }
}

void RenderStyleSystemIconTheme::renderCloseIcon()
{
    paintIconFromSystemTheme(QStringLiteral("window-close-symbolic"));
}

void RenderStyleSystemIconTheme::renderMaximizeIcon()
{
    paintIconFromSystemTheme(QStringLiteral("window-maximize-symbolic"));
}

void RenderStyleSystemIconTheme::renderRestoreIcon()
{
    paintIconFromSystemTheme(QStringLiteral("window-restore-symbolic"));
}

void RenderStyleSystemIconTheme::renderMinimizeIcon()
{
    paintIconFromSystemTheme(QStringLiteral("window-minimize-symbolic"));
}

void RenderStyleSystemIconTheme::renderKeepBehindIcon()
{
    paintIconFromSystemTheme(QStringLiteral("window-keep-below-symbolic"));
}

void RenderStyleSystemIconTheme::renderKeepInFrontIcon()
{
    paintIconFromSystemTheme(QStringLiteral("window-keep-above-symbolic"));
}

void RenderStyleSystemIconTheme::renderContextHelpIcon()
{
    renderRounderAndBolderContextHelpIcon();
}

void RenderStyleSystemIconTheme::renderShadeIcon()
{
    RenderDecorationButtonIcon18By18::renderShadeIcon();
}

void RenderStyleSystemIconTheme::renderUnShadeIcon()
{
    RenderDecorationButtonIcon18By18::renderUnShadeIcon();
}
}
