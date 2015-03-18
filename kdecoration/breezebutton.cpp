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
#include "breezebutton.h"

#include <KDecoration2/DecoratedClient>
#include <KColorUtils>

#include <QPainter>

namespace Breeze
{

    //__________________________________________________________________
    Button::Button(KDecoration2::DecorationButtonType type, Decoration* decoration, QObject* parent)
        : DecorationButton(type, decoration, parent)
        , m_animation( new QPropertyAnimation( this ) )
    {

        // setup animation
        m_animation->setStartValue( 0 );
        m_animation->setEndValue( 1.0 );
        m_animation->setTargetObject( this );
        m_animation->setPropertyName( "opacity" );
        m_animation->setEasingCurve( QEasingCurve::InOutQuad );

        // setup default geometry
        const int height = decoration->buttonHeight();
        setGeometry(QRect(0, 0, height, height));

        // connect hover state changed
        connect( this, &KDecoration2::DecorationButton::hoveredChanged, this, &Button::updateAnimationState );

    }

    //__________________________________________________________________
    Button::Button(QObject *parent, const QVariantList &args)
        : DecorationButton(args.at(0).value<KDecoration2::DecorationButtonType>(), args.at(1).value<Decoration*>(), parent)
        , m_flag(FlagStandalone)
        , m_animation( new QPropertyAnimation( this ) )
    {}

    //__________________________________________________________________
    Button *Button::create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
    {
        if (auto d = qobject_cast<Decoration*>(decoration))
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

        if (!decoration()) return;

        painter->save();

        // translate from offset
        if( m_flag == FlagFirstInList ) painter->translate( m_offset );
        else painter->translate( 0, m_offset.y() );

        if (type() == KDecoration2::DecorationButtonType::Menu)
        {

            const QSizeF iconSize( size().width()-m_offset.x(), size().height()-m_offset.y() );
            const QRectF iconRect( geometry().topLeft(), iconSize );
            const QPixmap pixmap = decoration()->client().data()->icon().pixmap( iconSize.toSize());
            painter->drawPixmap(iconRect.center() - QPoint(pixmap.width()/2, pixmap.height()/2), pixmap);

        } else {

            drawIcon( painter );

        }

        painter->restore();

    }

