/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylemetro.h"

namespace Breeze
{
void RenderStyleMetro18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderStyleMetro18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false);
}

void RenderStyleMetro18By18::renderFloatIcon()
{
    renderOverlappingWindowsIcon(false);
}

void RenderStyleMetro18By18::renderMinimizeIcon()
{
    renderCenteredLineMinimizeIcon();
}
}
