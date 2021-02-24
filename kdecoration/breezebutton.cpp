/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2020 Paul McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "breezebutton.h"
#include "breezerenderdecorationbuttonicon.h"

#include <KDecoration2/DecoratedClient>
#include <KColorUtils>
#include <KIconLoader>

#include <QPainter>
#include <QVariantAnimation>
#include <QPainterPath>

namespace Breeze
{

    using KDecoration2::ColorRole;
    using KDecoration2::ColorGroup;
    using KDecoration2::DecorationButtonType;


    //__________________________________________________________________
    Button::Button(DecorationButtonType type, Decoration* decoration, QObject* parent)
        : DecorationButton(type, decoration, parent)
        , m_animation( new QVariantAnimation( this ) )
    {

        // setup animation
        // It is important start and end value are of the same type, hence 0.0 and not just 0
        m_animation->setStartValue( 0.0 );
        m_animation->setEndValue( 1.0 );
        m_animation->setEasingCurve( QEasingCurve::InOutQuad );
        connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setOpacity(value.toReal());
        });

        // setup default geometry
        const int height = decoration->buttonHeight();
        setGeometry(QRect(0, 0, height, height));
        setIconSize(QSize( height, height ));

        // connections
        connect(decoration->client().data(), SIGNAL(iconChanged(QIcon)), this, SLOT(update()));
        connect(decoration->settings().data(), &KDecoration2::DecorationSettings::reconfigured, this, &Button::reconfigure);
        connect( this, &KDecoration2::DecorationButton::hoveredChanged, this, &Button::updateAnimationState );

        reconfigure();

    }

    //__________________________________________________________________
    Button::Button(QObject *parent, const QVariantList &args)
        : Button(args.at(0).value<DecorationButtonType>(), args.at(1).value<Decoration*>(), parent)
    {
        m_flag = FlagStandalone;
        //! icon size must return to !valid because it was altered from the default constructor,
        //! in Standalone mode the button is not using the decoration metrics but its geometry
        m_iconSize = QSize(-1, -1);
    }
            
    //__________________________________________________________________
    Button *Button::create(DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
    {
        if (auto d = qobject_cast<Decoration*>(decoration))
        {
            Button *b = new Button(type, d, parent);
            switch( type )
            {

                case DecorationButtonType::Close:
                b->setVisible( d->client().data()->isCloseable() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::closeableChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::Maximize:
                b->setVisible( d->client().data()->isMaximizeable() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::maximizeableChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::Minimize:
                b->setVisible( d->client().data()->isMinimizeable() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::minimizeableChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::ContextHelp:
                b->setVisible( d->client().data()->providesContextHelp() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::providesContextHelpChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::Shade:
                b->setVisible( d->client().data()->isShadeable() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::shadeableChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::Menu:
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::iconChanged, b, [b]() { b->update(); });
                break;

                default: break;

            }

            return b;
        }

        return nullptr;

    }

    //__________________________________________________________________
    void Button::paint(QPainter *painter, const QRect &repaintRegion)
    {
        Q_UNUSED(repaintRegion)

        if (!decoration()) return;

        painter->save();

        // translate from offset
        if( m_flag == FlagFirstInList ) painter->translate( m_offset );
        else painter->translate( 0, m_offset.y() );

        if( !m_iconSize.isValid() ) m_iconSize = geometry().size().toSize();

        // menu button
        if (type() == DecorationButtonType::Menu)
        {

            const QRectF iconRect( geometry().topLeft(), m_iconSize );
            if (auto deco =  qobject_cast<Decoration*>(decoration())) {
                const QPalette activePalette = KIconLoader::global()->customPalette();
                QPalette palette = decoration()->client().data()->palette();
                palette.setColor(QPalette::Foreground, deco->fontColor());
                KIconLoader::global()->setCustomPalette(palette);
                decoration()->client().data()->icon().paint(painter, iconRect.toRect());
                if (activePalette == QPalette()) {
                    KIconLoader::global()->resetPalette();
                }    else {
                    KIconLoader::global()->setCustomPalette(palette);
                }
            } else {
                decoration()->client().data()->icon().paint(painter, iconRect.toRect());
            }

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

        const qreal width( m_iconSize.width() );
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
            pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

            auto d = qobject_cast<Decoration*>( decoration() );

            std::unique_ptr<RenderDecorationButtonIcon18By18> iconRenderer;
            if (d) iconRenderer = RenderDecorationButtonIcon18By18::factory( painter, pen, d->internalSettings()->buttonIconStyle(), false );
            else iconRenderer = RenderDecorationButtonIcon18By18::factory( painter, pen );

            switch( type() )
            {

                case DecorationButtonType::Close:
                {
                    iconRenderer->renderCloseIcon();
                    break;
                }

                case DecorationButtonType::Maximize:
                {
                    if( isChecked() ) iconRenderer->renderRestoreIcon();
                    else iconRenderer->renderMaximizeIcon();
                    break;
                }

                case DecorationButtonType::Minimize:
                {
                    iconRenderer->renderMinimizeIcon();
                    break;
                }

                case DecorationButtonType::OnAllDesktops:
                {
                    if( isChecked()) iconRenderer->renderPinnedOnAllDesktopsIcon();
                    else iconRenderer->renderPinOnAllDesktopsIcon();
                    break;
                }

                case DecorationButtonType::Shade:
                {

                    if (isChecked()) iconRenderer->renderUnShadeIcon();
                    else iconRenderer->renderShadeIcon();
                    break;

                }

                case DecorationButtonType::KeepBelow:
                {

                    iconRenderer->renderKeepBehindIcon();
                    break;

                }

                case DecorationButtonType::KeepAbove:
                {
                    iconRenderer->renderKeepInFrontIcon();
                    break;
                }


                case DecorationButtonType::ApplicationMenu:
                {
                    iconRenderer->renderApplicationMenuIcon();
                    break;
                }

                case DecorationButtonType::ContextHelp:
                {
                   iconRenderer->renderContextHelpIcon();
                    break;
                }

                default: break;

            }

        }

    }

    //__________________________________________________________________
    QColor Button::foregroundColor() const
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        if( !d ) {

            return QColor();

        } else if( isPressed() ) {

            return d->titleBarColor();

        } else if( type() == DecorationButtonType::Close && d->internalSettings()->outlineCloseButton() ) {

            return d->titleBarColor();

        } else if( ( type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade ) && isChecked() ) {

            return d->titleBarColor();

        } else if( m_animation->state() == QAbstractAnimation::Running ) {

            return KColorUtils::mix( d->fontColor(), d->titleBarColor(), m_opacity );

        } else if( isHovered() ) {

            return d->titleBarColor();

        } else {

            return d->fontColor();

        }

    }

    //__________________________________________________________________
    QColor Button::backgroundColor() const
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        if( !d ) {

            return QColor();

        }

        auto c = d->client().data();
        QColor redColor( c->color( ColorGroup::Warning, ColorRole::Foreground ) );

        if( isPressed() ) {

            if( type() == DecorationButtonType::Close ) return redColor;
            else return KColorUtils::mix( d->titleBarColor(), d->fontColor(), 0.3 );

        } else if( ( type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade ) && isChecked() ) {

            return d->fontColor();

        } else if( m_animation->state() == QAbstractAnimation::Running ) {

            if( type() == DecorationButtonType::Close )
            {
                if( d->internalSettings()->outlineCloseButton() )
                {

                    return KColorUtils::mix( d->fontColor(), c->color( ColorGroup::Warning, ColorRole::Foreground ).lighter(), m_opacity );

                } else {

                    QColor color( redColor.lighter() );
                    color.setAlpha( color.alpha()*m_opacity );
                    return color;

                }

            } else {

                QColor color( d->fontColor() );
                color.setAlpha( color.alpha()*m_opacity );
                return color;

            }

        } else if( isHovered() ) {

            if( type() == DecorationButtonType::Close ) return redColor.lighter();
            else return d->fontColor();

        } else if( type() == DecorationButtonType::Close && d->internalSettings()->outlineCloseButton() ) {

            return d->fontColor();

        } else {

            return QColor();

        }

    }

    //________________________________________________________________
    void Button::reconfigure()
    {

        // animation
        auto d = qobject_cast<Decoration*>(decoration());
        if( d )  m_animation->setDuration( d->animationsDuration() );

    }

    //__________________________________________________________________
    void Button::updateAnimationState( bool hovered )
    {

        auto d = qobject_cast<Decoration*>(decoration());
        if( !(d && d->animationsDuration() > 0 ) ) return;

        m_animation->setDirection( hovered ? QAbstractAnimation::Forward : QAbstractAnimation::Backward );
        if( m_animation->state() != QAbstractAnimation::Running ) m_animation->start();

    }

} // namespace
