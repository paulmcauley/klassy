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

#include "breezedecoration.h"

#include "breeze.h"

#include "breezebutton.h"
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButtonGroup>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationShadow>

#include <KConfigGroup>
#include <KColorUtils>
#include <KSharedConfig>
#include <KPluginFactory>

#include <QPainter>
#include <QTextStream>

K_PLUGIN_FACTORY_WITH_JSON(
    BreezeDecoFactory,
    "breeze.json",
    registerPlugin<Breeze::Decoration>();
    registerPlugin<Breeze::Button>(QStringLiteral("button"));
)

namespace Breeze
{


    //________________________________________________________________
    static int g_sDecoCount = 0;
    static QSharedPointer<KDecoration2::DecorationShadow> g_sShadow;

    Decoration::Decoration(QObject *parent, const QVariantList &args)
        : KDecoration2::Decoration(parent, args)
        , m_colorSettings(client().data()->palette())
        , m_leftButtons(nullptr)
        , m_rightButtons(nullptr)
        , m_animation( new QPropertyAnimation( this ) )
    {
        g_sDecoCount++;
    }

    //________________________________________________________________
    Decoration::~Decoration()
    {
        g_sDecoCount--;
        if (g_sDecoCount == 0) {
            // last deco destroyed, clean up shadow
            g_sShadow.clear();
        }
    }

    //________________________________________________________________
    void Decoration::init()
    {

        // active state change animation
        m_animation->setStartValue( 0 );
        m_animation->setEndValue( 1.0 );
        m_animation->setTargetObject( this );
        m_animation->setPropertyName( "opacity" );
        m_animation->setEasingCurve( QEasingCurve::InOutQuad );


        recalculateBorders();
        updateTitleBar();
        auto s = settings();
        connect(s.data(), &KDecoration2::DecorationSettings::borderSizeChanged, this, &Decoration::recalculateBorders);

        // a change in font might cause the borders to change
        connect(s.data(), &KDecoration2::DecorationSettings::fontChanged, this, &Decoration::recalculateBorders);
        connect(s.data(), &KDecoration2::DecorationSettings::spacingChanged, this, &Decoration::recalculateBorders);
        connect(s.data(), &KDecoration2::DecorationSettings::reconfigured, this, &Decoration::reconfigure);
        connect(client().data(), &KDecoration2::DecoratedClient::adjacentScreenEdgesChanged, this, &Decoration::recalculateBorders);
        connect(client().data(), &KDecoration2::DecoratedClient::maximizedHorizontallyChanged, this, &Decoration::recalculateBorders);
        connect(client().data(), &KDecoration2::DecoratedClient::maximizedVerticallyChanged, this, &Decoration::recalculateBorders);
        connect(client().data(), &KDecoration2::DecoratedClient::captionChanged, this,
            [this]()
            {
                // update the caption area
                update(captionRect());
            }
        );

        connect(client().data(), &KDecoration2::DecoratedClient::activeChanged, this, &Decoration::updateAnimationState);
        connect(client().data(), &KDecoration2::DecoratedClient::paletteChanged,   this,
            [this]() {
                m_colorSettings.update(client().data()->palette());
                update();
            }
        );
        connect(client().data(), &KDecoration2::DecoratedClient::widthChanged,     this, &Decoration::updateTitleBar);
        connect(client().data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateTitleBar);
        connect(client().data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::setOpaque);

        connect(client().data(), &KDecoration2::DecoratedClient::widthChanged,     this, &Decoration::updateButtonPositions);
        connect(client().data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateButtonPositions);
        connect(client().data(), &KDecoration2::DecoratedClient::shadedChanged,    this, &Decoration::updateButtonPositions);

        createButtons();
        createShadow();
    }

    //________________________________________________________________
    void Decoration::updateTitleBar()
    {
        auto s = settings();
        const bool maximized = client().data()->isMaximized();
        const int width = client().data()->width();
        const int height = maximized ? borderTop() : borderTop() - s->smallSpacing()*Metrics::TitleBar_TopMargin;
        const int x = maximized ? 0 : s->largeSpacing()*Metrics::TitleBar_SideMargin;
        const int y = maximized ? 0 : s->smallSpacing()*Metrics::TitleBar_TopMargin;
        setTitleBar(QRect(x, y, width, height));
    }

