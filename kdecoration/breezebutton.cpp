/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2021 Paul McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "breezebutton.h"
#include "renderdecorationbuttonicon.h"
#include "colortools.h"

#include <KDecoration2/DecoratedClient>
#include <KColorUtils>
#include <KIconLoader>
#include <KColorScheme>
#include <KWindowSystem>

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
        , m_isGtkCsdButton( false )
    {

        // setup animation
        // It is important start and end value are of the same type, hence 0.0 and not just 0
        m_animation->setStartValue( 0.0 );
        m_animation->setEndValue( 1.0 );
        m_animation->setEasingCurve( QEasingCurve::InOutQuad );
        connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setOpacity(value.toReal());
        });
        
        //detect the kde-gtk-config-daemon
        if (QCoreApplication::applicationName() == QStringLiteral("kded5")) {
            // From Chris Holland https://github.com/Zren/material-decoration/
            // kde-gtk-config has a kded5 module which renders the buttons to svgs for gtk.
            m_isGtkCsdButton = true;
        }

        // setup default geometry
        const int iconHeight = decoration->iconHeight();
        setGeometry(QRect(0, 0, iconHeight, iconHeight));
        setIconSize(QSize( iconHeight, iconHeight ));

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

        setDevicePixelRatio(painter);
        setShouldDrawBoldButtonIcons();
        setStandardScaledPenWidth();
        
        m_backgroundColor = this->backgroundColor();
        m_foregroundColor = this->foregroundColor();
        
        m_lowContrastBetweenTitleBarAndBackground = ( d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText && (KColorUtils::contrastRatio(m_backgroundColor, d->titleBarColor()) < 1.3) );
        
        m_outlineColor = this->outlineColor();
        
        painter->save();
        
        if( !m_iconSize.isValid() || isStandAlone() ) m_iconSize = geometry().size().toSize();
        
        // menu button
        if (type() == DecorationButtonType::Menu)
        {
            //draw a background only with Full-sized or Large background shapes; 
            //for standalone/GTK we draw small buttons so can't draw menu
            if( d->buttonSize() == ButtonSize::FullSized && !( isStandAlone() || m_isGtkCsdButton ) ) 
                paintFullSizedButtonBackground(painter);
            //for standalone/GTK we draw small buttons so can't draw menu
            else if( d->buttonSize() == ButtonSize::Large  && !( isStandAlone() || m_isGtkCsdButton )) 
                paintLargeSizedButtonBackground(painter);
            
            // translate from icon offset
            painter->translate( m_iconOffset );
            
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
                
        
        //for standalone/GTK we draw small buttons so don't do anything
        if(  !( isStandAlone() || m_isGtkCsdButton ) ){
        
            //draw a background only with Full-sized Rectangle button shape; 
            //NB: paintFullSizedRectangleBackground function applies a translation to painter as different larger full-sized button geometry
            if( d->buttonSize() == ButtonSize::FullSized ) paintFullSizedButtonBackground(painter);
            if( d->buttonSize() == ButtonSize::Large ) paintLargeSizedButtonBackground(painter);
        }
        
        // translate from icon offset
        painter->translate( m_iconOffset );       
        
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
        
        
        // render background inside scaled area if small sized button shape, or if standAlone
        if( d->buttonSize() == ButtonSize::Small || isStandAlone() || m_isGtkCsdButton )
            paintSmallSizedButtonBackground(painter);
        
        // render mark
        if( m_foregroundColor.isValid() )
        {

            // setup painter
            QPen pen( m_foregroundColor );
            
            //this method commented out is for original non-cosmetic pen painting method (gives blurry icons at larger sizes )
            //pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
            
            //cannot use a scaled cosmetic pen if GTK CSD as kde-gtk-config generates svg icons
            if( m_isGtkCsdButton ) {
                pen.setWidthF( PenWidth::Symbol );
            }
            else {
                pen.setWidthF( m_standardScaledPenWidth );
                pen.setCosmetic(true);
            }
            painter->setPen(pen);


            std::unique_ptr<RenderDecorationButtonIcon18By18> iconRenderer;
            if (d) { 
                
                iconRenderer = RenderDecorationButtonIcon18By18::factory( d->internalSettings(), painter, false, m_boldButtonIcons );

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
        
        QColor higherContrastFontColor = ColorTools::getHigherContrastForegroundColor( d->fontColor(), m_backgroundColor, 2.3 );
        
        if( isPressed() ) {
            if( type() == DecorationButtonType::Close ) return Qt::GlobalColor::white;
            else if( d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText ) {
                return higherContrastFontColor;
            }
            else return d->titleBarColor();
            
        } else if( ( type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade ) && isChecked() ) {
            if( d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText ) return higherContrastFontColor;
            else return d->titleBarColor();
        }else if( type() == DecorationButtonType::OnAllDesktops && isChecked() && d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText ){
            return higherContrastFontColor;
        } else if( m_animation->state() == QAbstractAnimation::Running ) {
            if( type() == DecorationButtonType::Close ){
                if( d->internalSettings()->outlineCloseButton() ){
                    return KColorUtils::mix (d->titleBarColor(), Qt::GlobalColor::white, m_opacity);
                } else {
                    return KColorUtils::mix (d->fontColor(), Qt::GlobalColor::white, m_opacity);
                }
            }
            else if( d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText ) return KColorUtils::mix (d->fontColor(), higherContrastFontColor, m_opacity);
            else return KColorUtils::mix( d->fontColor(), d->titleBarColor(), m_opacity );
            
        } else if( isHovered() ) {
            if( type() == DecorationButtonType::Close ) return Qt::GlobalColor::white;
            else if( d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText ) return higherContrastFontColor;
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

        auto c = d->client().toStrongRef();
        Q_ASSERT(c);
        
        QColor buttonHoverColor;
        QColor buttonFocusColor;
        
        //set hover and focus colours
        if( d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccent ||  d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights ){
            if( type() == DecorationButtonType::Close ) { 
                buttonFocusColor = d->systemAccentColors()->negativeSaturated;
                buttonHoverColor = d->systemAccentColors()->negative;
            } else if ( type() == DecorationButtonType::Minimize && d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights ){
                buttonFocusColor = d->systemAccentColors()->neutral;
                buttonHoverColor = d->systemAccentColors()->neutralLessSaturated;
            } else if ( type() == DecorationButtonType::Maximize && d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights ){
                buttonFocusColor = d->systemAccentColors()->positive;
                buttonHoverColor = d->systemAccentColors()->positiveLessSaturated;
            } else {
                buttonFocusColor = d->systemAccentColors()->buttonFocus;
                buttonHoverColor = d->systemAccentColors()->buttonHover;
            }
        } else {
            if( type() == DecorationButtonType::Close ) { 
                buttonHoverColor = d->systemAccentColors()->negative;
                buttonFocusColor = d->systemAccentColors()->negativeSaturated;
            } else {
                buttonFocusColor = KColorUtils::mix( d->titleBarColor(), d->fontColor(), 0.3 );
                buttonHoverColor = d->fontColor();
            }
        }

        if( isPressed() ) {
            return buttonFocusColor;

        } else if( ( type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade ) && isChecked() ) {
            if( d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccent ||  d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights ) return buttonFocusColor;
            else return buttonHoverColor;
        
        }else if( type() == DecorationButtonType::OnAllDesktops && isChecked() && (d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccent ||  d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights) ){
            return buttonFocusColor;
        }else if( m_animation->state() == QAbstractAnimation::Running ) {

            if( type() == DecorationButtonType::Close )
            {
                if( d->internalSettings()->outlineCloseButton() )
                {
                    if ( d->internalSettings()->redOutline() && !c->isActive() ){
                        return KColorUtils::mix( d->fontColor(), buttonHoverColor, m_opacity );
                    } else if( d->internalSettings()->redOutline() && c->isActive() ) {
                        return buttonHoverColor; //non-hovered and hovered are both same red -- no animation in background, just in foreground
                    } else {
                        return KColorUtils::mix( d->fontColor(), buttonHoverColor, m_opacity );
                    }
                } else {
                    QColor color( buttonHoverColor );
                    color.setAlphaF( color.alphaF()*m_opacity );
                    return color;
                }

            } else {

                QColor color( buttonHoverColor );
                color.setAlphaF( color.alphaF()*m_opacity );
                return color;

            }

        } else if( isHovered() ) {
            return buttonHoverColor;

        } else if( type() == DecorationButtonType::Close && d->internalSettings()->outlineCloseButton() ) {
            if( d->internalSettings()->redOutline() ) return c->isActive() ? buttonHoverColor : d->fontColor();
            else return d->fontColor();

        } else {

            return QColor();
        }

    }
    
        //__________________________________________________________________
    QColor Button::outlineColor() const
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        if( !d ) return QColor();
        
        auto c = d->client().toStrongRef();
        Q_ASSERT(c);
        
        QColor buttonFocusColor;
        //set hover and focus colours
        if( d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccent ||  d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights ){
            if( type() == DecorationButtonType::Close ) { 
                buttonFocusColor = d->systemAccentColors()->negativeSaturated;
            } else if ( type() == DecorationButtonType::Minimize && d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights ){
                buttonFocusColor = d->systemAccentColors()->neutral;
            } else if ( type() == DecorationButtonType::Maximize && d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights ){
                buttonFocusColor = d->systemAccentColors()->positive;
            } else {
                buttonFocusColor = d->systemAccentColors()->buttonFocus;
            }
        } else {
            if( type() == DecorationButtonType::Close ) { 
                buttonFocusColor = d->systemAccentColors()->negativeSaturated;
            } else {
                buttonFocusColor = KColorUtils::mix( d->titleBarColor(), d->fontColor(), 0.3 );
            }
        }
        
        if( m_lowContrastBetweenTitleBarAndBackground && m_backgroundColor == buttonFocusColor )
            buttonFocusColor = d->fontColor();
        
        if( isPressed() || ( isChecked() && (type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade || type() == DecorationButtonType::OnAllDesktops) ) ) {
            return buttonFocusColor;
        } else if( m_animation->state() == QAbstractAnimation::Running ) {
            if( type() == DecorationButtonType::Close && d->internalSettings()->outlineCloseButton() ) {
                if( !( d->internalSettings()->redOutline() && c->isActive() ) ) return KColorUtils::mix( KColorUtils::mix( d->titleBarColor(), d->fontColor(), 0.3 ), buttonFocusColor, m_opacity );
                else return buttonFocusColor;
            }
            else {
                QColor color( buttonFocusColor );
                color.setAlphaF( color.alphaF()*m_opacity );
                return color;
            }
        } else if( type() == DecorationButtonType::Close && d->internalSettings()->outlineCloseButton() ) {
            if( d->internalSettings()->redOutline() ) {
                if( c->isActive() ) return buttonFocusColor;
                else if( isHovered() ){ 
                     return buttonFocusColor;
                } else return KColorUtils::mix( d->titleBarColor(), d->fontColor(), 0.3 );
            } else if( isHovered() ){
                return buttonFocusColor;
            } else return KColorUtils::mix( d->titleBarColor(), d->fontColor(), 0.3 );

        } else {
            return buttonFocusColor;
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
        
        if ( d->internalSettings()->alwaysShowIconHighlightUsing() == InternalSettings::EnumAlwaysShowIconHighlightUsing::AlwaysShowIconHighlightUsingBackgroundAndOutline )
            return true;
        else return ( m_lowContrastBetweenTitleBarAndBackground );
    }
    
    void Button::paintFullSizedButtonBackground(QPainter* painter) const
    {        
        if( m_backgroundColor.isValid() )
        {            
            auto d = qobject_cast<Decoration*>( decoration() );
            auto s = d->settings();
            
            painter->save();
            painter->setRenderHints( QPainter::Antialiasing, true );
            painter->setBrush( m_backgroundColor );
            
            QRectF backgroundBoundingRect;
            QPainterPath background;
            backgroundBoundingRect = geometry();
            
            if( shouldDrawBackgroundStroke() )
            {   
                QPen pen( m_outlineColor );
                pen.setWidthF( PenWidth::Symbol * m_devicePixelRatio );
                pen.setCosmetic(true);
                painter->setPen(pen);
                
                QRectF strokeRect;
                
                qreal geometryShrinkOffsetHorizontal = PenWidth::Symbol *1.5;
                if( !KWindowSystem::isPlatformWayland() ) geometryShrinkOffsetHorizontal *= m_devicePixelRatio;
                qreal geometryShrinkOffsetVertical = geometryShrinkOffsetHorizontal * 1.1;
                
                //shrink the backgroundBoundingRect to make border more visible
                strokeRect= QRectF( backgroundBoundingRect.adjusted( geometryShrinkOffsetHorizontal, geometryShrinkOffsetVertical, -geometryShrinkOffsetHorizontal, -geometryShrinkOffsetVertical ) );
                
                
                if( d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullSizedRoundedRectangle )
                    background.addRoundedRect(strokeRect,d->scaledCornerRadius(),d->scaledCornerRadius());
                else background.addRect(strokeRect);
            } else {
                painter->setPen( Qt::NoPen );
                if( d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullSizedRoundedRectangle )
                    background.addRoundedRect(backgroundBoundingRect,d->scaledCornerRadius(),d->scaledCornerRadius());
                else background.addRect( backgroundBoundingRect );
            }
            
            //clip the rounded corners using the windowPath
            if( !d->isMaximized() && s->isAlphaChannelSupported()  )
                background = background.intersected( *(d->windowPath()) );
            
            painter->drawPath( background );
            
            painter->restore();
        }
        
    }
    
    //paints large circle background shape
    void Button::paintLargeSizedButtonBackground(QPainter* painter) const
    {
        if( m_backgroundColor.isValid() )
        {  
            auto d = qobject_cast<Decoration*>( decoration() );
            auto s = d->settings();
            
            painter->save();
            
            painter->translate(m_largeBackgroundOffset);
            painter->setRenderHints( QPainter::Antialiasing, true );
            painter->setBrush( m_backgroundColor );
            
            qreal padding = s->smallSpacing()*2;
            qreal diameter = geometry().height() - padding;
            QRectF backgroundBoundingRect(
                qreal(geometry().topLeft().x()) + padding/2,
                qreal(geometry().topLeft().y()) + padding/2,
                diameter,
                diameter
            );
            QPainterPath background;
            
            if( shouldDrawBackgroundStroke() )
            {   
                QPen pen( m_outlineColor );
                pen.setWidthF( PenWidth::Symbol * m_devicePixelRatio );
                pen.setCosmetic(true);
                painter->setPen(pen);
                
            } else {
                painter->setPen( Qt::NoPen );
                //if( d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullSizedRoundedRectangle )
                //    background.addRoundedRect(backgroundBoundingRect,d->scaledCornerRadius(),d->scaledCornerRadius());
            }
            
            background.addEllipse( backgroundBoundingRect );
            
            //clip the rounded corners using the windowPath
            //do this for all buttons as there are some edge cases where even the non front/back buttons in the list may be at the window edge
            //kde-gtk-config kwin_bridge (via kded5) does not draw all buttons if we clip with titlebarPath
            //      -kwin_bridge works if we use windowPath, but cannot be sure either detection method will work in future so use both methods
            /*if( !d->isMaximized() && s->isAlphaChannelSupported() && !m_isGtkCsdButton )
                background = background.intersected( *(d->windowPath()) );*/
            
            painter->drawPath( background );
            
            painter->restore();
            
        }

    }
    
    void Button::paintSmallSizedButtonBackground(QPainter* painter) const
    {
        
        if( m_backgroundColor.isValid() )
        {
            auto d = qobject_cast<Decoration*>( decoration() );
            
            painter->save();
            
            qreal geometryShrinkOffset = 0;
            
            if( shouldDrawBackgroundStroke() )
            {   
                QPen pen( m_outlineColor );
                pen.setWidthF(PenWidth::Symbol*qMax((qreal)1.0, (qreal)20/m_iconSize.width() ));
                painter->setPen(pen);
            } else painter->setPen( Qt::NoPen );
            painter->setBrush( m_backgroundColor );
            
            if( d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeSmallSquare )
                painter->drawRect(
                    QRectF( 0 + geometryShrinkOffset, 0 + geometryShrinkOffset, 18 - geometryShrinkOffset, 18 - geometryShrinkOffset)
                );
            else if (
                d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeSmallRoundedSquare
                || d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullSizedRectangle //case where standalone
                || d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullSizedRoundedRectangle //case where standalone
            ) {
                painter->drawRoundedRect(
                    QRectF( 0 + geometryShrinkOffset, 0 + geometryShrinkOffset, 18 - geometryShrinkOffset, 18 - geometryShrinkOffset),
                    20,
                    20, Qt::RelativeSize
                );
            } else painter->drawEllipse( 
                QRectF( 0 + geometryShrinkOffset, 0 + geometryShrinkOffset, 18 - geometryShrinkOffset, 18 - geometryShrinkOffset) 
            );
            
            painter->restore();
        }
    }
    

    void Button::setDevicePixelRatio(QPainter* painter)
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        
        //determine DPR
        m_devicePixelRatio = painter->device()->devicePixelRatioF();
        
        // on X11 Kwin just returns 1.0 for the DPR instead of the correct value, so use the scaling setting directly
        if( KWindowSystem::isPlatformX11() ) m_devicePixelRatio = d->systemScaleFactor();
        if ( m_isGtkCsdButton ) m_devicePixelRatio = d->systemScaleFactor(); 
        
    }
    
    void Button::setStandardScaledPenWidth()
    {
        m_standardScaledPenWidth = PenWidth::Symbol;
        m_standardScaledPenWidth *= m_devicePixelRatio; //this is assuming you are going to use setCosmetic(true) for pen sizes
    }
    
    void Button::setShouldDrawBoldButtonIcons()
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        
        m_boldButtonIcons = false;
        
        switch( d->internalSettings()->boldButtonIcons() )
        {
            case InternalSettings::BoldIconsAuto:
                // If HiDPI system scaling use bold icons
                if ( m_devicePixelRatio  > 1.2 ) m_boldButtonIcons = true;
                break;
            case InternalSettings::BoldIconsFine:
            default:
                break;
            case InternalSettings::BoldIconsBold:
                m_boldButtonIcons = true;
                break;
        }
        
    }
    
} // namespace
