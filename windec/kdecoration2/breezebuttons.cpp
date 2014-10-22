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
#include "breezebuttons.h"

#include <KDecoration2/DecoratedClient>

#include <QPainter>

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
{
    s_self = nullptr;
}

ImageProvider *ImageProvider::self()
{
    // TODO: this is not thread safe!
    if (!s_self) {
        s_self = new ImageProvider();
    }
    return s_self;
}

void ImageProvider::invalidate()
{
    m_images.clear();
}

static ButtonState stateForButton(KDecoration2::DecorationButton *decorationButton)
{
    if (!decorationButton->isEnabled()) {
        return ButtonState::Disabled;
    }
    if (decorationButton->isChecked()) {
        if (decorationButton->isPressed()) {
            return ButtonState::CheckedPressed;
        }
        if (decorationButton->isHovered()) {
            return ButtonState::CheckedHovered;
        }
        return ButtonState::Checked;
    }
    if (decorationButton->isPressed()) {
        return ButtonState::Pressed;
    }
    if (decorationButton->isHovered()) {
        return ButtonState::Hovered;
    }
    return ButtonState::Normal;
}

const QImage &ImageProvider::button(KDecoration2::DecorationButton *decorationButton)
{
    auto paletteIt = m_images.begin();
    if (paletteIt == m_images.end() || paletteIt.key() != decorationButton->decoration()->client()->palette()) {
        paletteIt = m_images.find(decorationButton->decoration()->client()->palette());
    }
    if (paletteIt == m_images.end()) {
        const QPalette pal = decorationButton->decoration()->client()->palette();
        m_images.insert(pal, ImagesForButton());
        m_colorSettings.append(ColorSettings(pal));
        paletteIt = m_images.find(decorationButton->decoration()->client()->palette());
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

    auto it2 = it.value().find(decorationButton->decoration()->client()->isActive());
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

void ImageProvider::clearCache(KDecoration2::DecorationButton *decorationButton)
{
    auto paletteIt = m_images.begin();
    if (paletteIt == m_images.end() || paletteIt.key() != decorationButton->decoration()->client()->palette()) {
        paletteIt = m_images.find(decorationButton->decoration()->client()->palette());
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

const ColorSettings &ImageProvider::colorSettings(const QPalette &pal) const
{
    for (const ColorSettings &colorSettings : m_colorSettings) {
        if (colorSettings.palette() == pal) {
            return colorSettings;
        }
    }
    Q_ASSERT(false);
    return ColorSettings(pal);
}

const ColorSettings &ImageProvider::colorSettings(KDecoration2::DecorationButton *decorationButton) const
{
    return colorSettings(decorationButton->decoration()->client()->palette());
}


QImage ImageProvider::renderButton(KDecoration2::DecorationButton *decorationButton) const
{
    QImage image(decorationButton->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    switch (decorationButton->type()) {
    case KDecoration2::DecorationButtonType::Close:
        renderCloseButton(&p, decorationButton);
        break;
    case KDecoration2::DecorationButtonType::Maximize:
        renderMaximizeButton(&p, decorationButton);
        break;
    case KDecoration2::DecorationButtonType::OnAllDesktops:
        renderOnAllDesktopsButton(&p, decorationButton);
        break;
    case KDecoration2::DecorationButtonType::Shade:
        renderShadeButton(&p, decorationButton);
        break;
    case KDecoration2::DecorationButtonType::Minimize:
        drawGenericButtonBackground(&p, decorationButton);
        drawDownArrow(&p, decorationButton);
        break;
    case KDecoration2::DecorationButtonType::KeepBelow:
        // TODO: Needs a checked state
        drawGenericButtonBackground(&p, decorationButton);
        drawDownArrow(&p, decorationButton, QPointF(0.0, -2.0));
        drawDownArrow(&p, decorationButton, QPointF(0.0,  2.0));
        break;
    case KDecoration2::DecorationButtonType::KeepAbove:
        // TODO: Needs a checked state
        drawGenericButtonBackground(&p, decorationButton);
        drawUpArrow(&p, decorationButton, QPointF(0.0, -2.0));
        drawUpArrow(&p, decorationButton, QPointF(0.0,  2.0));
        break;
    default:
        break;
    }

    return image;
}

void ImageProvider::renderCloseButton(QPainter *painter, KDecoration2::DecorationButton *decorationButton) const
{
    const bool active   = decorationButton->decoration()->client()->isActive();
    const QPalette &pal = decorationButton->decoration()->client()->palette();
    const bool pressed  = decorationButton->isPressed();
    const bool hovered  = decorationButton->isHovered();
    const QSize &size   = decorationButton->size();

    const QColor pressedColor = QColor(237, 21, 21);
    const QColor backgroundColor = pressed ? pressedColor : hovered ? pressedColor.lighter() : colorSettings(pal).font(active);
    drawBackground(painter, decorationButton, backgroundColor);

    // draw the X
    QPen pen(hovered || pressed ? colorSettings(pal).font(active) : colorSettings(pal).titleBarColor(active));
    pen.setWidth(2);
    painter->setPen(pen);
    painter->translate(size.width() / 2.0, size.height() / 2.0);
    painter->rotate(45.0);
    painter->drawLine(0, -size.height() / 4, 0, size.height() / 4);
    painter->drawLine(-size.width() / 4, 0, size.width() / 4, 0);
}

void ImageProvider::renderMaximizeButton(QPainter *painter, KDecoration2::DecorationButton *decorationButton) const
{
    painter->save();
    drawGenericButtonBackground(painter, decorationButton);

    QPen pen(foregroundColor(decorationButton));
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

void ImageProvider::renderOnAllDesktopsButton(QPainter *painter, KDecoration2::DecorationButton *decorationButton) const
{
    const bool active = decorationButton->decoration()->client()->isActive();
    painter->save();
    drawGenericButtonBackground(painter, decorationButton);

    if (decorationButton->isChecked()) {
        // on all desktops
        painter->setPen(Qt::NoPen);
        painter->setBrush(foregroundColor(decorationButton));
        painter->translate(decorationButton->size().width() / 2.0, decorationButton->size().height() / 2.0);
        const int radius = decorationButton->size().width() / 2 - 3;
        painter->drawEllipse(-radius, -radius, radius * 2, radius * 2);
        painter->setBrush(decorationButton->isHovered() ? colorSettings(decorationButton).font(active) :  colorSettings(decorationButton).titleBarColor(active));
        painter->drawEllipse(-1, -1, 2, 2);
    } else {
        // not on all desktops
        // TODO: implement the pin
    }

    painter->restore();
}

void ImageProvider::renderShadeButton(QPainter *painter, KDecoration2::DecorationButton *decorationButton) const
{
    drawGenericButtonBackground(painter, decorationButton);
    painter->save();
    QPen pen(foregroundColor(decorationButton));
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

void ImageProvider::drawGenericButtonBackground(QPainter *painter, KDecoration2::DecorationButton *decorationButton) const
{
    if (!decorationButton->isPressed() && !decorationButton->isHovered()) {
        return;
    }
    const QColor baseBackgroundColor = colorSettings(decorationButton).font(decorationButton->decoration()->client()->isActive());
    drawBackground(painter, decorationButton, QColor(baseBackgroundColor.red(),
                                                     baseBackgroundColor.green(),
                                                     baseBackgroundColor.blue(),
                                                     decorationButton->isPressed() ? 50 : 127));
}

void ImageProvider::drawBackground(QPainter *painter, KDecoration2::DecorationButton *decorationButton, const QColor &color) const
{
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(decorationButton->isEnabled() ? color : QColor(color.red(), color.green(), color.blue(), color.alpha() * 0.6));
    painter->drawEllipse(QRect(QPoint(0, 0), decorationButton->size()));
    painter->restore();
}

void ImageProvider::drawDownArrow(QPainter *painter, KDecoration2::DecorationButton *decorationButton, const QPointF &offset) const
{
    painter->save();
    QPen pen(foregroundColor(decorationButton));
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

void ImageProvider::drawUpArrow(QPainter *painter, KDecoration2::DecorationButton *decorationButton, const QPointF &offset) const
{
    painter->save();
    QPen pen(foregroundColor(decorationButton));
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

QColor ImageProvider::foregroundColor(KDecoration2::DecorationButton *decorationButton) const
{
    const ColorSettings &colors = colorSettings(decorationButton->decoration()->client()->palette());
    const bool active = decorationButton->decoration()->client()->isActive();
    if (decorationButton->isHovered()) {
        return colors.titleBarColor(active);
    }
    QColor c = colors.font(active);
    if (!decorationButton->isEnabled()) {
        c.setAlphaF(c.alphaF() * 0.6);
    }
    return c;
}

Button::Button(KDecoration2::DecorationButtonType type, Decoration* decoration, QObject* parent)
    : DecorationButton(type, decoration, parent)
{
    const int height = decoration->captionHeight();
    setGeometry(QRect(0, 0, height, height));
    connect(decoration, &Decoration::bordersChanged, this, [this, decoration] {
        const int height = decoration->captionHeight();
        if (height == geometry().height()) {
            return;
        }
        ImageProvider::self()->clearCache(this);
        setGeometry(QRect(geometry().topLeft(), QSize(height, height)));
    });
}

Button *Button::create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
{
    if (Decoration *d = qobject_cast<Decoration*>(decoration)) {
        Button *b = new Button(type, d, parent);
        if (type == KDecoration2::DecorationButtonType::Menu) {
            QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::iconChanged, b, [b]() { b->update(); });
        }
        return b;
    }
    return nullptr;
}

Button::~Button() = default;

void Button::paint(QPainter *painter)
{
    if (type() == KDecoration2::DecorationButtonType::Menu) {
        const QPixmap pixmap = decoration()->client()->icon().pixmap(size());
        painter->drawPixmap(geometry().center() - QPoint(pixmap.width()/2, pixmap.height()/2), pixmap);
    } else {
        painter->drawImage(geometry().topLeft(), ImageProvider::self()->button(this));
    }
}

} // namespace