    //________________________________________________________________
    void Decoration::updateAnimationState()
    {
        if( m_internalSettings.animationsEnabled() )
        {
            m_animation->setDirection( client().data()->isActive() ? QPropertyAnimation::Forward : QPropertyAnimation::Backward );
            if( m_animation->state() != QPropertyAnimation::Running ) m_animation->start();
        }
    }

    //________________________________________________________________
    static int borderSize(const QSharedPointer<KDecoration2::DecorationSettings> &settings, bool bottom)
    {
        const int baseSize = settings->smallSpacing();
        switch (settings->borderSize()) {
            case KDecoration2::BorderSize::None:
            return 0;
            case KDecoration2::BorderSize::NoSides:
            return bottom ? baseSize : 0;
            case KDecoration2::BorderSize::Tiny:
            return baseSize;
            case KDecoration2::BorderSize::Normal:
            return baseSize*2;
            case KDecoration2::BorderSize::Large:
            return baseSize * 3;
            case KDecoration2::BorderSize::VeryLarge:
            return baseSize * 4;
            case KDecoration2::BorderSize::Huge:
            return baseSize * 5;
            case KDecoration2::BorderSize::VeryHuge:
            return baseSize * 6;
            case KDecoration2::BorderSize::Oversized:
            return baseSize * 10;
            default:
            return baseSize;
        }
    }

    //________________________________________________________________
    static int borderSize(const QSharedPointer<KDecoration2::DecorationSettings> &settings)
    {
        return borderSize(settings, false);
    }

    //________________________________________________________________
    void Decoration::reconfigure()
    {
        m_internalSettings.read();
        m_animation->setDuration( m_internalSettings.animationsDuration() );
        recalculateBorders();
    }

    //________________________________________________________________
    void Decoration::recalculateBorders()
    {
        auto s = settings();
        const auto c = client().data();
        const Qt::Edges edges = c->adjacentScreenEdges();
        int left   = c->isMaximizedHorizontally() || edges.testFlag(Qt::LeftEdge) ? 0 : borderSize(s);
        int right  = c->isMaximizedHorizontally() || edges.testFlag(Qt::RightEdge) ? 0 : borderSize(s);

        QFontMetrics fm(s->font());
        int top = qMax(fm.boundingRect(c->caption()).height(), buttonHeight() );

        // padding below
        // extra pixel is used for the active window outline
        const int baseSize = settings()->smallSpacing();
        top += baseSize*Metrics::TitleBar_TopMargin + 1;

        // padding above
        if (!c->isMaximized()) top += baseSize*TitleBar_BottomMargin;

        int bottom = c->isMaximizedVertically() || edges.testFlag(Qt::BottomEdge) ? 0 : borderSize(s, true);
        setBorders(QMargins(left, top, right, bottom));

        // extended sizes
        const int extSize = s->largeSpacing();
        int extSides = 0;
        int extBottom = 0;
        if (s->borderSize() == KDecoration2::BorderSize::None)
        {
            extSides = extSize;
            extBottom = extSize;

        } else if (s->borderSize() == KDecoration2::BorderSize::NoSides) {

            extSides = extSize;

        }

        setResizeOnlyBorders(QMargins(extSides, 0, extSides, extBottom));
    }

