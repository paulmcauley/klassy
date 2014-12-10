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

#include "breezeimageprovider.h"
#include "breezebutton.h"
#include "breezedecoration.h"

#include <QPainter>

#include <KDecoration2/DecoratedClient>

//__________________________________________________________________
uint qHash(const QPalette &pal)
{
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    stream << pal;

    return byteArray.toBase64().toUInt();
}

namespace Breeze
{

    ImageProvider *ImageProvider::s_self = nullptr;

    ImageProvider::ImageProvider() = default;

    ImageProvider::~ImageProvider()
    { s_self = nullptr; }

    ImageProvider *ImageProvider::self()
    {
        // TODO: this is not thread safe!
        if (!s_self) s_self = new ImageProvider();
        return s_self;
    }

    void ImageProvider::invalidate()
    {
        m_images.clear();
    }

    //__________________________________________________________________
    static ButtonState stateForButton(Button *decorationButton)
    {
        if (!decorationButton->isEnabled()) return ButtonState::Disabled;
        else if (decorationButton->isChecked()) {

            if (decorationButton->isPressed()) return ButtonState::CheckedPressed;
            if (decorationButton->isHovered()) return ButtonState::CheckedHovered;
            return ButtonState::Checked;

        } else if (decorationButton->isPressed()) return ButtonState::Pressed;
        else if (decorationButton->isHovered()) return ButtonState::Hovered;
        else if (decorationButton->isStandAlone()) return ButtonState::Preview;
        else return ButtonState::Normal;
    }

    //__________________________________________________________________
    QImage ImageProvider::button(Breeze::Button *decorationButton)
    {
        auto paletteIt = m_images.begin();
        if (!decorationButton->decoration()) { return QImage(); }

        auto client = decorationButton->decoration()->client().data();
        if (paletteIt == m_images.end() || paletteIt.key() != client->palette())
        {
            paletteIt = m_images.find(client->palette());
        }

        if (paletteIt == m_images.end())
        {
            const QPalette pal = client->palette();
            m_images.insert(pal, ImagesForButton());
            m_colorSettings.append(ColorSettings(pal));
            paletteIt = m_images.find(client->palette());
        }
        Q_ASSERT(paletteIt != m_images.end());

        auto it = paletteIt.value().find(decorationButton->type());
        if (it == paletteIt.value().end()) {
            auto hash = ImagesForDecoState();
            hash.insert(true, ImagesForButtonState());
            hash.insert(false, ImagesForButtonState());
            paletteIt.value().insert(decorationButton->type(), hash);
            it = paletteIt.value().find(decorationButton->type());
        }
        Q_ASSERT(it != paletteIt.value().end());

        auto it2 = it.value().find(client->isActive());
        Q_ASSERT(it2 != it.value().end());

        const ButtonState state = stateForButton(decorationButton);
        auto it3 = it2.value().find(state);
        if (it3 == it2.value().end()) {
            QImage image = renderButton(decorationButton);
            it2.value().insert(state, image);
            it3 = it2.value().find(state);
        }
        Q_ASSERT(it3 != it2.value().end());
        return it3.value();
    }

    //__________________________________________________________________
    void ImageProvider::clearCache(Breeze::Button *decorationButton)
    {
        auto paletteIt = m_images.begin();
        if (!decorationButton->decoration()) {
            return;
        }
        const QPalette &palette = decorationButton->decoration()->client().data()->palette();
        if (paletteIt == m_images.end() || paletteIt.key() != palette) {
            paletteIt = m_images.find(palette);
        }
        if (paletteIt == m_images.end()) {
            return;
        }

        auto it = paletteIt.value().find(decorationButton->type());
        if (it == paletteIt.value().end()) {
            return;
        }
        paletteIt.value().erase(it);
    }

    //__________________________________________________________________
    ColorSettings ImageProvider::colorSettings(const QPalette &pal) const
    {
        for (const ColorSettings &colorSettings : m_colorSettings) {
            if (colorSettings.palette() == pal) {
                return colorSettings;
            }
        }
        Q_ASSERT(false);
        return ColorSettings(pal);
    }

