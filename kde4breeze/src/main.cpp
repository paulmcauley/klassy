/***************************************************************************
 *   Copyright (C) 2014 by Marco Martin <mart@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStyleFactory>
#include <kdelibs4migration.h>

#include <KConfigGroup>
#include <kconfig.h>

void updateKdeGlobals()
{
    Kdelibs4Migration migration;
    //Apply Breeze color scheme
    KConfig config(migration.locateLocal("config", "kdeglobals"));

    //use QtCurve only if installed
    if (QStyleFactory::keys().contains("QtCurve")) {
        KConfigGroup group(&config, "General");
        group.writeEntry("ColorScheme", "Breeze");
        group.writeEntry("widgetStyle", "qtcurve");
        group.sync();
    }

    //TODO: write icon theme
}

void applyQtCurveConfig()
{
    QString src = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "apps/QtCurve/Breeze.qtcurve");
    QString dest = QDir::homePath() + "/.config/qtcurve/stylerc";

    QFile::remove(dest);
    QFile::copy(src, dest);
}


int main(int argc, char **argv)
{

    QCoreApplication app(argc, argv);


    updateKdeGlobals();
    applyQtCurveConfig();

    return 0;
}
