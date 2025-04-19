/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylefluent.h"

namespace Breeze
{

void RenderStyleFluent18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false, 20);
}

void RenderStyleFluent18By18::renderRestoreIcon()
{
    renderOverlappingWindowsIcon(false, 20);
}

}
