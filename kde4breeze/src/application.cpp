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

#include "application.h"

#include <QDebug>
#include <QDir>

#include <KGlobal>
#include <KStandardDirs>

Application::Application()
    : KApplication()
{
    //Apply Breeze color scheme
    KSharedPtr<KSharedConfig>config = KSharedConfig::openConfig("kdeglobals");
    KConfigGroup group(config, "General");
    group.writeEntry("ColorScheme", "Breeze");

    group.writeEntry("widgetStyle", "oxygen");

    //TODO: icons

    applyQtCurveConfig();
}

Application::~Application()
{
}

void Application::applyQtCurveConfig()
{
    QString src = KGlobal::dirs()->locate("data", "QtCurve/Breeze.qtcurve");
    QString dest = QDir::homePath() + "/.config/qtcurve/stylerc";

    QFile::remove(dest);
    QFile::copy(src, dest);
}

#include "application.moc"
