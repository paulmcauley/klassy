/*
 * SPDX-FileCopyrightText: 2025 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylekisweet.h"

namespace Klassy
{

void RenderKisweet18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderKisweet18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false, 0.025, true);
}

void RenderKisweet18By18::renderFloatIcon()
{
    renderOverlappingWindowsIcon(true);
}

void RenderKisweet18By18::renderMinimizeIcon()
{
    renderTinySquareMinimizeIcon(true);
}
}
