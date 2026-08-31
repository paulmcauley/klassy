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

void PlasmaTools::taskManagerTypeAndSide(TaskManagerType &taskManagerType, Side &taskManagerSide)
{
    // set defaults
    taskManagerType = TaskManagerType::IconsAndTextTaskManager;
    taskManagerSide = Side::SideBottom;

    KConfig plasmaAppletsConfig(QStringLiteral("plasma-org.kde.plasma.desktop-appletsrc"), KConfig::OpenFlag::SimpleConfig);

    if (!plasmaAppletsConfig.hasGroup(QStringLiteral("Containments"))) {
        return;
    }
    KConfigGroup containments = plasmaAppletsConfig.group(QStringLiteral("Containments"));

    struct LocationAndType {
        int location;
        TaskManagerType type;
    };

    std::map<int, LocationAndType> taskPanelScreenLocationAndType; // screen, location; auto-sorts by screen priority

    for (QString &containmentGroupString : containments.groupList()) {
        if (containments.group(containmentGroupString).readEntry(QStringLiteral("plugin"), QString()) == QStringLiteral("org.kde.panel")) {
            if (!containments.group(containmentGroupString).hasGroup(QStringLiteral("Applets")))
                return;

            KConfigGroup panelAppletsGroup = containments.group(containmentGroupString).group(QStringLiteral("Applets"));
            for (QString &panelAppletsString : panelAppletsGroup.groupList()) {
                if (panelAppletsGroup.group(panelAppletsString).readEntry(QStringLiteral("plugin"), QString())
                    == QStringLiteral("org.kde.plasma.taskmanager")) {
                    int location = containments.group(containmentGroupString).readEntry(QStringLiteral("location"), -1);
                    if (location > 0) {
                        int screen = containments.group(containmentGroupString).readEntry(QStringLiteral("lastScreen"), -1);
                        if (screen >= 0 && !taskPanelScreenLocationAndType.contains(screen)) {
                            taskPanelScreenLocationAndType.insert({screen, {location, TaskManagerType::IconsAndTextTaskManager}});
                        }
                    }
                } else if (panelAppletsGroup.group(panelAppletsString).readEntry(QStringLiteral("plugin"), QString())
                           == QStringLiteral("org.kde.plasma.icontasks")) {
                    int location = containments.group(containmentGroupString).readEntry(QStringLiteral("location"), -1);
                    if (location > 0) {
                        int screen = containments.group(containmentGroupString).readEntry(QStringLiteral("lastScreen"), -1);
                        if (screen >= 0 && !taskPanelScreenLocationAndType.contains(screen)) {
                            taskPanelScreenLocationAndType.insert({screen, {location, TaskManagerType::IconsOnlyTaskManager}});
                        }
                    }

                } else if (panelAppletsGroup.group(panelAppletsString).readEntry(QStringLiteral("plugin"), QString())
                           == QStringLiteral("org.kde.plasma.windowlist")) {
                    int location = containments.group(containmentGroupString).readEntry(QStringLiteral("location"), -1);
                    if (location > 0) {
                        int screen = containments.group(containmentGroupString).readEntry(QStringLiteral("lastScreen"), -1);
                        if (screen >= 0 && !taskPanelScreenLocationAndType.contains(screen)) {
                            taskPanelScreenLocationAndType.insert({screen, {location, TaskManagerType::WindowList}});
                        }
                    }
                }
            }
        }
    }

    if (taskPanelScreenLocationAndType.size()) {
        auto it = taskPanelScreenLocationAndType.begin(); // use only the highest priority screen to determine the taskmanager location
        taskManagerType = it->second.type;
        taskManagerSide = panelLocationToSide(it->second.location);
    }
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
    }
}

}
