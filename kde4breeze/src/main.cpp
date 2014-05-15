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

// application header
#include "application.h"
// KDE headers
#include <KDE/KAboutData>
#include <KDE/KCmdLineArgs>
#include <KDE/KLocale>

static const char description[] =
    I18N_NOOP("Apply Breeze as KDE4 default settings");

static const char version[] = "0.1";

int main(int argc, char **argv)
{
    KAboutData about("simple", 0, ki18n("kde4breeze"), version, ki18n(description),
                     KAboutData::License_GPL, ki18n("(C) 2014 Marco Martin"), KLocalizedString(), 0, "mart@kde.org");
    KCmdLineArgs::init(argc, argv, &about);

    Application app;

    return app.exec();
}
