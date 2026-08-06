/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "plasmatools.h"

#include <KConfig>
#include <map>

namespace Breeze
{

Side PlasmaTools::taskManagerSide(const bool dotIfIconsOnlyTaskManager)
{
    KConfig plasmaAppletsConfig(QStringLiteral("plasma-org.kde.plasma.desktop-appletsrc"), KConfig::OpenFlag::SimpleConfig);

    if (!plasmaAppletsConfig.hasGroup(QStringLiteral("Containments"))) {
        return SideBottom;
    }
    KConfigGroup containments = plasmaAppletsConfig.group(QStringLiteral("Containments"));

    std::map<int, int> taskPanelScreenAndLocation; // screen, location; auto-sorts by screen priority

    for (QString &containmentGroupString : containments.groupList()) {
        if (containments.group(containmentGroupString).readEntry(QStringLiteral("plugin"), QString()) == QStringLiteral("org.kde.panel")) {
            if (!containments.group(containmentGroupString).hasGroup(QStringLiteral("Applets")))
                return SideBottom;

            KConfigGroup panelAppletsGroup = containments.group(containmentGroupString).group(QStringLiteral("Applets"));
            for (QString &panelAppletsString : panelAppletsGroup.groupList()) {
                if (panelAppletsGroup.group(panelAppletsString).readEntry(QStringLiteral("plugin"), QString())
                    == QStringLiteral("org.kde.plasma.taskmanager")) {
                    int location = containments.group(containmentGroupString).readEntry(QStringLiteral("location"), -1);
                    if (location >= 0) {
                        int screen = containments.group(containmentGroupString).readEntry(QStringLiteral("lastScreen"), -1);
                        if (screen >= 0 && !taskPanelScreenAndLocation.contains(screen)) {
                            taskPanelScreenAndLocation.insert({screen, location});
                        }
                    }
                } else if (panelAppletsGroup.group(panelAppletsString).readEntry(QStringLiteral("plugin"), QString())
                           == QStringLiteral("org.kde.plasma.icontasks")) {
                    int location;
                    if (dotIfIconsOnlyTaskManager)
                        location = 0;
                    else
                        location = containments.group(containmentGroupString).readEntry(QStringLiteral("location"), -1);
                    if (location >= 0) {
                        int screen = containments.group(containmentGroupString).readEntry(QStringLiteral("lastScreen"), -1);
                        if (screen >= 0 && !taskPanelScreenAndLocation.contains(screen)) {
                            taskPanelScreenAndLocation.insert({screen, location});
                        }
                    }
                } else if (panelAppletsGroup.group(panelAppletsString).readEntry(QStringLiteral("plugin"), QString())
                           == QStringLiteral("org.kde.plasma.windowlist")) {
                    int location = containments.group(containmentGroupString).readEntry(QStringLiteral("location"), -1);
                    if (location >= 0) {
                        int screen = containments.group(containmentGroupString).readEntry(QStringLiteral("lastScreen"), -1);
                        if (screen >= 0 && !taskPanelScreenAndLocation.contains(screen)) {
                            taskPanelScreenAndLocation.insert({screen, location});
                        }
                    }
                }
            }
        }
    }

    if (taskPanelScreenAndLocation.size()) {
        auto it = taskPanelScreenAndLocation.begin();
        return (panelLocationToSide(it->second)); // use only the highest priority screen to determine the taskmanager location
    }

    return SideBottom;
}

Side PlasmaTools::panelLocationToSide(const int panelLocation)
{
    switch (panelLocation) {
    case 4:
    default:
        return SideBottom;
    case 3:
        return SideTop;
    case 5:
        return SideLeft;
    case 6:
        return SideRight;
    case 0:
        return SideDot;
    }
}

}
