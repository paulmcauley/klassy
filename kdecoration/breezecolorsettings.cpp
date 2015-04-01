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
    ColorSettings::ColorSettings(const QPalette &palette, const KDecoration2::DecoratedClient &client)
    { init(palette, client); }

    //________________________________________________________________
    void ColorSettings::update(const QPalette &palette, const KDecoration2::DecoratedClient &client)
    { init(palette, client); }

    //________________________________________________________________
    void ColorSettings::init(const QPalette &palette, const KDecoration2::DecoratedClient &client)
    {
        using KDecoration2::ColorRole;
        using KDecoration2::ColorGroup;

        m_palette = palette;
        m_activeFrameColor = client.color(ColorGroup::Active, ColorRole::Frame);
        m_inactiveFrameColor = client.color(ColorGroup::Inactive, ColorRole::Frame);

        m_activeTitleBarColor = client.color(ColorGroup::Active, ColorRole::TitleBar);
        m_inactiveTitleBarColor = client.color(ColorGroup::Inactive, ColorRole::TitleBar);

        m_activeFontColor = client.color(ColorGroup::Active, ColorRole::Foreground);
        m_inactiveFontColor = client.color(ColorGroup::Inactive, ColorRole::Foreground);

        m_closeButtonColor = client.color(ColorGroup::Warning, ColorRole::Foreground);
    }

}
