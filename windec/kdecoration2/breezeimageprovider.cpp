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
        switch (decorationButton->type()) {
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
            drawGenericButtonBackground(painter, decorationButton);
            drawDownArrow(painter, decorationButton);
            break;
            case KDecoration2::DecorationButtonType::KeepBelow:
            // TODO: Needs a checked state
            drawGenericButtonBackground(painter, decorationButton);
            drawDownArrow(painter, decorationButton, QPointF(0.0, -2.0));
            drawDownArrow(painter, decorationButton, QPointF(0.0,  2.0));
            break;
            case KDecoration2::DecorationButtonType::KeepAbove:
            // TODO: Needs a checked state
            drawGenericButtonBackground(painter, decorationButton);
            drawUpArrow(painter, decorationButton, QPointF(0.0, -2.0));
            drawUpArrow(painter, decorationButton, QPointF(0.0,  2.0));
            break;
            default:
            break;
        }
        return;
    }

    //__________________________________________________________________
    void ImageProvider::renderCloseButton(QPainter *painter, Breeze::Button *decorationButton) const
    {
        if (!decorationButton->decoration()) {
            return;
        }

        const QColor backgroundColor( decorationButton->backgroundColor() );
        if( backgroundColor.isValid() ) drawBackground(painter, decorationButton, backgroundColor);

        // draw the X
        QPen pen( decorationButton->foregroundColor() );
        pen.setWidth(2);
        painter->setPen(pen);

        const QSize &size   = decorationButton->size().toSize();
        painter->translate(size.width() / 2.0, size.height() / 2.0);
        painter->rotate(45.0);
        painter->drawLine(0, -size.height() / 4, 0, size.height() / 4);
        painter->drawLine(-size.width() / 4, 0, size.width() / 4, 0);

    }

    //__________________________________________________________________
    void ImageProvider::renderMaximizeButton(QPainter *painter, Breeze::Button *decorationButton) const
    {
        painter->save();
        drawGenericButtonBackground(painter, decorationButton);

        QPen pen( decorationButton->foregroundColor() );
        if (decorationButton->isChecked()) {
            // restore button
            const qreal width = (decorationButton->size().height() - 5) / 4;
            painter->translate(decorationButton->size().width() / 2.0, decorationButton->size().height() / 2.0);
            painter->rotate(45.0);
            pen.setWidth(2);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(-width, -width, width * 2, width * 2);
        } else {
            // maximize button
            drawUpArrow(painter, decorationButton);
        }

        painter->restore();
    }

    //__________________________________________________________________
    void ImageProvider::renderOnAllDesktopsButton(QPainter *painter, Breeze::Button *decorationButton) const
    {

        drawGenericButtonBackground(painter, decorationButton);
        const QColor foregroundColor( decorationButton->foregroundColor() );

        if( !decorationButton->isChecked())
        {

            Decoration *d = qobject_cast<Decoration*>( decorationButton->decoration() );
            if( !d ) return;

            QColor backgroundColor( decorationButton->backgroundColor() );
            if( !backgroundColor.isValid() ) backgroundColor = d->titleBarColor();

            // on all desktops
            painter->setPen(Qt::NoPen);

            painter->translate(decorationButton->size().width() / 2.0, decorationButton->size().height() / 2.0);
            const int radius = decorationButton->size().width() / 2 - 3;
            painter->setBrush( foregroundColor );
            painter->drawEllipse(-radius, -radius, radius * 2, radius * 2);

            painter->setBrush( backgroundColor );
            painter->drawEllipse(-1, -1, 2, 2);

        } else {

        }

    }

    //__________________________________________________________________
    void ImageProvider::renderShadeButton(QPainter *painter, Breeze::Button *decorationButton) const
    {
        drawGenericButtonBackground(painter, decorationButton);
        painter->save();
        QPen pen( decorationButton->foregroundColor() );
        pen.setWidth(2);
        painter->setPen(pen);
        painter->translate(decorationButton->size().width() / 2.0, decorationButton->size().height() / 2.0);
        const qreal width = decorationButton->size().width() /2.0 - 3.0;
        painter->drawLine(-width, -4.0, width, -4.0);
        painter->restore();
        if (decorationButton->isChecked()) {
            drawDownArrow(painter, decorationButton, QPointF(0.0, 2.0));
        } else {
            drawUpArrow(painter, decorationButton, QPointF(0.0, 2.0));
        }
    }

    //__________________________________________________________________
    void ImageProvider::drawGenericButtonBackground(QPainter *painter, Breeze::Button *decorationButton) const
    {
        const QColor backgroundColor( decorationButton->backgroundColor() );
        if( backgroundColor.isValid() ) drawBackground(painter, decorationButton, backgroundColor );
     }

    //__________________________________________________________________
    void ImageProvider::drawBackground(QPainter *painter, Breeze::Button *decorationButton, const QColor &color) const
    {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(decorationButton->isEnabled() ? color : QColor(color.red(), color.green(), color.blue(), color.alpha() * 0.6));
        painter->drawEllipse(QRectF(QPointF(0, 0), decorationButton->size()));
        painter->restore();
    }

    //__________________________________________________________________
    void ImageProvider::drawDownArrow(QPainter *painter, Breeze::Button *decorationButton, const QPointF &offset) const
    {
        painter->save();
        QPen pen(decorationButton->foregroundColor());
        // TODO: where do the magic values come from?
        const qreal width = (decorationButton->size().height() - 5) / 2;
        pen.setWidth(2);
        painter->setPen(pen);
        painter->translate(QPointF(decorationButton->size().width() / 2.0, decorationButton->size().height() / 2.0) + offset + QPointF(0.0, 0.707*width/2.0));
        painter->rotate(45.0);
        painter->drawLine(0, -width, 0, 0);
        painter->rotate(-90.0);
        painter->drawLine(0, -width, 0, 0);
        painter->restore();
    }

    //__________________________________________________________________
    void ImageProvider::drawUpArrow(QPainter *painter, Breeze::Button *decorationButton, const QPointF &offset) const
    {
        painter->save();
        QPen pen(decorationButton->foregroundColor());
        // TODO: where do the magic values come from?
        const qreal width = (decorationButton->size().height() - 5) / 2;
        pen.setWidth(2);
        painter->setPen(pen);
        painter->translate(QPointF(decorationButton->size().width() / 2.0, decorationButton->size().height() / 2.0) + offset - QPointF(0.0, 0.707*width/2.0));
        painter->rotate(225.0);
        painter->drawLine(0, 0, 0, -width);
        painter->rotate(-90.0);
        painter->drawLine(0, 0, 0, -width);
        painter->restore();
    }

}
