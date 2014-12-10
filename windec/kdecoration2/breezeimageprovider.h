#ifndef breezeimageprovider_h
#define breezeimageprovider_h

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
#include "breeze.h"

#include <kdecoration2/decorationdefines.h>

#include <QImage>

//* QPalette hash, to use in caches
uint qHash(const QPalette &pal);

namespace Breeze
{

    class Button;

    class ImageProvider final
    {
        public:
        ~ImageProvider();
        static ImageProvider *self();
        QImage button(Button *decorationButton);
        void clearCache(Button *decorationButton);

        void invalidate();

        void renderButton(QPainter *p, Button *decorationButton) const;

        private:

        ImageProvider();

        QImage renderButton(Button *decorationButton) const;

        void renderCloseButton(QPainter *p, Button *decorationButton) const;

        void renderShadeButton(QPainter *p, Button *decorationButton) const;

        void renderMaximizeButton(QPainter *p, Button *decorationButton) const;

        void renderOnAllDesktopsButton(QPainter *p, Button *decorationButton) const;

        void renderMinimizeButton(QPainter *p, Button *decorationButton) const;

        void renderKeepBelowButton(QPainter *p, Button *decorationButton) const;

        void renderKeepAboveButton(QPainter *p, Button *decorationButton) const;

        ColorSettings colorSettings(Button *decorationButton) const;

        ColorSettings colorSettings(const QPalette &pal) const;

        static ImageProvider *s_self;

        typedef QHash<ButtonState, QImage> ImagesForButtonState;

        typedef QHash<bool, ImagesForButtonState> ImagesForDecoState;

        typedef QHash<KDecoration2::DecorationButtonType, ImagesForDecoState> ImagesForButton;

        QHash<QPalette, ImagesForButton> m_images;

        QList<ColorSettings> m_colorSettings;

    };

}

#endif