    //__________________________________________________________________
    ColorSettings ImageProvider::colorSettings(Breeze::Button *decorationButton) const
    {
        if (!decorationButton->decoration()) {
            return colorSettings(QPalette());
        }
        return colorSettings(decorationButton->decoration()->client().data()->palette());
    }

    //__________________________________________________________________
    QImage ImageProvider::renderButton(Breeze::Button *decorationButton) const
    {
        QImage image(decorationButton->size().toSize(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);

        {
            QPainter p(&image);
            p.setRenderHint(QPainter::Antialiasing);
            renderButton( &p, decorationButton );
        }

        return image;
    }

    //__________________________________________________________________
    void ImageProvider::renderButton(QPainter *painter, Breeze::Button *decorationButton) const
    {

        // scale painter so that its window matches 18x18
        // this makes all further rendering and painting simpler
        painter->save();
        painter->scale( decorationButton->geometry().width()/18, decorationButton->geometry().height()/18 );
        painter->setRenderHints( QPainter::Antialiasing );

        // render background
        const QColor backgroundColor( decorationButton->backgroundColor() );
        if( backgroundColor.isValid() )
        {
            painter->setPen( Qt::NoPen );
            painter->setBrush( backgroundColor );
            painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
        }

        // setup pen for rendering the mark
        QPen pen( decorationButton->foregroundColor() );
        pen.setCapStyle( Qt::RoundCap );
        pen.setJoinStyle( Qt::MiterJoin );
        pen.setWidth(2);
        painter->setPen( pen );
        painter->setBrush( Qt::NoBrush );

        switch (decorationButton->type())
        {
            case KDecoration2::DecorationButtonType::Close:
            renderCloseButton(painter, decorationButton);
            break;
            case KDecoration2::DecorationButtonType::Maximize:
            renderMaximizeButton(painter, decorationButton);
            break;
            case KDecoration2::DecorationButtonType::OnAllDesktops:
            renderOnAllDesktopsButton(painter, decorationButton);
            break;
            case KDecoration2::DecorationButtonType::Shade:
            renderShadeButton(painter, decorationButton);
            break;
            case KDecoration2::DecorationButtonType::Minimize:
            renderMinimizeButton( painter, decorationButton );
            break;
            case KDecoration2::DecorationButtonType::KeepBelow:
            renderKeepBelowButton( painter, decorationButton );
            break;
            case KDecoration2::DecorationButtonType::KeepAbove:
            renderKeepAboveButton( painter, decorationButton );
            break;
            default:
            break;
        }

        painter->restore();

        return;
    }

    //__________________________________________________________________
    void ImageProvider::renderCloseButton(QPainter *painter, Breeze::Button* ) const
    {

        const qreal penWidth( painter->pen().width()/2 );
        painter->drawLine( QPointF( 5 + penWidth, 5 + penWidth ), QPointF( 13 - penWidth, 13 - penWidth ) );
        painter->drawLine( 13 - penWidth, 5 + penWidth, 5 + penWidth, 13 - penWidth );

    }

    //__________________________________________________________________
    void ImageProvider::renderMaximizeButton(QPainter *painter, Breeze::Button *decorationButton) const
    {

        const qreal penWidth( painter->pen().width()/2 );
        if (decorationButton->isChecked())
        {

            QPen pen( painter->pen() );
            pen.setJoinStyle( Qt::RoundJoin );
            painter->setPen( pen );

            painter->drawPolygon( QPolygonF()
                << QPointF( 3.5 + penWidth, 9 )
                << QPointF( 9, 3.5 + penWidth )
                << QPointF( 14.5 - penWidth, 9 )
                << QPointF( 9, 14.5 - penWidth ) );

        } else {

            painter->drawPolyline( QPolygonF()
                << QPointF( 3.5 + penWidth, 11.5 - penWidth )
                << QPointF( 9, 5.5 + penWidth )
                << QPointF( 14.5 - penWidth, 11.5 - penWidth ) );

        }

    }

    //__________________________________________________________________
    void ImageProvider::renderOnAllDesktopsButton(QPainter *painter, Breeze::Button *decorationButton) const
    {

        // get foreground
        const QColor foregroundColor( decorationButton->foregroundColor() );
        painter->setPen( Qt::NoPen );
        painter->setBrush( foregroundColor );

        if( decorationButton->isChecked())
        {

            // outer ring
            painter->drawEllipse( QRectF( 3, 3, 12, 12 ) );

            // center dot
            QColor backgroundColor( decorationButton->backgroundColor() );
            Decoration *d = qobject_cast<Decoration*>( decorationButton->decoration() );
            if( !backgroundColor.isValid() && d ) backgroundColor = d->titleBarColor();

            if( backgroundColor.isValid() )
            {
                painter->setBrush( backgroundColor );
                painter->drawEllipse( QRectF( 8, 8, 2, 2 ) );
            }

        } else {

            painter->drawRoundedRect( QRectF( 6, 2, 6, 9 ), 1.5, 1.5 );
            painter->drawRect( QRectF( 4, 10, 10, 2 ) );
            painter->drawRoundRect( QRectF( 8, 12, 2, 4 ) );

        }

    }

    //__________________________________________________________________
    void ImageProvider::renderShadeButton(QPainter *painter, Breeze::Button *decorationButton) const
    {

        const qreal penWidth( painter->pen().width()/2 );
        if (decorationButton->isChecked())
        {

            painter->drawLine( 3 + penWidth, 5.5 + penWidth, 15 - penWidth, 5.5+penWidth );
            painter->drawPolyline( QPolygonF()
                << QPointF( 3.5 + penWidth, 8.5 + penWidth )
                << QPointF( 9, 14.5 - penWidth )
                << QPointF( 14.5 - penWidth, 8.5 + penWidth ) );

        } else {

            painter->drawLine( 3 + penWidth, 5.5 + penWidth, 15 - penWidth, 5.5+penWidth );
            painter->drawPolyline( QPolygonF()
                << QPointF( 3.5 + penWidth, 14.5 - penWidth )
                << QPointF( 9, 8.5 + penWidth )
                << QPointF( 14.5 - penWidth, 14.5 - penWidth ) );

        }

    }

    //__________________________________________________________________
    void ImageProvider::renderMinimizeButton(QPainter *painter, Breeze::Button*) const
    {
        const qreal penWidth( painter->pen().width()/2 );
        painter->drawPolyline( QPolygonF()
            << QPointF( 3.5 + penWidth, 6.5 + penWidth )
            << QPointF( 9, 12.5 - penWidth )
            << QPointF( 14.5 - penWidth, 6.5 + penWidth ) );
    }

    //__________________________________________________________________
    void ImageProvider::renderKeepBelowButton(QPainter *painter, Breeze::Button *) const
    {
        const qreal penWidth( painter->pen().width()/2 );
        painter->drawPolyline( QPolygonF()
            << QPointF( 3.5 + penWidth, 4.5 + penWidth )
            << QPointF( 9, 10.5 - penWidth )
            << QPointF( 14.5 - penWidth, 4.5 + penWidth ) );

        painter->drawPolyline( QPolygonF()
            << QPointF( 3.5 + penWidth, 8.5 + penWidth )
            << QPointF( 9, 14.5 - penWidth )
            << QPointF( 14.5 - penWidth, 8.5 + penWidth ) );
    }

    //__________________________________________________________________
    void ImageProvider::renderKeepAboveButton(QPainter *painter, Breeze::Button *) const
    {
        const qreal penWidth( painter->pen().width()/2 );
        painter->drawPolyline( QPolygonF()
            << QPointF( 3.5 + penWidth, 9.5 - penWidth )
            << QPointF( 9, 3.5 + penWidth )
            << QPointF( 14.5 - penWidth, 9.5 - penWidth ) );

        painter->drawPolyline( QPolygonF()
            << QPointF( 3.5 + penWidth, 13.5 - penWidth )
            << QPointF( 9, 7.5 + penWidth )
            << QPointF( 14.5 - penWidth, 13.5 - penWidth ) );
    }

}
