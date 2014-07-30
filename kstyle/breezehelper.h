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

#include <KSharedConfig>
#include <KColorScheme>

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

        //! merge active and inactive palettes based on ratio, for smooth enable state change transition
        QPalette mergePalettes( const QPalette&, qreal ratio ) const;

        //!@name brushes
        //@{

        //! focus brush
        const KStatefulBrush& viewFocusBrush( void ) const
        { return _viewFocusBrush; }

        //! hover brush
        const KStatefulBrush& viewHoverBrush( void ) const
        { return _viewHoverBrush; }

        //! negative text brush ( used for close button hover )
        const KStatefulBrush& viewNegativeTextBrush( void ) const
        { return _viewNegativeTextBrush; }

        //@}

        //!@name rendering utilities
        //@{

        //! generic frame
        void renderFrame( QPainter*, const QRect&, const QColor& color, const QColor& outline, bool focus = false ) const;

        //! generic button slab
        void renderButtonSlab( QPainter*, const QRect&, const QColor& color, const QColor& outline, const QColor& shadow, bool focus, bool sunken ) const;

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

        //! slider hole
        void renderSliderHole( QPainter*, const QRect&, const QColor& ) const;

        //! slider hole
        void renderSliderHandle( QPainter*, const QRect&, const QColor&, const QColor& outline, const QColor& shadow, bool focus, bool sunken ) const;

        //! progress bar hole
        void renderProgressBarHole( QPainter*, const QRect&, const QColor& ) const;

        //! progress bar hole
        void renderProgressBarContents( QPainter* painter, const QRect& rect, const QColor& color ) const
        { return renderProgressBarHole( painter, rect, color ); }

        //! progress bar hole
        void renderProgressBarBusyContents( QPainter* painter, const QRect& rect, const QColor& first, const QColor& second, bool horizontal, int progress  ) const;

        //! scrollbar hole
        void renderScrollBarHole( QPainter* painter, const QRect& rect, const QColor& color ) const
        { return renderScrollBarHandle( painter, rect, color, QColor() ); }

        //! scrollbar hole
        void renderScrollBarHandle( QPainter*, const QRect&, const QColor& color, const QColor& outline ) const;

        //@}

        protected:

        //! return color key for a given color, properly accounting for invalid colors
        quint64 colorKey( const QColor& color ) const
        { return color.isValid() ? color.rgba():0; }

        private:

        //! configuration
        KSharedConfigPtr _config;

        //!@name brushes
        //@{
        KStatefulBrush _viewFocusBrush;
        KStatefulBrush _viewHoverBrush;
        KStatefulBrush _viewNegativeTextBrush;
        //@}

      };

}

#endif
