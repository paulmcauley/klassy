/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "styleredmond11.h"

namespace Breeze
{

void RenderStyleRedmond1118By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false, 20);
}

void RenderStyleRedmond1118By18::renderRestoreIcon()
{
    renderOverlappingWindowsIcon(20);
}

}
