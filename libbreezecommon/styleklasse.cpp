/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "styleklasse.h"

namespace Breeze
{

void RenderStyleKlasse18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderStyleKlasse18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false);
}

void RenderStyleKlasse18By18::renderFloatIcon()
{
    renderOverlappingWindowsIcon(false);
}

void RenderStyleKlasse18By18::renderMinimizeIcon()
{
    renderTinySquareMinimizeIcon();
}

// For consistency with breeze icon set
void RenderStyleKlasse18By18::renderKeepBehindIcon()
{
    renderKeepBehindIconAsFromBreezeIcons();
}

void RenderStyleKlasse18By18::renderKeepInFrontIcon()
{
    renderKeepInFrontIconAsFromBreezeIcons();
}

void RenderStyleKlasse18By18::renderContextHelpIcon()
{
    renderRounderAndBolderContextHelpIcon();
}
}
