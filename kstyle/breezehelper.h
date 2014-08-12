#ifndef breeze_helper_h
#define breeze_helper_h

/*
 * Copyright 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
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

#include "breeze.h"
#include "breezeanimationdata.h"

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

        //!@name color utilities
        //@{

        //! add alpha channel multiplier to color
        QColor alphaColor( QColor color, qreal alpha ) const;

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
        QColor frameOutlineColor( const QPalette&, bool mouseOver = false, bool hasFocus = false, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //! frame background color
        QColor frameBackgroundColor( const QPalette& palette ) const
        { return frameBackgroundColor( palette, palette.currentColorGroup() ); }

        //! frame background color
        QColor frameBackgroundColor( const QPalette&, QPalette::ColorGroup ) const;

        //! return altered palette suitable for tinted frames
        QPalette framePalette( const QPalette& ) const;

        //! arrow outline color, using animations
        QColor arrowColor( const QPalette&, bool mouseOver, bool hasFocus, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //! button outline color, using animations
        QColor buttonOutlineColor( const QPalette&, bool mouseOver, bool hasFocus, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //! button panel color, using animations
        QColor buttonBackgroundColor( const QPalette&, bool mouseOver, bool hasFocus, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //! tool button color
        QColor toolButtonColor( const QPalette&, bool mouseOver, bool hasFocus, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //! slider outline color, using animations
        QColor sliderOutlineColor( const QPalette&, bool mouseOver, bool hasFocus, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //! checkbox indicator, using animations
        QColor checkBoxIndicatorColor( const QPalette&, bool mouseOver, bool active, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //! separator color
        QColor separatorColor( const QPalette& ) const;

        //! merge active and inactive palettes based on ratio, for smooth enable state change transition
        QPalette disabledPalette( const QPalette&, qreal ratio ) const;

        //@}

        //!@name rendering utilities
        //@{

        //! debug frame
        void renderDebugFrame( QPainter*, const QRect& ) const;

        //! focus line
        void renderFocusLine( QPainter*, const QRect&, const QColor& ) const;

        //! generic frame
        void renderFrame( QPainter*, const QRect&, const QColor& color, const QColor& outline = QColor(), bool focus = false ) const;

        //! menu frame
        void renderMenuFrame( QPainter*, const QRect&, const QColor& color, const QColor& outline, bool roundCorners = true ) const;

        //! button frame
        void renderButtonFrame( QPainter*, const QRect&, const QColor& color, const QColor& outline, const QColor& shadow, bool focus, bool sunken ) const;

        //! toolbutton frame
        void renderToolButtonFrame( QPainter*, const QRect&, const QColor& color, bool sunken ) const;

        //! toolbutton frame
        void renderToolBoxFrame( QPainter*, const QRect&, int tabWidth, const QColor& color ) const;

        //! tab widget frame
        void renderTabWidgetFrame( QPainter*, const QRect&, const QColor& color, const QColor& outline, Corners corners ) const;

        //! selection frame
        void renderSelection( QPainter*, const QRect&, const QColor&, Corners ) const;

        //! separator
        void renderSeparator( QPainter*, const QRect&, const QColor&, bool vertical = false ) const;

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
        { return renderScrollBarHandle( painter, rect, color ); }

        //! scrollbar handle
        void renderScrollBarHandle( QPainter*, const QRect&, const QColor& ) const;

        //! toolbar handle
        void renderToolBarHandle( QPainter* painter, const QRect& rect, const QColor& color ) const
        { return renderProgressBarGroove( painter, rect, color ); }

        //! tabbar tab
        void renderTabBarTab( QPainter*, const QRect&, const QColor& color, const QColor& outline, Corners ) const;

        //! generic arrow
        void renderArrow( QPainter*, const QRect&, const QColor&, ArrowOrientation ) const;

        //! generic button (for mdi decorations, tabs and dock widgets)
        void renderButton( QPainter*, const QRect&, const QColor&, ButtonType, bool inverted ) const;

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

        //! frame radius
        qreal frameRadius( void ) const
        { return qreal( Metrics::Frame_FrameRadius )-0.5; }

        //! return rectangle for widgets shadow, offset depending on light source
        QRectF shadowRect( const QRectF& ) const;

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

#endif
