/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "breeze.h"
#include "klassycommon_export.h"

namespace Klassy
{

/**
 * @brief Functions to interact with Plasma shell within Klassy
 *
 */
class KLASSYCOMMON_EXPORT PlasmaTools
{
public:
    /**
     * @param taskManagerType Outputs the first task manager type on the primary screen
     * @param taskManagerSide Outputs the side of the primary screen on which the first task manager is placed
     */
    static void taskManagerTypeAndSide(TaskManagerType &taskMananagerType, Side &taskManagerSide);
    static Side panelLocationToSide(const int panelLocation);
};

}
