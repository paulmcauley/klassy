/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2021 Paul McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "breezebutton.h"
#include "renderdecorationbuttonicon.h"

#include <KDecoration2/DecoratedClient>
#include <KColorUtils>
#include <KIconLoader>
#include <KColorScheme>

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
        auto d = qobject_cast<Decoration*>( decoration() );

        m_backgroundColor = this->backgroundColor();
        m_foregroundColor = this->foregroundColor();
        
        m_lowContrastBetweenTitleBarAndBackground = false;
        if( d->internalSettings()->inheritSystemHighlightColors() && (KColorUtils::contrastRatio(m_backgroundColor, d->titleBarColor()) < 1.4) )
            m_lowContrastBetweenTitleBarAndBackground = true;
        
        painter->save();
        
        if( !m_iconSize.isValid() ) m_iconSize = geometry().size().toSize();
        
        // menu button
        if (type() == DecorationButtonType::Menu)
        {
            //draw a background only with square highlight style; 
            //NB: paintSquareBackground function applies a translation to painter as different square button geometry
            if( d->internalSettings()->buttonHighlightStyle() == InternalSettings::EnumButtonHighlightStyle::HighlightSquare ) 
                paintSquareBackground(painter);
            
            // translate from offset -- this is the main offset that applies to all button geometries to space each button correctly
            if( m_flag == FlagFirstInList ) painter->translate( m_offset );
            else painter->translate( 0, m_offset.y() );
            
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

        auto d = qobject_cast<Decoration*>( decoration() );
                
        //draw a background only with square highlight style; 
        //NB: paintSquareBackground function applies a translation to painter as different larger square button geometry
        if( d->internalSettings()->buttonHighlightStyle() == InternalSettings::EnumButtonHighlightStyle::HighlightSquare ) 
            paintSquareBackground(painter);
        
        // translate from offset
        if( m_flag == FlagFirstInList ) painter->translate( m_offset );
        else painter->translate( 0, m_offset.y() );
        
        /*
        scale painter so that its window matches QRect( -1, -1, 20, 20 )
        this makes all further rendering and scaling simpler
        all further rendering is preformed inside QRect( 0, 0, 18, 18 )
        */
        painter->translate( geometry().topLeft() );

        const qreal width( m_iconSize.width() );
        painter->scale( width/20, width/20 );
        painter->translate( 1, 1 );
        painter->setRenderHints( QPainter::Antialiasing );
        
        
        // render background if Circle button highlight style
        if((d->internalSettings()->buttonHighlightStyle() != InternalSettings::EnumButtonHighlightStyle::HighlightSquare ) && m_backgroundColor.isValid() )
            paintCircleBackground(painter);
        
        
        // render mark
        if( m_foregroundColor.isValid() )
        {

            // setup painter
            QPen pen( m_foregroundColor );
            pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
            painter->setPen(pen);


            std::unique_ptr<RenderDecorationButtonIcon18By18> iconRenderer;
            if (d) { 
                
                iconRenderer = RenderDecorationButtonIcon18By18::factory( d->internalSettings(), painter, false );

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

    }

    //__________________________________________________________________
    QColor Button::foregroundColor() const
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        if( !d ) return QColor();
        auto c = d->client().toStrongRef().data();
        
        QColor higherContrastFontColor = getHigherContrastForegroundColor( d->fontColor(), m_backgroundColor, 2.3 );
        
        if( isPressed() ) {
            if( type() == DecorationButtonType::Close ) return Qt::GlobalColor::white;
            else if( d->internalSettings()->inheritSystemHighlightColors() ) {
                return higherContrastFontColor;
            }
            else return d->titleBarColor();
            
        } else if( ( type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade ) && isChecked() ) {
            if( d->internalSettings()->inheritSystemHighlightColors() ) return higherContrastFontColor;
            else return d->titleBarColor();
        }else if( type() == DecorationButtonType::OnAllDesktops && isChecked() && d->internalSettings()->inheritSystemHighlightColors() ){
            return higherContrastFontColor;
        } else if( m_animation->state() == QAbstractAnimation::Running ) {
            if( type() == DecorationButtonType::Close ){
                if( d->internalSettings()->outlineCloseButton() ){
                    return KColorUtils::mix (d->titleBarColor(), Qt::GlobalColor::white, m_opacity);
                } else {
                    return KColorUtils::mix (d->fontColor(), Qt::GlobalColor::white, m_opacity);
                }
            }
            else if( d->internalSettings()->inheritSystemHighlightColors() ) return KColorUtils::mix (d->fontColor(), higherContrastFontColor, m_opacity);
            else return KColorUtils::mix( d->fontColor(), d->titleBarColor(), m_opacity );
            
        } else if( isHovered() ) {
            if( type() == DecorationButtonType::Close ) return Qt::GlobalColor::white;
            else if( d->internalSettings()->inheritSystemHighlightColors()) return higherContrastFontColor;
            else return d->titleBarColor();    
        } else if( type() == DecorationButtonType::Close && d->internalSettings()->outlineCloseButton() ) {
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

        auto c = d->client().toStrongRef().data();
        QColor redColor( c->color( ColorGroup::Warning, ColorRole::Foreground ) );
        int redColorHsv[3];
        redColor.getHsv(&redColorHsv[0], &redColorHsv[1], &redColorHsv[2]);
        redColorHsv[1] = 255; //increase saturation to max
        QColor redColorSaturated;
        redColorSaturated.setHsv(redColorHsv[0], redColorHsv[1], redColorHsv[2]);
        
        QColor buttonHoverColor;
        QColor buttonFocusColor;
        
        //set hover and focus colours
        if( d->internalSettings()->inheritSystemHighlightColors() ){
            if( type() == DecorationButtonType::Close ) { 
                buttonHoverColor = redColor; 
                buttonFocusColor = redColorSaturated;
            }
            else {
                KStatefulBrush buttonFocusStatefulBrush = KStatefulBrush( KColorScheme::Button, KColorScheme::FocusColor );
                KStatefulBrush buttonHoverStatefulBrush = KStatefulBrush( KColorScheme::Button, KColorScheme::HoverColor );
                buttonFocusColor = buttonFocusStatefulBrush.brush( c->palette() ).color();
                buttonHoverColor = buttonHoverStatefulBrush.brush( c->palette() ).color();
            }
        } else {
            if( type() == DecorationButtonType::Close ) { 
                buttonHoverColor = redColor;
                buttonFocusColor = redColorSaturated;
            } else {
                buttonFocusColor = KColorUtils::mix( d->titleBarColor(), d->fontColor(), 0.3 );
                buttonHoverColor = d->fontColor();
            }
        }

        if( isPressed() ) {
            return buttonFocusColor;

        } else if( ( type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade ) && isChecked() ) {
            if( d->internalSettings()->inheritSystemHighlightColors() ) return buttonFocusColor;
            else return buttonHoverColor;
        
        }else if( type() == DecorationButtonType::OnAllDesktops && isChecked() && d->internalSettings()->inheritSystemHighlightColors() ){
            return buttonFocusColor;
        }else if( m_animation->state() == QAbstractAnimation::Running ) {

            if( type() == DecorationButtonType::Close )
            {
                if( d->internalSettings()->outlineCloseButton() )
                {
                    if ( d->internalSettings()->redOutline() && !c->isActive() ){
                        return KColorUtils::mix( d->fontColor(), redColor, m_opacity );
                    } else if( d->internalSettings()->redOutline() && c->isActive() ) {
                        return redColor; //non-hovered and hovered are both same red -- no animation in background, just in foreground
                    } else {
                        return KColorUtils::mix( d->fontColor(), redColor, m_opacity );
                    }
                } else {
                    QColor color( redColor );
                    color.setAlpha( color.alpha()*m_opacity );
                    return color;
                }

            } else {

                QColor color( buttonHoverColor );
                color.setAlpha( color.alpha()*m_opacity );
                return color;

            }

        } else if( isHovered() ) {
            return buttonHoverColor;

        } else if( type() == DecorationButtonType::Close && d->internalSettings()->outlineCloseButton() ) {
            if( d->internalSettings()->redOutline() ) return c->isActive() ? redColor : d->fontColor();
            else return d->fontColor();

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
    
    bool Button::shouldDrawBackgroundStroke() const
    {
        auto d = qobject_cast<Decoration*>(decoration());
        if(!d) return false;
        
        return ( m_lowContrastBetweenTitleBarAndBackground );
    }
    
    QColor Button::getHigherContrastForegroundColor( const QColor& foregroundColor, const QColor& backgroundColor, double blackWhiteContrastThreshold ) const
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        
        double contrastRatio = KColorUtils::contrastRatio(foregroundColor, backgroundColor);
        /*
        qDebug() << "Button type" << static_cast<int>(type()) ;
        qDebug() << "Contrast ratio: " << contrastRatio;
        */
        if( contrastRatio < blackWhiteContrastThreshold ) return getBlackOrWhiteForegroundForHighContrast(backgroundColor);
        else return foregroundColor;
    }
    
    QColor Button::getBlackOrWhiteForegroundForHighContrast( const QColor& backgroundColor ) const
    {
        // based on http://www.w3.org/TR/AERT#color-contrast
        
        if ( !backgroundColor.isValid() ) return QColor();
        
        int rgbBackground[3];
        
        backgroundColor.getRgb(&rgbBackground[0], &rgbBackground[1], &rgbBackground[2]);
        
        double brightness = qRound(static_cast<double>(( (rgbBackground[0] * 299) + (rgbBackground[1] *587) + (rgbBackground[2] * 114) ) /1000));
        
        return (brightness > 125) ? QColor(Qt::GlobalColor::black) : QColor(Qt::GlobalColor::white);
    }
    
    
    void Button::paintSquareBackground(QPainter* painter) const
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        
        if( m_backgroundColor.isValid() )
        {            
            painter->save();
            painter->setRenderHints( QPainter::Antialiasing );
            painter->setBrush( m_backgroundColor );
            
            if( shouldDrawBackgroundStroke() )
            {   
                QColor strokeColor = d->fontColor();
                if( m_animation->state() == QAbstractAnimation::Running ) {
                    strokeColor.setAlpha( strokeColor.alpha()*m_opacity );
                }
                QPen pen( strokeColor );
                pen.setWidthF( PenWidth::Symbol );
                painter->setPen(pen);
                
                //the size of the stroke rectangle needs to be smaller than the button geometry to prevent drawing a line outside the button
                QRectF strokeRect = QRectF( geometry().left() + pen.width()+0.5, geometry().top() + pen.width(), geometry().width() - pen.width()*2-1, geometry().height() - pen.width()*2);
                painter->drawRect( strokeRect );
            } else {
                painter->setPen( Qt::NoPen );
                painter->drawRect( geometry() );
            }
            
            painter->restore();
        }
        
        
        //NB: if square highlight, must add a translation due to larger geometry
        painter->translate( m_squareHighlightIconHorizontalTranslation, m_squareHighlightIconVerticalTranslation );
    }
    
    void Button::paintCircleBackground(QPainter* painter) const
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        
        painter->save();
        if( shouldDrawBackgroundStroke() )
        {   
            QColor strokeColor = d->fontColor();
            if( m_animation->state() == QAbstractAnimation::Running ) {
                strokeColor.setAlpha( strokeColor.alpha()*m_opacity );
            }
            QPen pen( strokeColor );
            pen.setWidthF( PenWidth::Symbol );
            painter->setPen(pen);
        } else painter->setPen( Qt::NoPen );;
        painter->setBrush( m_backgroundColor );
        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
        painter->restore();
    }
    
    
} // namespace
