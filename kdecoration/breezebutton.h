#ifndef BREEZE_BUTTONS_H
#define BREEZE_BUTTONS_H

/*
* SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
* SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
* SPDX-FileCopyrightText: 2021 Paul McAuley <kde@paulmcauley.com>
* 
* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include <KDecoration2/DecorationButton>
#include "breezedecoration.h"

#include <QHash>
#include <QImage>

class QVariantAnimation;

namespace Breeze
{

    class Button : public KDecoration2::DecorationButton
    {
        Q_OBJECT

        public:

        //* constructor
        explicit Button(QObject *parent, const QVariantList &args);

        //* destructor
        virtual ~Button() = default;

        //* button creation
        static Button *create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);

        //* render
        virtual void paint(QPainter *painter, const QRect &repaintRegion) override;

        //* flag
        enum Flag
        {
            FlagNone,
            FlagStandalone,
            FlagLeftmostNotAtEdge,
            FlagLeftmostAndAtEdge,
            FlagRightmostNotAtEdge,
            FlagRightmostAndAtEdge,
        };

        //* flag
        void setFlag( Flag value )
        { m_flag = value; }

        //* standalone buttons
        bool isStandAlone() const { return ( m_flag == FlagStandalone || m_flag == FlagLeftmostNotAtEdge || m_flag == FlagRightmostNotAtEdge); }

        //* offset for drawing icon
        void setIconOffset( const QPointF& value )
        { m_iconOffset = value; }

        //* horizontal offset, for rendering icon
        void setHorizontalIconOffset( qreal value )
        { m_iconOffset.setX( value ); }

        //* vertical offset, for rendering icon
        void setVerticalIconOffset( qreal value )
        { m_iconOffset.setY( value ); }
        
        //* offset for drawing large background -- used for the far left button
        void setLargeBackgroundOffset( const QPointF& value )
        { m_largeBackgroundOffset = value; }
        
        //* set icon size
        void setIconSize( const QSize& value )
        { m_iconSize = value; }

        //*@name active state change animation
        //@{
        void setOpacity( qreal value )
        {
            if( m_opacity == value ) return;
            m_opacity = value;
            update();
        }

        qreal opacity() const
        { return m_opacity; }

        //@}

        private Q_SLOTS:

        //* apply configuration changes
        void reconfigure();

        //* animation state
        void updateAnimationState(bool);

        private:

        //* private constructor
        explicit Button(KDecoration2::DecorationButtonType type, Decoration *decoration, QObject *parent = nullptr);

        //* draw button icon
        void drawIcon( QPainter *) const;

        //*@name colors
        //@{
        QColor foregroundColor() const;
        QColor backgroundColor() const;
        QColor outlineColor() const;
        //@}
        
        bool shouldDrawBackgroundStroke() const;
        void setDevicePixelRatio(QPainter* painter);
        void setShouldDrawBoldButtonIcons();
        void setStandardScaledPenWidth();
        
        /**
        * @brief Paint the button background for the Full-sized Rectangle button shape;
        * @param painter Current QPainter object.
        */
        void paintFullSizedButtonBackground( QPainter* painter ) const;
        
        /**
        * @brief Paint the button background for the large circle button shape
        * @param painter Current QPainter object
        */
        void paintLargeSizedButtonBackground( QPainter* painter ) const;
        
        /**
        * @brief Paint the button background for the small circle button shape as originally done in Breeze
        *        Also paints the small square or small rounded square shapes
        * @param painter Current QPainter object
        */
        void paintSmallSizedButtonBackground( QPainter* painter) const;
        
        QColor m_foregroundColor;
        QColor m_backgroundColor;
        QColor m_outlineColor;

        Flag m_flag = FlagNone;

        //* active state change animation
        QVariantAnimation *m_animation;

        //* icon offset (for rendering)
        QPointF m_iconOffset;
        
        QPointF m_largeBackgroundOffset = QPointF(0,0);

        //* icon size
        QSize m_iconSize;

        //* active state change opacity
        qreal m_opacity = 0;
        
        bool m_lowContrastBetweenTitleBarAndBackground = false;
        bool m_isGtkCsdButton;
        qreal m_devicePixelRatio = 1.0;
        bool m_boldButtonIcons;
        qreal m_standardScaledPenWidth = 1.0;
    };

} // namespace

#endif
