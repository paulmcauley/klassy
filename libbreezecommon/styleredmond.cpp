/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "styleredmond.h"

namespace Breeze
{

void RenderStyleRedmond18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderStyleRedmond18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false);
}

void RenderStyleRedmond18By18::renderRestoreIcon()
{
    renderOverlappingWindowsIcon();
}

void RenderStyleRedmond18By18::renderMinimizeIcon()
{
    // first determine the size of the maximize icon so the minimize icon can align with it
    QRectF maximizeRect = renderSquareMaximizeIcon(true);
    QPen pen = m_painter->pen();
    bool isOddPenWidth = true;

    if (!m_fromKstyle) {
        int roundedBoldPenWidth;
        if (m_boldButtonIcons) {
            // thicker pen in titlebar
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, m_squareMaximizeBoldPenWidthFactor);
        } else
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        pen.setWidthF(roundedBoldPenWidth);
    }

    // make excessively thick pen widths translucent to balance with other buttons
    qreal opacity = straightLineOpacity();
    QColor penColor = pen.color();
    penColor.setAlphaF(penColor.alphaF() * opacity);
    pen.setColor(penColor);

    m_painter->setPen(pen);

    // horizontal line
    // original y position in design was 12.5 -- this is often too high
    if (isOddPenWidth) {
        m_painter->drawLine(
            snapToNearestPixel(QPointF(4.5, maximizeRect.bottom()), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Up, ThresholdRound::Down),
            snapToNearestPixel(QPointF(13.5, maximizeRect.bottom()), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Up, ThresholdRound::Down));

    } else {
        m_painter->drawLine(
            snapToNearestPixel(QPointF(4.5, maximizeRect.bottom()), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Up, ThresholdRound::Down),
            snapToNearestPixel(QPointF(13.5, maximizeRect.bottom()), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Up, ThresholdRound::Down));
    }
}

// For consistency with breeze icon set
void RenderStyleRedmond18By18::renderKeepBehindIcon()
{
    renderKeepBehindIconAsFromBreezeIcons();
}

void RenderStyleRedmond18By18::renderKeepInFrontIcon()
{
    renderKeepInFrontIconAsFromBreezeIcons();
}

void RenderStyleRedmond18By18::renderContextHelpIcon()
{
    renderRounderAndBolderContextHelpIcon();
}

}
