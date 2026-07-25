/*
 * SPDX-FileCopyrightText: 2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "geometrytools.h"

namespace Breeze
{

// from breezehelper.cpp
QPainterPath GeometryTools::roundedPath(const QRectF &rect, Corners corners, qreal radius, Sides sides, qreal penProtrusion)
{
    QPainterPath path;

    // simple cases
    if (corners == 0 && sides == AllSides) {
        path.addRect(rect);
        return path;
    }

    if (corners == AllCorners && sides == AllSides) {
        path.addRoundedRect(rect, radius, radius);
        return path;
    }

    const QSizeF cornerSize(2 * radius, 2 * radius);

    // rotate counterclockwise
    // top left corner
    if ((corners & CornerTopLeft) && (sides & SideTop) && (sides & SideLeft)) {
        path.moveTo(rect.topLeft() + QPointF(radius, 0));
        path.arcTo(QRectF(rect.topLeft(), cornerSize), 90, 90);

    } else {
        if (sides & SideTop) {
            path.moveTo(rect.topLeft());
        } else {
            path.moveTo(rect.topLeft() + QPointF(0, penProtrusion));
        }
    }

    // bottom left corner
    if ((corners & CornerBottomLeft) && (sides & SideLeft) && (sides & SideBottom)) {
        path.lineTo(rect.bottomLeft() - QPointF(0, radius));
        path.arcTo(QRectF(rect.bottomLeft() - QPointF(0, 2 * radius), cornerSize), 180, 90);

    } else {
        if (sides & SideLeft) {
            if (sides & SideBottom) {
                path.lineTo(rect.bottomLeft());
            } else {
                path.lineTo(rect.bottomLeft() - QPointF(0, penProtrusion));
            }
        } else {
            path.moveTo(rect.bottomLeft() + QPointF(penProtrusion, 0));
        }
    }

    // bottom right corner
    if ((corners & CornerBottomRight) && (sides & SideBottom) && (sides & SideRight)) {
        path.lineTo(rect.bottomRight() - QPointF(radius, 0));
        path.arcTo(QRectF(rect.bottomRight() - QPointF(2 * radius, 2 * radius), cornerSize), 270, 90);

    } else {
        if (sides & SideBottom) {
            if (sides & SideRight) {
                path.lineTo(rect.bottomRight());
            } else {
                path.lineTo(rect.bottomRight() - QPointF(penProtrusion, 0));
            }
        } else {
            path.moveTo(rect.bottomRight() - QPointF(0, penProtrusion));
        }
    }

    // top right corner
    if ((corners & CornerTopRight) && (sides & SideTop) && (sides & SideRight)) {
        path.lineTo(rect.topRight() + QPointF(0, radius));
        path.arcTo(QRectF(rect.topRight() - QPointF(2 * radius, 0), cornerSize), 0, 90);

    } else {
        if (sides & SideRight) {
            if (sides & SideTop) {
                path.lineTo(rect.topRight());
            } else {
                path.lineTo(rect.topRight() + QPointF(0, penProtrusion));
            }
        } else {
            path.moveTo(rect.topRight() - QPointF(penProtrusion, 0));
        }
    }

    if ((corners & CornerTopLeft) && (sides & SideTop) && (sides & SideLeft)) {
        path.lineTo(rect.topLeft() + QPointF(radius, 0));
    } else {
        if (sides & SideTop) {
            if (sides & SideLeft) {
                path.lineTo(rect.topLeft());
            } else {
                path.lineTo(rect.topLeft() + QPointF(penProtrusion, 0));
            }
        }
    }
    return path;
}

}
