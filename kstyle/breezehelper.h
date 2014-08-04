#ifndef breeze_helper_h
#define breeze_helper_h

/*
 * Copyright 2009-2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * Copyright 2008 Long Huynh Huu <long.upcase@googlemail.com>
 * Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright 2007 Casper Boemann <cbr@boemann.dk>
 * Copyright 2007 Fredrik H?glund <fredrik@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "breezeanimationmodes.h"

#include <KSharedConfig>
#include <KColorScheme>

#include <QPainterPath>
#include <QWidget>

#if HAVE_X11
#include <QX11Info>
#include <xcb/xcb.h>
#endif

namespace Breeze
{

    //! breeze style helper class.
    /*! contains utility functions used at multiple places in both breeze style and breeze window decoration */
    class Helper
    {
        public:

        //! scoped pointer convenience typedef
        template <typename T> using ScopedPointer = QScopedPointer<T, QScopedPointerPodDeleter>;

        //! constructor
        explicit Helper( KSharedConfig::Ptr );

        //! destructor
        virtual ~Helper()
        {}

        //! load configuration
        virtual void loadConfig();

        //! pointer to shared config
        KSharedConfigPtr config() const;

        //! add alpha channel multiplier to color
        static QColor alphaColor( QColor color, qreal alpha );

        //!@name color utilities
        //@{

        //! mouse over color
        QColor hoverColor( const QPalette& palette ) const
        { return _viewHoverBrush.brush( palette ).color(); }

        //! focus color
        QColor focusColor( const QPalette& palette ) const
        { return _viewFocusBrush.brush( palette ).color(); }

        //! negative text color (used for close button)
        QColor negativeText( const QPalette& palette ) const
        { return _viewNegativeTextBrush.brush( palette ).color(); }

        //! shadow
        QColor shadowColor( const QPalette& palette ) const
        { return alphaColor( palette.color( QPalette::Shadow ), 0.2 ); }

        //! frame outline color, using animations
        QColor frameOutlineColor( const QPalette&, bool mouseOver = false, bool hasFocus = false, qreal opacity = -1, AnimationMode = AnimationNone ) const;

        //! frame background color
        QColor frameBackgroundColor( const QPalette& palette ) const
        { return frameBackgroundColor( palette, palette.currentColorGroup() ); }

        //! frame background color
        QColor frameBackgroundColor( const QPalette&, QPalette::ColorGroup ) const;

        //! return altered palette suitable for tinted frames
        QPalette framePalette( const QPalette& ) const;

        //! button outline color, using animations
        QColor buttonOutlineColor( const QPalette&, bool mouseOver, bool hasFocus, qreal opacity, AnimationMode ) const;

        //! button panel color, using animations
        QColor buttonBackgroundColor( const QPalette&, bool mouseOver, bool hasFocus, qreal opacity, AnimationMode ) const;

        //! slider outline color, using animations
        QColor sliderOutlineColor( const QPalette&, bool mouseOver, bool hasFocus, qreal opacity, AnimationMode ) const;

        //! checkbox indicator, using animations
        QColor checkBoxIndicatorColor( const QPalette&, bool mouseOver, bool active, qreal opacity = -1, AnimationMode = AnimationNone ) const;

        //! separator color
        QColor separatorColor( const QPalette& ) const;

        //! merge active and inactive palettes based on ratio, for smooth enable state change transition
        QPalette disabledPalette( const QPalette&, qreal ratio ) const;

        //@}

        //!@name rendering utilities
        //@{

        //! debug frame
        void renderDebugRect( QPainter*, const QRect& ) const;

        //! generic frame
        void renderFrame( QPainter*, const QRect&, const QColor& color, const QColor& outline, bool focus = false ) const;

        //! menu frame
        void renderMenuFrame( QPainter*, const QRect&, const QColor& color, const QColor& outline, bool roundCorners = true ) const;

        //! button frame
        void renderButtonFrame( QPainter*, const QRect&, const QColor& color, const QColor& outline, const QColor& shadow, bool focus, bool sunken ) const;

        //! corner enumeration, needed for tabbar tabs
        enum Corner
        {
            CornerTopLeft = 1 << 0,
            CornerTopRight = 1 << 1,
            CornerBottomLeft = 1 << 2,
            CornerBottomRight = 1 << 3,
            CornersTop = CornerTopLeft|CornerTopRight,
            CornersBottom = CornerBottomLeft|CornerBottomRight,
            CornersLeft = CornerTopLeft|CornerBottomLeft,
            CornersRight = CornerTopRight|CornerBottomRight,
            CornersAll = CornerTopLeft|CornerTopRight|CornerBottomLeft|CornerBottomRight
        };

        Q_DECLARE_FLAGS( Corners, Corner );

        //! tab widget frame
        void renderTabWidgetFrame( QPainter*, const QRect&, const QColor& color, const QColor& outline, Corners corners ) const;

        //! separator
        void renderSeparator( QPainter*, const QRect&, const QColor&, bool vertical = false ) const;

        //! checkbox state (used for checkboxes _and_ radio buttons)
        enum CheckBoxState
        {
            CheckOff = 0,
            CheckPartial = 1,
            CheckOn = 2
        };

        //! checkbox
        void renderCheckBox( QPainter*, const QRect&, const QColor& color, const QColor& shadow, bool sunken, CheckBoxState state ) const;

        //! radio button
        void renderRadioButton( QPainter*, const QRect&, const QColor& color, const QColor& shadow, bool sunken, bool checked ) const;

        //! slider groove
        void renderSliderGroove( QPainter*, const QRect&, const QColor& ) const;

        //! slider handle
        void renderSliderHandle( QPainter*, const QRect&, const QColor&, const QColor& outline, const QColor& shadow, bool focus, bool sunken ) const;

        //! dial groove
        void renderDialGroove( QPainter*, const QRect&, const QColor& ) const;

        //! dial groove
        void renderDialContents( QPainter*, const QRect&, const QColor&, qreal first, qreal second ) const;

        //! progress bar groove
        void renderProgressBarGroove( QPainter*, const QRect&, const QColor& ) const;

        //! progress bar contents
        void renderProgressBarContents( QPainter* painter, const QRect& rect, const QColor& color ) const
        { return renderProgressBarGroove( painter, rect, color ); }

        //! progress bar contents (animated)
        void renderProgressBarBusyContents( QPainter* painter, const QRect& rect, const QColor& first, const QColor& second, bool horizontal, bool reverse, int progress  ) const;

        //! scrollbar groove
        void renderScrollBarGroove( QPainter* painter, const QRect& rect, const QColor& color ) const
        { return renderScrollBarHandle( painter, rect, color, QColor() ); }

        //! scrollbar handle
        void renderScrollBarHandle( QPainter*, const QRect&, const QColor& color, const QColor& outline ) const;

        //! tabbar tab
        void renderTabBarTab( QPainter*, const QRect&, const QColor& color, const QColor& outline, Corners ) const;

        //@}

        //!@name compositing utilities
        //@{

        //! true if style was compiled for and is running on X11
        bool isX11() const
        { return _isX11; }

        //! returns true if compositing is active
        bool compositingActive( void ) const;

        //! returns true if a given widget supports alpha channel
        bool hasAlphaChannel( const QWidget* ) const;

        #if HAVE_X11

        //! create xcb atom
        xcb_atom_t createAtom( const QString& ) const;

        #endif

        //@}

        protected:

        //! return color key for a given color, properly accounting for invalid colors
        quint64 colorKey( const QColor& color ) const
        { return color.isValid() ? color.rgba():0; }

        //! return rounded path in a given rect, with only selected corners rounded, and for a given radius
        QPainterPath roundedPath( const QRectF&, qreal, Corners ) const;

        private:

        //! configuration
        KSharedConfigPtr _config;

        //!@name brushes
        //@{
        KStatefulBrush _viewFocusBrush;
        KStatefulBrush _viewHoverBrush;
        KStatefulBrush _viewNegativeTextBrush;
        //@}

        bool _isX11;

        #if HAVE_X11

        //! atom used for compositing manager
        xcb_atom_t _compositingManagerAtom;

        #endif

    };

}

Q_DECLARE_OPERATORS_FOR_FLAGS( Breeze::Helper::Corners );

#endif
