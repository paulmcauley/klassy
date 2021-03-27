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
            FlagFirstInList,
            FlagLastInList,
        };

        //* flag
        void setFlag( Flag value )
        { m_flag = value; }

        //* standalone buttons
        bool isStandAlone() const { return m_flag == FlagStandalone; }

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
        
        void setSquareButtonIconVerticalTranslation( int value )
        { m_squareHighlightIconVerticalTranslation = value; }
        
        void setSquareButtonIconHorizontalTranslation( int value )
        { m_squareHighlightIconHorizontalTranslation = value; }

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
        
        /**
        * @brief Checks the contrast ratio of the two given colours, and if is below the given threshold returns a higher contrast black or white foreground
        * @param foregroundColor The foreground colour to potentially replace
        * @param backgroundColor The background colour to compare with
        * @param blackWhiteContrastThreshold The contrast threshold, below which a black or white foreground colour will be returned
        * @return the higher contrast QColor
        */
        QColor getHigherContrastForegroundColor( const QColor& foregroundColor, const QColor& backgroundColor, double blackWhiteContrastThreshold ) const;
        QColor getBlackOrWhiteForegroundForHighContrast( const QColor& backgroundColor ) const;
        bool shouldDrawBackgroundStroke() const;
        
        /**
        * @brief Paint the button background for the square highlight style; NB: applies a translation at end due to different square button geometries
        * @param painter Current QPainter object. NB: will be modified at end with a translation
        */
        void paintSquareBackground( QPainter* painter ) const;
        
        /**
        * @brief Paint the button background for the circle highlight style;
        * @param painter Current QPainter object
        */
        void paintCircleBackground( QPainter* painter ) const;
        
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
        
        int m_squareHighlightIconVerticalTranslation = 0;
        int m_squareHighlightIconHorizontalTranslation = 0;
        bool m_lowContrastBetweenTitleBarAndBackground = false;
    };

} // namespace

#endif
