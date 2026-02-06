/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#define KDECORATION_VERSION KDECORATION_VERSION_CHECK(KDECORATION_VERSION_MAJOR, KDECORATION_VERSION_MINOR, KDECORATION_VERSION_PATCH)
#define KDECORATION_VERSION_CHECK(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
