/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "stylekitedynamic.h"

#include <QPainter>

namespace Breeze
{

class RenderStyleKite18By18 : public RenderStyleKiteDynamic18By18
{
public:
    RenderStyleKite18By18(QPainter *painter)
        : RenderStyleKiteDynamic18By18(painter) { };

    void renderMinimizeIcon() override
    {
        renderCenteredLineMinimizeIcon(true);
    }

private:
};

}
