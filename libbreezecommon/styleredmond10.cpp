/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "styleredmond10.h"

namespace Breeze
{
void RenderStyleRedmond1018By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderStyleRedmond1018By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false);
}

void RenderStyleRedmond1018By18::renderRestoreIcon()
{
    renderOverlappingWindowsIcon();
}


void RenderStyleRedmond1018By18::renderMinimizeIcon()
{
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
    if (isOddPenWidth) {
        m_painter->drawLine(snapToNearestPixel(QPointF(4.5, 9.5), SnapPixel::ToHalf, SnapPixel::ToHalf),
                            snapToNearestPixel(QPointF(13.5, 9.5), SnapPixel::ToHalf, SnapPixel::ToHalf));

    } else {
        m_painter->drawLine(snapToNearestPixel(QPointF(4.5, 9.5), SnapPixel::ToWhole, SnapPixel::ToWhole),
                            snapToNearestPixel(QPointF(13.5, 9.5), SnapPixel::ToWhole, SnapPixel::ToWhole));
    }
}

// For consistency with breeze icon set
void RenderStyleRedmond1018By18::renderKeepBehindIcon()
{
    renderKeepBehindIconAsFromBreezeIcons();
}

void RenderStyleRedmond1018By18::renderKeepInFrontIcon()
{
    renderKeepInFrontIconAsFromBreezeIcons();
}

void RenderStyleRedmond1018By18::renderContextHelpIcon()
{
    renderRounderAndBolderContextHelpIcon();
}

}
