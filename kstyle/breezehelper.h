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

#include "breezetileset.h"

#include <KSharedConfig>
#include <KColorScheme>

#include <QCache>
#include <QPixmap>
#include <QScopedPointer>

#if HAVE_X11
#include <xcb/xcb.h>
#endif

namespace Breeze
{

    template<typename T> class BaseCache: public QCache<quint64, T>
    {

        public:

        //! constructor
        BaseCache( int maxCost ):
            QCache<quint64, T>( maxCost ),
            _enabled( true )
        {}

        //! constructor
        explicit BaseCache( void ):
            _enabled( true )
            {}

        //! destructor
        ~BaseCache( void )
        {}

        //! enable
        void setEnabled( bool value )
        { _enabled = value; }

        //! enable state
        bool enabled( void ) const
        { return _enabled; }

        //! access
        T* object( const quint64& key )
        { return _enabled ? QCache<quint64, T>::object( key ) : 0; }

        //! max cost
        void setMaxCost( int cost )
        {
            if( cost <= 0 ) {

                QCache<quint64, T>::clear();
                QCache<quint64, T>::setMaxCost( 1 );
                setEnabled( false );

            } else {

                setEnabled( true );
                QCache<quint64, T>::setMaxCost( cost );

            }
        }

        private:

        //! enable flag
        bool _enabled;

    };

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

        //! reset all caches
        virtual void invalidateCaches();

        //! update maximum cache size
        virtual void setMaxCacheSize( int );

        //! add alpha channel multiplier to color
        static QColor alphaColor( QColor color, qreal alpha );

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

        //!@name pixmaps
        //@{


        //! checkbox state (used for checkboxes _and_ radio buttons)
        enum CheckBoxState
        {
            CheckOff = 0,
            CheckPartial = 1,
            CheckOn = 2
        };

        //! checkbox
        QPixmap* checkBox( const QColor& color, const QColor& shadow, bool sunken, CheckBoxState state );

        //! radio button
        QPixmap* radioButton( const QColor& color, const QColor& shadow, bool sunken, bool checked );

        //!@name tilesets
        //@{

        //! scrollbar hole
        TileSet* scrollBarHole( const QColor& );

        //! scrollbar handle
        TileSet* scrollBarHandle( const QColor&, const QColor& );

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

        //!@name pixmap caches
        //@{
        typedef BaseCache<QPixmap> PixmapCache;
        PixmapCache _checkBoxCache;
        PixmapCache _radioButtonCache;
        //@}

        //!@name tileset caches
        //@{

        typedef BaseCache<TileSet> TileSetCache;
        TileSetCache _scrollBarHandleCache;

        //@}
    };

}

#endif
