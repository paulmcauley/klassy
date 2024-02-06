/*
 * SPDX-FileCopyrightText: 2022-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "colortools.h"

#include <KColorUtils>
#include <QIcon>

namespace Breeze
{

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

bool ColorTools::getHigherContrastForegroundColor(const QColor &foregroundColor,
                                                  const QColor &backgroundColor,
                                                  const qreal contrastThreshold,
                                                  QColor &outputColor,
                                                  QColor potentialReplacementColor)
{
    qreal contrastRatio = KColorUtils::contrastRatio(foregroundColor, backgroundColor);

    if (contrastRatio < contrastThreshold) {
        if (potentialReplacementColor.isValid()) {
            outputColor = potentialReplacementColor;
        } else {
            outputColor = getBlackOrWhiteForegroundForHighContrast(backgroundColor);
        }
        return true;
    } else {
        outputColor = foregroundColor;
        return false;
    }
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
    outputColor.setAlphaF(qMin(outputColor.alphaF() * alphaMixFactor, 1.0));
    return outputColor;
}

void ColorTools::convertAlphaToColor(QImage &image, const QColor tintColor)
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

void ColorTools::convertAlphaToColor(QIcon &icon, QSize iconSize, const QColor tintColor)
{
    QImage iconImage(icon.pixmap(iconSize).toImage());
    convertAlphaToColor(iconImage, tintColor);
    QPixmap pixmap(iconSize * iconImage.devicePixelRatioF());
    pixmap.setDevicePixelRatio(iconImage.devicePixelRatioF());
    pixmap.fill(Qt::transparent);
    std::unique_ptr<QPainter> painter = std::make_unique<QPainter>(&pixmap);
    painter->drawImage(QPoint(0, 0), iconImage);
    icon = QIcon(pixmap);
}
}
