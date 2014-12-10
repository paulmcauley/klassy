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
#include <KColorUtils>

#include <QPainter>

namespace Breeze
{

    //__________________________________________________________________
    Button::Button(KDecoration2::DecorationButtonType type, Decoration* decoration, QObject* parent)
        : DecorationButton(type, decoration, parent)
        ,m_animation( new QPropertyAnimation( this ) )
        ,m_opacity(0)
    {

        // setup animation
        m_animation->setStartValue( 0 );
        m_animation->setEndValue( 1.0 );
        m_animation->setTargetObject( this );
        m_animation->setPropertyName( "opacity" );
        m_animation->setEasingCurve( QEasingCurve::InOutQuad );

        // setup geometry
        const int height = decoration->buttonHeight();
        setGeometry(QRect(0, 0, height, height));
        connect(decoration, &Decoration::bordersChanged, this, [this, decoration]
        {
            const int height = decoration->buttonHeight();
            if (height == geometry().height()) return;
            ImageProvider::self()->clearCache(this);
            setGeometry(QRectF(geometry().topLeft(), QSizeF(height, height)));
        });

        // connect hover state changed
        connect( this, &KDecoration2::DecorationButton::hoveredChanged, this, &Button::updateAnimationState );

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

        // TODO: optimize based on repaintRegion
        if (type() == KDecoration2::DecorationButtonType::Menu)
        {
            const QPixmap pixmap = decoration()->client().data()->icon().pixmap(size().toSize());
            painter->drawPixmap(geometry().center() - QPoint(pixmap.width()/2, pixmap.height()/2), pixmap);

        } else {

            painter->save();
            painter->setRenderHints( QPainter::Antialiasing );
            painter->translate( geometry().topLeft() );
            ImageProvider::self()->renderButton( painter, this );
            painter->restore();

        }

    }

    //__________________________________________________________________
    QColor Button::foregroundColor( void ) const
    {

        Decoration *d = qobject_cast<Decoration*>( decoration() );
        if( !d ) return QColor();
        if( type() == KDecoration2::DecorationButtonType::Close ) {

            return d->titleBarColor();

        } else if( ( type() == KDecoration2::DecorationButtonType::KeepBelow || type() == KDecoration2::DecorationButtonType::KeepAbove ) && isChecked() ) {

            return d->titleBarColor();

        } else if( m_animation->state() == QPropertyAnimation::Running ) {

            return KColorUtils::mix( d->fontColor(), d->titleBarColor(), m_opacity );

        } else if( isHovered() ) {

            return d->titleBarColor();

        } else {

            return d->fontColor();

        }

    }

    //__________________________________________________________________
    QColor Button::backgroundColor( void ) const
    {
        Decoration *d = qobject_cast<Decoration*>( decoration() );
        if( !d ) return QColor();
        if( isPressed() )
        {

            if( type() == KDecoration2::DecorationButtonType::Close ) return d->colorSettings().closeButtonColor();
            else return KColorUtils::mix( d->titleBarColor(), d->fontColor(), 0.3 );

        } else if( ( type() == KDecoration2::DecorationButtonType::KeepBelow || type() == KDecoration2::DecorationButtonType::KeepAbove ) && isChecked() ) {

            return d->fontColor();

        } else if( m_animation->state() == QPropertyAnimation::Running ) {

            if( type() == KDecoration2::DecorationButtonType::Close )
            {

                return KColorUtils::mix( d->fontColor(), d->colorSettings().closeButtonColor().lighter(), m_opacity );

            } else {

                QColor color( d->fontColor() );
                color.setAlpha( color.alpha()*m_opacity );
                return color;

            }

        } else if( isHovered() ) {

            if( type() == KDecoration2::DecorationButtonType::Close ) return d->colorSettings().closeButtonColor().lighter();
            else return d->fontColor();

        } else if( type() == KDecoration2::DecorationButtonType::Close ) {

            return d->fontColor();

        } else {

            return QColor();

        }

    }

    //__________________________________________________________________
    void Button::updateAnimationState( bool hovered )
    {

        Decoration *d = qobject_cast<Decoration*>(decoration());
        if( !(d && d->internalSettings().animationsEnabled() ) ) return;

        m_animation->setDirection( hovered ? QPropertyAnimation::Forward : QPropertyAnimation::Backward );
        if( m_animation->state() != QPropertyAnimation::Running ) m_animation->start();

   }

} // namespace
