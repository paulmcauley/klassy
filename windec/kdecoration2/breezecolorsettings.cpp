/*
* Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
* Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "breezecolorsettings.h"

#include <KConfigGroup>
#include <KSharedConfig>

namespace Breeze
{

    //________________________________________________________________
    ColorSettings::ColorSettings(const QPalette &palette)
    { init(palette); }

    //________________________________________________________________
    void ColorSettings::update(const QPalette &palette)
    { init(palette); }

    //________________________________________________________________
    void ColorSettings::init(const QPalette &palette)
    {
        m_palette = palette;

        {
            KConfigGroup wmConfig(KSharedConfig::openConfig(QStringLiteral("kdeglobals")), QStringLiteral("WM"));
            m_activeFrameColor      = wmConfig.readEntry("frame", palette.color(QPalette::Active, QPalette::Background));
            m_inactiveFrameColor    = wmConfig.readEntry("inactiveFrame", m_activeFrameColor);
            m_activeTitleBarColor   = wmConfig.readEntry("activeBackground", palette.color(QPalette::Active, QPalette::Highlight));
            m_inactiveTitleBarColor = wmConfig.readEntry("inactiveBackground", m_inactiveFrameColor);
            m_activeFontColor       = wmConfig.readEntry("activeForeground", palette.color(QPalette::Active, QPalette::HighlightedText));
            m_inactiveFontColor     = wmConfig.readEntry("inactiveForeground", m_activeFontColor.dark());
        }

        {
            KConfigGroup wmConfig(KSharedConfig::openConfig(QStringLiteral("kdeglobals")), QStringLiteral("Colors:Window"));
            m_closeButtonColor = wmConfig.readEntry("ForegroundNegative", QColor(237, 21, 2));
        }

    }

}