    //________________________________________________________________
    void Decoration::createButtons()
    {
        m_leftButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Left, this, &Button::create);
        m_rightButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Right, this, &Button::create);
        updateButtonPositions();
    }

    //________________________________________________________________
    void Decoration::updateButtonPositions()
    {
        auto s = settings();
        const int padding = (client().data()->isMaximized() ? 0 : s->smallSpacing()*Metrics::TitleBar_TopMargin) + (captionHeight()-buttonHeight())/2;

        m_rightButtons->setSpacing(s->smallSpacing()*Metrics::TitleBar_ButtonSpacing);
        m_leftButtons->setSpacing(s->smallSpacing()*Metrics::TitleBar_ButtonSpacing);
        m_leftButtons->setPos(QPointF(padding, padding));
        m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width() - padding, padding));
    }

    //________________________________________________________________
    void Decoration::paint(QPainter *painter, const QRect &repaintRegion)
    {
        // TODO: optimize based on repaintRegion

        // paint background
        painter->fillRect(rect(), Qt::transparent);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(m_colorSettings.frame(client().data()->isActive()));

        // clip away the top part
        painter->save();
        painter->setClipRect(0, borderTop(), size().width(), size().height() - borderTop(), Qt::IntersectClip);
        painter->drawRoundedRect(rect(), Metrics::Frame_FrameRadius, Metrics::Frame_FrameRadius);
        painter->restore();

        paintTitleBar(painter, repaintRegion);

        painter->restore();
    }

    //________________________________________________________________
    void Decoration::paintTitleBar(QPainter *painter, const QRect &repaintRegion)
    {
        const auto c = client().data();
        const bool active = c->isActive();
        const QRect titleRect(QPoint(0, 0), QSize(size().width(), borderTop()));
        const QColor titleBarColor( this->titleBarColor() );

        // render a linear gradient on title area
        QLinearGradient gradient( 0, 0, 0, titleRect.height() );
        gradient.setColorAt(0.0, titleBarColor.lighter(100.0));
        gradient.setColorAt(0.8, titleBarColor);

        painter->save();
        painter->setBrush(gradient);
        painter->setPen(Qt::NoPen);

        if (c->isMaximized())
        {

            painter->drawRect(titleRect);

        } else {

            // we make the rect a little bit larger to be able to clip away the rounded corners on bottom
            painter->setClipRect(titleRect, Qt::IntersectClip);
            painter->drawRoundedRect(titleRect.adjusted(0, 0, 0, Metrics::Frame_FrameRadius), Metrics::Frame_FrameRadius, Metrics::Frame_FrameRadius);

        }

        auto s = settings();

        if( active )
        {
            // outline
            painter->setRenderHint( QPainter::Antialiasing, false );
            painter->setBrush( Qt::NoBrush );
            painter->setPen( c->palette().color( QPalette::Highlight ) );
            painter->drawLine( titleRect.bottomLeft(), titleRect.bottomRight() );
        }

        painter->restore();

        // draw caption
        painter->setFont(s->font());
        const QRect cR = captionRect();
        const QString caption = painter->fontMetrics().elidedText(c->caption(), Qt::ElideMiddle, cR.width());
        painter->setPen(m_colorSettings.font(c->isActive()));
        painter->drawText(cR, Qt::AlignCenter | Qt::TextSingleLine, caption);

        // draw all buttons
        m_leftButtons->paint(painter, repaintRegion);
        m_rightButtons->paint(painter, repaintRegion);
    }

    //________________________________________________________________
    int Decoration::buttonHeight() const
    {
        const int baseSize = settings()->gridUnit();
        switch( m_internalSettings.buttonSize() )
        {
            case Breeze::InternalSettings::ButtonVerySmall: return baseSize;

            default:
            case Breeze::InternalSettings::ButtonSmall: return baseSize*1.5;
            case Breeze::InternalSettings::ButtonDefault: return baseSize*2;
            case Breeze::InternalSettings::ButtonLarge: return baseSize*2.5;
            case Breeze::InternalSettings::ButtonVeryLarge: return baseSize*3.5;
        }

    }

    //________________________________________________________________
    int Decoration::captionHeight() const
    {

        return client().data()->isMaximized() ?
            borderTop() - settings()->smallSpacing()*Metrics::TitleBar_BottomMargin - 1:
            borderTop() - settings()->smallSpacing()*(Metrics::TitleBar_BottomMargin + Metrics::TitleBar_TopMargin ) - 1;
    }

    //________________________________________________________________
    QRect Decoration::captionRect() const
    {
        const int leftOffset = m_leftButtons->geometry().x() + m_leftButtons->geometry().width();
        const int rightOffset = size().width() - m_rightButtons->geometry().x();
        const int yOffset = client().data()->isMaximized() ? 0 : settings()->smallSpacing()*Metrics::TitleBar_TopMargin;

        QRect boundingRect( settings()->fontMetrics().boundingRect( client().data()->caption() ).toRect() );
        boundingRect.setTop( yOffset );
        boundingRect.setHeight( captionHeight() );

        switch( m_internalSettings.titleAlignment() )
        {
            case Breeze::InternalSettings::AlignLeft:
            boundingRect.moveLeft( leftOffset );
            break;

            case Breeze::InternalSettings::AlignRight:
            boundingRect.moveRight( size().width() - rightOffset - 1 );
            break;

            case Breeze::InternalSettings::AlignCenter:
            boundingRect.moveLeft( leftOffset + (size().width() - leftOffset - rightOffset - boundingRect.width() )/2 );
            break;

            default:
            case Breeze::InternalSettings::AlignCenterFullWidth:
            boundingRect.moveLeft( ( size().width() - boundingRect.width() )/2 );
            break;

        }

        // make sure there is no overlap with buttons
        if( boundingRect.left() < leftOffset )
        {

            boundingRect.moveLeft( leftOffset );
            boundingRect.setRight( qMin( boundingRect.right(), size().width() - rightOffset - 1 ) );

        } else if( boundingRect.right() >  size().width() - rightOffset - 1 ) {

            boundingRect.moveRight( size().width() - rightOffset - 1 );
            boundingRect.setLeft( qMax( boundingRect.left(), leftOffset ) );
        }

        return boundingRect;

    }

    //________________________________________________________________
    void Decoration::createShadow()
    {
        if (g_sShadow) {
            setShadow(g_sShadow);
            return;
        }
        auto decorationShadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
        decorationShadow->setPadding(QMargins(10, 10, 20, 20));
        decorationShadow->setInnerShadowRect(QRect(19, 19, 1, 1));

        // QImage image(60, 60, QImage::Format_ARGB32_Premultiplied);
        QImage image(40, 40, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);

        auto gradientStopColor = [](QColor color, qreal alpha) {
            color.setAlphaF(alpha);
            return color;
        };

        const QColor shadowColor( client().data()->palette().color( QPalette::Shadow ) );

        QRadialGradient radialGradient(20, 20, 20);
        radialGradient.setColorAt(0.0,  gradientStopColor(shadowColor, 0.35));
        radialGradient.setColorAt(0.25, gradientStopColor(shadowColor, 0.25));
        radialGradient.setColorAt(0.5,  gradientStopColor(shadowColor, 0.13));
        radialGradient.setColorAt(0.75, gradientStopColor(shadowColor, 0.04));
        radialGradient.setColorAt(1.0,  gradientStopColor(shadowColor, 0.0));

        QPainter p(&image);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(QRect(0, 0, 40, 40), radialGradient);

        decorationShadow->setShadow(image);

        g_sShadow = decorationShadow;
        setShadow(decorationShadow);
    }

    //________________________________________________________________
    QColor Decoration::titleBarColor() const
    {

        if( m_animation->state() == QPropertyAnimation::Running )
        {
            return KColorUtils::mix( m_colorSettings.inactiveTitleBar(), m_colorSettings.activeTitleBar(), m_opacity );
        } else return m_colorSettings.titleBar( client().data()->isActive() );

    }

    //________________________________________________________________
    QColor Decoration::outlineColor() const
    {

        if( m_animation->state() == QPropertyAnimation::Running )
        {
            QColor color( client().data()->palette().color( QPalette::Highlight ) );
            color.setAlpha( color.alpha()*m_opacity );
            return color;
        } else if( client().data()->isActive() ) return client().data()->palette().color( QPalette::Highlight );
        else return QColor();
    }

    //________________________________________________________________
    QColor Decoration::fontColor() const
    {

        if( m_animation->state() == QPropertyAnimation::Running )
        {
            return KColorUtils::mix( m_colorSettings.inactiveFont(), m_colorSettings.activeFont(), m_opacity );
        } else return m_colorSettings.font( client().data()->isActive() );

    }

} // namespace


#include "breezedecoration.moc"
