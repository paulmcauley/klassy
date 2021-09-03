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

        //* offset
        void setOffset( const QPointF& value )
        { m_offset = value; }

        //* horizontal offset, for rendering
        void setHorizontalOffset( qreal value )
        { m_offset.setX( value ); }

        //* vertical offset, for rendering
        void setVerticalOffset( qreal value )
        { m_offset.setY( value ); }

        //* set icon size
        void setIconSize( const QSize& value )
        { m_iconSize = value; }
        
        void setFullSizedRectangleButtonIconVerticalTranslation( int value )
        { m_fullSizedRectangleHighlightIconVerticalTranslation = value; }
        
        void setFullSizedRectangleButtonIconHorizontalTranslation( int value )
        { m_fullSizedRectangleHighlightIconHorizontalTranslation = value; }

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
        //@}
        
        bool shouldDrawBackgroundStroke() const;
        void setShouldDrawBoldButtonIcons(QPainter* painter);
        
        /**
        * @brief Paint the button background for the Full-sized Rectangle highlight style; NB: applies a translation at end due to different full-sized button geometries
        * @param painter Current QPainter object. NB: will be modified at end with a translation
        */
        void paintFullSizedRectangleBackground( QPainter* painter ) const;
        
        /**
        * @brief Paint the button background for the circle highlight style;
        * @param painter Current QPainter object
        * @param square If true draws a square highlight, else draws a circle
        */
        void paintCircleOrSquareBackground( QPainter* painter, bool square) const;
        
        QColor m_foregroundColor;
        QColor m_backgroundColor;

        Flag m_flag = FlagNone;

        //* active state change animation
        QVariantAnimation *m_animation;

        //* vertical offset (for rendering)
        QPointF m_offset;

        //* icon size
        QSize m_iconSize;

        //* active state change opacity
        qreal m_opacity = 0;
        
        int m_fullSizedRectangleHighlightIconVerticalTranslation = 0;
        int m_fullSizedRectangleHighlightIconHorizontalTranslation = 0;
        bool m_lowContrastBetweenTitleBarAndBackground = false;
        bool m_isGtkCsdButton;
        qreal m_devicePixelRatio = 1.0;
        bool m_boldButtonIcons;
    };

} // namespace

#endif
