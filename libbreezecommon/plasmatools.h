/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "breeze.h"
#include "breezecommon_export.h"

namespace Breeze
{

/**
 * @brief Functions to interact with Plasma shell within Klassy
 *
 */
class BREEZECOMMON_EXPORT PlasmaTools
{
public:
    static Side taskManagerSide(const bool dotIfIconsOnlyTaskManager = false);
    static Side panelLocationToSide(const int panelLocation);
};

}
