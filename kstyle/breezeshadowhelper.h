#ifndef breezeshadowhelper_h
#define breezeshadowhelper_h

//////////////////////////////////////////////////////////////////////////////
// breezeshadowhelper.h
// handle shadow pixmaps passed to window manager via X property
// -------------------
//
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "breezetileset.h"

#include <QObject>
#include <QMap>

#if HAVE_X11
#include <xcb/xcb.h>
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
        static const char* const netWMShadowAtomName;
        static const char* const netWMForceShadowPropertyName;
        static const char* const netWMSkipShadowPropertyName;
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

        protected:

        //* true if shadows are supported
        bool checkSupported( void ) const;

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
        const QVector<uint32_t>& createPixmapHandles( void );

        // create pixmap handle from pixmap
        uint32_t createPixmap( const QPixmap& );

        //* install shadow X11 property on given widget
        /**
        shadow atom and property specification available at
        http://community.kde.org/KWin/Shadow
        */
        bool installX11Shadows( QWidget* );

        //* uninstall shadow X11 property on given widget
        void uninstallX11Shadows( QWidget* ) const;

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
        QVector<uint32_t> _pixmaps;

        #if HAVE_X11

        //* graphical context
        xcb_gcontext_t _gc;

        //* shadow atom
        xcb_atom_t _atom;

        #endif

    };

}

#endif
