/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "styletraditional.h"

namespace Klassy
{

void RenderTraditional18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderTraditional18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false);
}

void RenderTraditional18By18::renderFloatIcon()
{
    renderOverlappingWindowsIcon(false);
}

void RenderTraditional18By18::renderMinimizeIcon()
{
    renderDynamicMinimizeIcon(false);
}
}
