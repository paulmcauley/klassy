/*
 * SPDX-FileCopyrightText: 2025 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylekisweet.h"

namespace Breeze
{

void RenderStyleKisweet18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderStyleKisweet18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false, 0.025, true);
}

void RenderStyleKisweet18By18::renderFloatIcon()
{
    renderOverlappingWindowsIcon(true);
}

void RenderStyleKisweet18By18::renderMinimizeIcon()
{
    renderTinySquareMinimizeIcon(true);
}

// For consistency with breeze icon set
void RenderStyleKisweet18By18::renderKeepBehindIcon()
{
    renderKeepBehindIconAsFromBreezeIcons();
    // renderOverlappingWindowsIcon(false, -1, false, true);
}

void RenderStyleKisweet18By18::renderKeepInFrontIcon()
{
    renderKeepInFrontIconAsFromBreezeIcons();
    // renderOverlappingWindowsIcon(false, -1, true, false);
}

void RenderStyleKisweet18By18::renderContextHelpIcon()
{
    renderRounderAndBolderContextHelpIcon();
}

}
