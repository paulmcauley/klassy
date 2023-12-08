/*
 * SPDX-FileCopyrightText: 2022-2023 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylesystemicontheme.h"
#include <QIcon>
#include <vector>

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
        QImage tintedIcon(convertAlphaToColorOnTransparent(icon.pixmap(pixmapSize).toImage(), m_pen.color()));

        m_painter->drawImage(rect, tintedIcon);
    } else
        icon.paint(m_painter, rect);
}

QImage RenderStyleSystemIconTheme::convertAlphaToColorOnTransparent(const QImage &srcImage, const QColor &tintColor)
{
    // copy raw srcImage data to STL container
    std::vector<QRgb> pixels;
    qreal totalPixels = srcImage.height() * srcImage.width();
    pixels.resize(totalPixels);
    memmove(pixels.data(), srcImage.bits(), totalPixels * sizeof(QRgb));

    QColor outputColor(tintColor);
    int alpha;

    // for each pixel with opacity set the colour to the tintColor and alphablend
    // TODO:enable the following line instead with #include <execution> for parallelism, requires C++17 gcc9.1, TBB
    // (https://oneapi-src.github.io/oneTBB/GSG/integrate.html#integrate)
    //  (TBB conflicts with Qt - need to set add_definitions(-DQT_NO_KEYWORDS) to CMake and replace emit, signal, slot and foreach keywords)
    // std::for_each(std::execution::parallel_unsequenced_policy,
    std::for_each(pixels.begin(), pixels.end(), [&](QRgb &pixel) {
        alpha = qAlpha(pixel);
        if (alpha > 0) {
            outputColor.setAlphaF((qreal(alpha) / 255) * tintColor.alphaF());
            pixel = outputColor.rgba();
        }
    });

    return (QImage((uchar *)pixels.data(), srcImage.width(), srcImage.height(), QImage::Format_ARGB32));
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
