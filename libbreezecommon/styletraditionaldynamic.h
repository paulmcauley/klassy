/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "styletraditional.h"

#include <QPainter>

namespace Breeze
{

class RenderStyleTraditionalDynamic18By18 : public RenderStyleTraditional18By18
{
public:
    RenderStyleTraditionalDynamic18By18(QPainter *painter)
        : RenderStyleTraditional18By18(painter) { };

    void renderMinimizeIcon() override
    {
        renderDynamicMinimizeIcon(true);
    }

private:
};

}