    //__________________________________________________________________
    void Button::drawIcon( QPainter *painter ) const
    {

        painter->setRenderHints( QPainter::Antialiasing );

        /*
        scale painter so that its window matches QRect( -1, -1, 20, 20 )
        this makes all further rendering and scaling simpler
        all further rendering is preformed inside QRect( 0, 0, 18, 18 )
        */
        painter->translate( geometry().topLeft() );

        const qreal width( geometry().width() - m_offset.x() );
        painter->scale( width/20, width/20 );
        painter->translate( 1, 1 );

        // render background
        const QColor backgroundColor( this->backgroundColor() );
        if( backgroundColor.isValid() )
        {
            painter->setPen( Qt::NoPen );
            painter->setBrush( backgroundColor );
            painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
        }

        // render mark
        const QColor foregroundColor( this->foregroundColor() );
        if( foregroundColor.isValid() )
        {

            // setup painter
            QPen pen( foregroundColor );
            pen.setCapStyle( Qt::RoundCap );
            pen.setJoinStyle( Qt::MiterJoin );
            pen.setWidthF( 1.1*qMax(1.0, 20/width ) );

            painter->setPen( pen );
            painter->setBrush( Qt::NoBrush );

            switch( type() )
            {

                case KDecoration2::DecorationButtonType::Close:
                {
                    painter->drawLine( QPointF( 5, 5 ), QPointF( 13, 13 ) );
                    painter->drawLine( 13, 5, 5, 13 );
                    break;
                }

                case KDecoration2::DecorationButtonType::Maximize:
                {
                    if( isChecked() )
                    {
                        pen.setJoinStyle( Qt::RoundJoin );
                        painter->setPen( pen );
                        painter->drawPolygon( QPolygonF()
                            << QPointF( 4.5, 9 )
                            << QPointF( 9, 4.5 )
                            << QPointF( 13.5, 9 )
                            << QPointF( 9, 13.5 ) );
                    } else {
                        painter->drawPolyline( QPolygonF()
                            << QPointF( 4.5, 10.5 )
                            << QPointF( 9, 6.5 )
                            << QPointF( 13.5, 10.5 ) );
                    }
                    break;
                }

                case KDecoration2::DecorationButtonType::Minimize:
                {
                    painter->drawPolyline( QPolygonF()
                        << QPointF( 4.5, 7.5 )
                        << QPointF( 9, 11.5 )
                        << QPointF( 13.5, 7.5 ) );
                    break;
                }

                case KDecoration2::DecorationButtonType::OnAllDesktops:
                {
                    painter->setPen( Qt::NoPen );
                    painter->setBrush( foregroundColor );

                    if( isChecked())
                    {

                        // outer ring
                        painter->drawEllipse( QRectF( 3, 3, 12, 12 ) );

                        // center dot
                        QColor backgroundColor( this->backgroundColor() );
                        auto d = qobject_cast<Decoration*>( decoration() );
                        if( !backgroundColor.isValid() && d ) backgroundColor = d->titleBarColor();

                        if( backgroundColor.isValid() )
                        {
                            painter->setBrush( backgroundColor );
                            painter->drawEllipse( QRectF( 8, 8, 2, 2 ) );
                        }

                    } else {

                        painter->drawPolygon( QPolygonF()
                            << QPointF( 6.5, 8.5 )
                            << QPointF( 12, 3 )
                            << QPointF( 15, 6 )
                            << QPointF( 9.5, 11.5 ) );

                        painter->setPen( pen );
                        painter->drawLine( QPointF( 5, 7 ), QPointF( 11, 13 ) );
                        painter->drawLine( QPointF( 12, 6 ), QPointF( 4.5, 13.5 ) );
                    }
                    break;
                }

                case KDecoration2::DecorationButtonType::Shade:
                {

                    if (isChecked())
                    {

                        painter->drawLine( 4, 6.5, 14, 6.5 );
                        painter->drawPolyline( QPolygonF()
                            << QPointF( 4.5, 9.5 )
                            << QPointF( 9, 13.5 )
                            << QPointF( 13.5, 9.5 ) );

                    } else {

                        painter->drawLine( 4, 6.5, 14, 6.5 );
                        painter->drawPolyline( QPolygonF()
                            << QPointF( 4.5, 13.5 )
                            << QPointF( 9, 9.5 )
                            << QPointF( 13.5, 13.5 ) );
                    }

                    break;

                }

                case KDecoration2::DecorationButtonType::KeepBelow:
                {

                    painter->drawPolyline( QPolygonF()
                        << QPointF( 4.5, 5.5 )
                        << QPointF( 9, 9.5 )
                        << QPointF( 13.5, 5.5 ) );

                    painter->drawPolyline( QPolygonF()
                        << QPointF( 4.5, 9.5 )
                        << QPointF( 9, 13.5 )
                        << QPointF( 13.5, 9.5 ) );
                    break;

                }

                case KDecoration2::DecorationButtonType::KeepAbove:
                {
                    painter->drawPolyline( QPolygonF()
                        << QPointF( 4.5, 8.5 )
                        << QPointF( 9, 4.5 )
                        << QPointF( 13.5, 8.5 ) );

                    painter->drawPolyline( QPolygonF()
                        << QPointF( 4.5, 12.5 )
                        << QPointF( 9, 8.5 )
                        << QPointF( 13.5, 12.5 ) );
                    break;
                }

                case KDecoration2::DecorationButtonType::ApplicationMenu:
                {
                    painter->drawLine( QPointF( 3.5, 5 ), QPointF( 14.5, 5 ) );
                    painter->drawLine( QPointF( 3.5, 9 ), QPointF( 14.5, 9 ) );
                    painter->drawLine( QPointF( 3.5, 13 ), QPointF( 14.5, 13 ) );
                    break;
                }

                case KDecoration2::DecorationButtonType::ContextHelp:
                {
                    QPainterPath path;
                    path.moveTo( 5, 6 );
                    path.arcTo( QRectF( 5, 3.5, 8, 5 ), 180, -180 );
                    path.cubicTo( QPointF(12.5, 9.5), QPointF( 9, 7.5 ), QPointF( 9, 11.5 ) );
                    painter->drawPath( path );

                    painter->drawPoint( 9, 15 );

                    break;
                }

                default: break;

            }

        }

    }

    //__________________________________________________________________
    QColor Button::foregroundColor( void ) const
    {

        auto d = qobject_cast<Decoration*>( decoration() );
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
        auto d = qobject_cast<Decoration*>( decoration() );
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

        auto d = qobject_cast<Decoration*>(decoration());
        if( !(d && d->internalSettings()->animationsEnabled() ) ) return;

        m_animation->setDirection( hovered ? QPropertyAnimation::Forward : QPropertyAnimation::Backward );
        if( m_animation->state() != QPropertyAnimation::Running ) m_animation->start();

    }

} // namespace
