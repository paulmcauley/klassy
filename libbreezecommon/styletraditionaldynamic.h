/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "styletraditional.h"

#include <QPainter>

namespace Klassy
{

class RenderTraditionalDynamic18By18 : public RenderTraditional18By18
{
public:
    RenderTraditionalDynamic18By18(QPainter *painter)
        : RenderTraditional18By18(painter) { };

    void renderMinimizeIcon() override
    {
        renderDynamicMinimizeIcon(true);
    }

private:
};

}
