#ifndef breezeshadowhelper_h
#define breezeshadowhelper_h

/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

#include "breezetileset.h"
#include "config-breeze.h"

#include <QObject>
#include <QMap>
#include <QMargins>

#if BREEZE_HAVE_X11
#include <xcb/xcb.h>
#endif

#if BREEZE_HAVE_KWAYLAND
namespace KWayland
{
namespace Client
{
    class ShadowManager;
    class ShmPool;
}
}
#endif

namespace Breeze
{

    //* forward declaration
    class Helper;

    //* handle shadow pixmaps passed to window manager via X property
    class ShadowHelper: public QObject
    {

        Q_OBJECT

        public:

        //*@name property names
        //@{
        static const char netWMShadowAtomName[];
        //@}

        //* constructor
        ShadowHelper( QObject*, Helper& );

        //* destructor
        virtual ~ShadowHelper( void );

        //* true if supported
        bool isSupported( void ) const
        { return _supported; }

        //* reset
        void reset( void );

        //* load config
        void loadConfig( void );

        //* register widget
        bool registerWidget( QWidget*, bool force = false );

        //* unregister widget
        void unregisterWidget( QWidget* );

        //* event filter
        virtual bool eventFilter( QObject*, QEvent* );

        //* shadow tiles
        /** is public because it is also needed for mdi windows */
        TileSet shadowTiles( void );

        protected Q_SLOTS:

        //* unregister widget
        void objectDeleted( QObject* );

        //* initializes the Wayland specific parts
        void initializeWayland();

        protected:

        //* true if widget is a menu
        bool isMenu( QWidget* ) const;

        //* true if widget is a tooltip
        bool isToolTip( QWidget* ) const;

        //* dock widget
        bool isDockWidget( QWidget* ) const;

        //* toolbar
        bool isToolBar( QWidget* ) const;

        //* accept widget
        bool acceptWidget( QWidget* ) const;

        // create pixmap handles from tileset
        const QVector<quint32>& createPixmapHandles( void );

        // create pixmap handle from pixmap
        quint32 createPixmap( const QPixmap& );

        //* installs shadow on given widget in a platform independent way
        bool installShadows( QWidget * );

        //* uninstalls shadow on given widget in a platform independent way
        void uninstallShadows( QWidget * ) const;

        //* install shadow X11 property on given widget
        /**
        shadow atom and property specification available at
        http://community.kde.org/KWin/Shadow
        */
        bool installX11Shadows( QWidget* );

        //* uninstall shadow X11 property on given widget
        void uninstallX11Shadows( QWidget* ) const;

        //* install shadow on given widget for Wayland
        bool installWaylandShadows( QWidget * );

        //* uninstall shadow on given widget for Wayland
        void uninstallWaylandShadows( QWidget* ) const;

        //* gets the shadow margins for the given widget
        QMargins shadowMargins( QWidget* ) const;

        private:

        //* helper
        Helper& _helper;

        //* true if supported
        bool _supported;

        //* registered widgets
        QMap<QWidget*, WId> _widgets;

        //* tileset
        TileSet _shadowTiles;

        //* number of pixmaps
        enum { numPixmaps = 8 };

        //* pixmaps
        QVector<quint32> _pixmaps;

        #if BREEZE_HAVE_X11

        //* graphical context
        xcb_gcontext_t _gc;

        //* shadow atom
        xcb_atom_t _atom;

        #endif

        #if BREEZE_HAVE_KWAYLAND
        //* The Wayland shadow manager to create Shadows for Surfaces (QWindow)
        KWayland::Client::ShadowManager* _shadowManager;
        //* The Wayland Shared memory pool to share the shadow pixmaps with compositor
        KWayland::Client::ShmPool* _shmPool;
        #endif

    };

}

#endif
