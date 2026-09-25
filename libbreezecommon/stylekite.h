/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "stylekitedynamic.h"

#include <QPainter>

namespace Klassy
{

class RenderKite18By18 : public RenderKiteDynamic18By18
{
public:
    RenderKite18By18(QPainter *painter)
        : RenderKiteDynamic18By18(painter) { };

    void renderMinimizeIcon() override
    {
        renderCenteredLineMinimizeIcon(true);
    }

private:
};

}
