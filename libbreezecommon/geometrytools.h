/*
 * SPDX-FileCopyrightText: 2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "breeze.h"
#include "klassycommon_export.h"

#include <QPainterPath>

namespace Klassy
{

/**
 * @brief Functions to manipulate geometry within Klassy
 *        To be used as common code base across both kdecoration and kstyle.
 */
class KLASSYCOMMON_EXPORT GeometryTools
{
public:
    static QPainterPath roundedPath(const QRectF &rect,
                                    const Corners corners,
                                    const qreal radius,
                                    const Sides sides = AllSides,
                                    const qreal subtractTopLeft = 0,
                                    const qreal subtractTopRight = 0,
                                    const qreal subtractBottomRight = 0,
                                    const qreal subtractBottomLeft = 0);
};

}
