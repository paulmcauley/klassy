/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylemetro.h"

namespace Klassy
{
void RenderMetro18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderMetro18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false);
}

void RenderMetro18By18::renderFloatIcon()
{
    renderOverlappingWindowsIcon(false);
}

void RenderMetro18By18::renderMinimizeIcon()
{
    renderCenteredLineMinimizeIcon();
}
}
