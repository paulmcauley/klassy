/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
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
#include "breezebutton.h"
#include "breezeimageprovider.h"

#include <KDecoration2/DecoratedClient>

#include <QPainter>

namespace Breeze
{

    //__________________________________________________________________
    Button::Button(KDecoration2::DecorationButtonType type, Decoration* decoration, QObject* parent)
        : DecorationButton(type, decoration, parent)
    {
        const int height = decoration->captionHeight();
        setGeometry(QRect(0, 0, height, height));
        connect(decoration, &Decoration::bordersChanged, this, [this, decoration]
        {
            const int height = decoration->captionHeight();
            if (height == geometry().height()) return;
            ImageProvider::self()->clearCache(this);
            setGeometry(QRectF(geometry().topLeft(), QSizeF(height, height)));
        });
    }

    //__________________________________________________________________
    Button::Button(QObject *parent, const QVariantList &args)
        : DecorationButton(args.at(0).value<KDecoration2::DecorationButtonType>(), args.at(1).value<Decoration*>(), parent)
        , m_standalone(true)
    {}

    //__________________________________________________________________
    Button *Button::create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
    {
        if (Decoration *d = qobject_cast<Decoration*>(decoration))
        {
            Button *b = new Button(type, d, parent);
            if (type == KDecoration2::DecorationButtonType::Menu)
            {
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::iconChanged, b, [b]() { b->update(); });
            }
            return b;
        }
        return nullptr;
    }

    //__________________________________________________________________
    Button::~Button() = default;

    //__________________________________________________________________
    void Button::paint(QPainter *painter, const QRect &repaintRegion)
    {
        Q_UNUSED(repaintRegion)

        if (!decoration())
        { return; }

        #if 0
        painter->save();
        painter->setPen( Qt::red );
        painter->setBrush( Qt::NoBrush );
        painter->setRenderHints( QPainter::Antialiasing );
        painter->drawRect( QRectF( geometry() ).adjusted( 0.5, 0.5, -0.5, -0.5 ) );
        painter->restore();
        #endif

        // TODO: optimize based on repaintRegion
        if (type() == KDecoration2::DecorationButtonType::Menu)
        {
            const QPixmap pixmap = decoration()->client().data()->icon().pixmap(size().toSize());
            painter->drawPixmap(geometry().center() - QPoint(pixmap.width()/2, pixmap.height()/2), pixmap);

        } else {

            painter->drawImage(geometry().topLeft(), ImageProvider::self()->button(this));

        }

    }

} // namespace
