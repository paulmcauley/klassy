/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "styleklassy.h"

namespace Breeze
{

void RenderStyleKlassy18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderStyleKlassy18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false);
}

void RenderStyleKlassy18By18::renderRestoreIcon()
{
    renderOverlappingWindowsIcon();
}

void RenderStyleKlassy18By18::renderMinimizeIcon()
{
    renderTinySquareMinimizeIcon();
}


// For consistency with breeze icon set
void RenderStyleKlassy18By18::renderKeepBehindIcon()
{
    renderKeepBehindIconAsFromBreezeIcons();
}

void RenderStyleKlassy18By18::renderKeepInFrontIcon()
{
    renderKeepInFrontIconAsFromBreezeIcons();
}

void RenderStyleKlassy18By18::renderContextHelpIcon()
{
    renderRounderAndBolderContextHelpIcon();
}
}
