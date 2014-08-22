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

#include "breezeshadowhelper.h"
#include "breezeshadowhelper.moc"

#include "breeze.h"
#include "breezehelper.h"
#include "breezestyleconfigdata.h"

#include <QDockWidget>
#include <QEvent>
#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QToolBar>
#include <QTextStream>

#if HAVE_X11
#include <QX11Info>
#endif

namespace Breeze
{

    const char* const ShadowHelper::netWMShadowAtomName( "_KDE_NET_WM_SHADOW" );
    const char* const ShadowHelper::netWMForceShadowPropertyName( "_KDE_NET_WM_FORCE_SHADOW" );
    const char* const ShadowHelper::netWMSkipShadowPropertyName( "_KDE_NET_WM_SKIP_SHADOW" );

    //_____________________________________________________
    ShadowHelper::ShadowHelper( QObject* parent, Helper& helper ):
        QObject( parent ),
        _helper( helper ),
        _supported( checkSupported() )
        #if HAVE_X11
        ,_gc( 0 ),
        _atom( 0 )
        #endif
    {}

    //_______________________________________________________
    ShadowHelper::~ShadowHelper( void )
    {

        #if HAVE_X11
        if( Helper::isX11() )
        { foreach( const uint32_t& value, _pixmaps  ) xcb_free_pixmap( Helper::connection(), value ); }
        #endif

    }

    //______________________________________________
    void ShadowHelper::reset( void )
    {
        #if HAVE_X11
        if( Helper::isX11() )
        { foreach( const uint32_t& value, _pixmaps  ) xcb_free_pixmap( Helper::connection(), value ); }
        #endif

        _pixmaps.clear();
        _shadowTiles = TileSet();

    }

    //_______________________________________________________
    bool ShadowHelper::registerWidget( QWidget* widget, bool force )
    {

        // do nothing if not supported
        if( !_supported ) return false;

        // make sure widget is not already registered
        if( _widgets.contains( widget ) ) return false;

        // check if widget qualifies
        if( !( force || acceptWidget( widget ) ) )
        { return false; }

        // store in map and add destroy signal connection
        widget->removeEventFilter( this );
        widget->installEventFilter( this );
        _widgets.insert( widget, 0 );

        /*
        need to install shadow directly when widget "created" state is already set
        since WinID changed is never called when this is the case
        */
        if( widget->testAttribute(Qt::WA_WState_Created) && installX11Shadows( widget ) )
        {  _widgets.insert( widget, widget->winId() ); }

        connect( widget, SIGNAL(destroyed(QObject*)), SLOT(objectDeleted(QObject*)) );

        return true;

    }

    //_______________________________________________________
    void ShadowHelper::unregisterWidget( QWidget* widget )
    {
        if( _widgets.remove( widget ) )
        { uninstallX11Shadows( widget ); }
    }

    //_______________________________________________________
    bool ShadowHelper::eventFilter( QObject* object, QEvent* event )
    {

        // check event type
        if( event->type() != QEvent::WinIdChange ) return false;

        // cast widget
        QWidget* widget( static_cast<QWidget*>( object ) );

        // install shadows and update winId
        if( installX11Shadows( widget ) )
        { _widgets.insert( widget, widget->winId() ); }

        return false;

    }

    //_______________________________________________________
    TileSet ShadowHelper::shadowTiles( void )
    {
        if( !_shadowTiles.isValid() )
        {

            const QPalette palette( QApplication::palette() );
            const QColor shadowColor( palette.color( QPalette::Shadow ) );

            // create pixmap
            QPixmap pixmap = QPixmap( Metrics::Shadow_Size*3, Metrics::Shadow_Size*3 );
            pixmap.fill( Qt::transparent );

            // paint
            QPainter painter( &pixmap );
            painter.setRenderHint( QPainter::Antialiasing );
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.setPen( Qt::NoPen );

            auto gradientStopColor = [](QColor color, qreal alpha) {
                color.setAlphaF(alpha);
                return color;
            };

            QRadialGradient radialGradient(Metrics::Shadow_Size, Metrics::Shadow_Size, Metrics::Shadow_Size);
            radialGradient.setColorAt(0.0,  gradientStopColor( shadowColor, 0.35 ) );
            radialGradient.setColorAt(0.25, gradientStopColor( shadowColor, 0.25 ) );
            radialGradient.setColorAt(0.5,  gradientStopColor( shadowColor, 0.13 ) );
            radialGradient.setColorAt(0.75, gradientStopColor( shadowColor, 0.04 ) );
            radialGradient.setColorAt(1.0,  gradientStopColor( shadowColor, 0.0 ) );

            QLinearGradient linearGradient;
            linearGradient.setColorAt(0.0,  gradientStopColor( shadowColor, 0.35 ) );
            linearGradient.setColorAt(0.25, gradientStopColor( shadowColor, 0.25 ) );
            linearGradient.setColorAt(0.5,  gradientStopColor( shadowColor, 0.13 ) );
            linearGradient.setColorAt(0.75, gradientStopColor( shadowColor, 0.04 ) );
            linearGradient.setColorAt(1.0,  gradientStopColor( shadowColor, 0.0) );

            // topLeft
            QRect rect( 0, 0, Metrics::Shadow_Size, Metrics::Shadow_Size );
            painter.fillRect(rect, radialGradient);

            // top
            rect.translate( Metrics::Shadow_Size, 0 );
            linearGradient.setStart( rect.bottomLeft() );
            linearGradient.setFinalStop( rect.topLeft() );
            painter.fillRect( rect, linearGradient );

            // topRight
            rect.translate( Metrics::Shadow_Size, 0 );
            radialGradient.setCenter( rect.bottomLeft() );
            radialGradient.setFocalPoint( rect.bottomLeft() );
            painter.fillRect( rect, radialGradient );

            // right
            rect.translate( 0, Metrics::Shadow_Size );
            linearGradient.setStart( rect.topLeft() );
            linearGradient.setFinalStop( rect.topRight() );
            painter.fillRect( rect, linearGradient);

            // bottom right
            rect.translate( 0, Metrics::Shadow_Size );
            radialGradient.setCenter( rect.topLeft() );
            radialGradient.setFocalPoint( rect.topLeft() );
            painter.fillRect( rect, radialGradient );

            // bottom
            rect.translate( -Metrics::Shadow_Size, 0 );
            linearGradient.setStart( rect.topLeft() );
            linearGradient.setFinalStop( rect.bottomLeft() );
            painter.fillRect( rect, linearGradient);

            // bottom left
            rect.translate( -Metrics::Shadow_Size, 0 );
            radialGradient.setCenter( rect.topRight() );
            radialGradient.setFocalPoint( rect.topRight() );
            painter.fillRect( rect, radialGradient );

            // left
            rect.translate( 0, -Metrics::Shadow_Size );
            linearGradient.setStart( rect.topRight() );
            linearGradient.setFinalStop( rect.topLeft() );
            painter.fillRect( rect, linearGradient);

            painter.end();

            // create tiles from pixmap
            _shadowTiles = TileSet( pixmap, Metrics::Shadow_Size, Metrics::Shadow_Size, 1, 1 );

        }

        return _shadowTiles;

    }


    //_______________________________________________________
    void ShadowHelper::objectDeleted( QObject* object )
    { _widgets.remove( static_cast<QWidget*>( object ) ); }

    //_______________________________________________________
    bool ShadowHelper::checkSupported( void ) const
    {

        // create atom
        #if HAVE_X11

        // make sure we are on X11
        if( !Helper::isX11() ) return false;

        // create atom
        xcb_atom_t netSupportedAtom( _helper.createAtom( "_NET_SUPPORTED" ) );
        if( !netSupportedAtom ) return false;

        // store connection locally
        xcb_connection_t* connection( Helper::connection() );

        // get property
        const uint32_t maxLength = std::string().max_size();
        xcb_get_property_cookie_t cookie( xcb_get_property( connection, 0, QX11Info::appRootWindow(), netSupportedAtom, XCB_ATOM_ATOM, 0, (maxLength+3) / 4 ) );
        ScopedPointer<xcb_get_property_reply_t> reply( xcb_get_property_reply( connection, cookie, nullptr ) );
        if( !reply ) return false;

        // get reply length and data
        const int count( xcb_get_property_value_length( reply.data() )/sizeof( xcb_atom_t ) );
        xcb_atom_t *atoms = reinterpret_cast<xcb_atom_t*>( xcb_get_property_value( reply.data() ) );

        bool found( false );
        for( int i = 0; i < count && !found; ++i )
        {
            // get atom name and print
            xcb_atom_t atom( atoms[i] );

            xcb_get_atom_name_cookie_t cookie( xcb_get_atom_name( connection, atom ) );
            ScopedPointer<xcb_get_atom_name_reply_t> reply( xcb_get_atom_name_reply( connection, cookie, 0 ) );
            if( !reply ) continue;

            // get name and compare
            const QString name( QByteArray( xcb_get_atom_name_name( reply.data() ), xcb_get_atom_name_name_length( reply.data() ) ) );
            if( strcmp( netWMShadowAtomName, xcb_get_atom_name_name( reply.data() ) ) == 0 ) found = true;

        }

        return found;

        #else
        return false;
        #endif

    }

    //_______________________________________________________
    bool ShadowHelper::isMenu( QWidget* widget ) const
    { return qobject_cast<QMenu*>( widget ); }

    //_______________________________________________________
    bool ShadowHelper::isToolTip( QWidget* widget ) const
    { return widget->inherits( "QTipLabel" ) || (widget->windowFlags() & Qt::WindowType_Mask) == Qt::ToolTip; }

    //_______________________________________________________
    bool ShadowHelper::isDockWidget( QWidget* widget ) const
    { return qobject_cast<QDockWidget*>( widget ); }

    //_______________________________________________________
    bool ShadowHelper::isToolBar( QWidget* widget ) const
    { return qobject_cast<QToolBar*>( widget ); }

    //_______________________________________________________
    bool ShadowHelper::acceptWidget( QWidget* widget ) const
    {

        // flags
        if( widget->property( netWMSkipShadowPropertyName ).toBool() ) return false;
        if( widget->property( netWMForceShadowPropertyName ).toBool() ) return true;

        // menus
        if( isMenu( widget ) ) return true;

        // combobox dropdown lists
        if( widget->inherits( "QComboBoxPrivateContainer" ) ) return true;

        // tooltips
        if( isToolTip( widget ) && !widget->inherits( "Plasma::ToolTip" ) )
        { return true; }

        // detached widgets
        if( isDockWidget( widget ) || isToolBar( widget ) )
        { return true; }

        // reject
        return false;
    }

    //______________________________________________
    const QVector<uint32_t>& ShadowHelper::createPixmapHandles( void )
    {

        /**
        shadow atom and property specification available at
        http://community.kde.org/KWin/Shadow
        */

        // create atom
        #if HAVE_X11
        if( !_atom && Helper::isX11() ) _atom = _helper.createAtom( QLatin1String( netWMShadowAtomName ) );
        #endif

        shadowTiles();

        // make sure size is valid
        if( _pixmaps.empty() && _shadowTiles.isValid() )
        {

            _pixmaps.push_back( createPixmap( _shadowTiles.pixmap( 1 ) ) );
            _pixmaps.push_back( createPixmap( _shadowTiles.pixmap( 2 ) ) );
            _pixmaps.push_back( createPixmap( _shadowTiles.pixmap( 5 ) ) );
            _pixmaps.push_back( createPixmap( _shadowTiles.pixmap( 8 ) ) );
            _pixmaps.push_back( createPixmap( _shadowTiles.pixmap( 7 ) ) );
            _pixmaps.push_back( createPixmap( _shadowTiles.pixmap( 6 ) ) );
            _pixmaps.push_back( createPixmap( _shadowTiles.pixmap( 3 ) ) );
            _pixmaps.push_back( createPixmap( _shadowTiles.pixmap( 0 ) ) );

        }

        // return relevant list of pixmap handles
        return _pixmaps;

    }

    //______________________________________________
    uint32_t ShadowHelper::createPixmap( const QPixmap& source )
    {

        // do nothing for invalid pixmaps
        if( source.isNull() ) return 0;
        if( !Helper::isX11() ) return 0;

        /*
        in some cases, pixmap handle is invalid. This is the case notably
        when Qt uses to RasterEngine. In this case, we create an X11 Pixmap
        explicitly and draw the source pixmap on it.
        */

        #if HAVE_X11

        const int width( source.width() );
        const int height( source.height() );

        // create X11 pixmap
        xcb_pixmap_t pixmap = xcb_generate_id( Helper::connection() );
        xcb_create_pixmap( Helper::connection(), 32, pixmap, QX11Info::appRootWindow(), width, height );

        // create gc
        if( !_gc )
        {
            _gc = xcb_generate_id( Helper::connection() );
            xcb_create_gc( Helper::connection(), _gc, pixmap, 0, 0x0 );
        }

        // create image from QPixmap and assign to pixmap
        QImage image( source.toImage() );
        xcb_put_image( Helper::connection(), XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap, _gc, image.width(), image.height(), 0, 0, 0, 32, image.byteCount(), image.constBits());

        return pixmap;

        #else
        return 0;
        #endif

    }

    //_______________________________________________________
    bool ShadowHelper::installX11Shadows( QWidget* widget )
    {

        // do nothing if not supported
        if( !_supported ) return false;

        // check widget and shadow
        if( !widget ) return false;
        if( !Helper::isX11() ) return false;

        #if HAVE_X11
        #ifndef QT_NO_XRENDER

        // TODO: also check for NET_WM_SUPPORTED atom, before installing shadow

        /*
        From bespin code. Supposibly prevent playing with some 'pseudo-widgets'
        that have winId matching some other -random- window
        */
        if( !(widget->testAttribute(Qt::WA_WState_Created) || widget->internalWinId() ))
        { return false; }

        // create pixmap handles if needed
        const QVector<uint32_t>& pixmaps( createPixmapHandles() );
        if( pixmaps.size() != numPixmaps ) return false;

        // create data
        // add pixmap handles
        QVector<uint32_t> data;
        foreach( const uint32_t& value, pixmaps )
        { data.push_back( value ); }

        // add padding
        /*
        in most cases all 4 paddings are identical, since offsets are handled when generating the pixmaps.
        There is one extra pixel needed with respect to actual shadow size, to deal with how
        menu backgrounds are rendered.
        Some special care is needed for QBalloonTip, since the later have an arrow
        */

        // also need to decrement default size further due to extra hard coded round corner
        int size = Metrics::Shadow_Size - Metrics::Shadow_Overlap;
        if( widget->inherits( "QBalloonTip" ) )
        {

            // balloon tip needs special margins to deal with the arrow
            int top = 0;
            int bottom = 0;
            widget->getContentsMargins(NULL, &top, NULL, &bottom );

            // also need to decrement default size further due to extra hard coded round corner
            size -= 2;

            // it seems arrow can be either to the top or the bottom. Adjust margins accordingly
            if( top > bottom ) data << size - (top - bottom) << size << size << size;
            else data << size << size << size - (bottom - top) << size;

        } else {

            // data << size << size << size << size;
            data << size - Metrics::Shadow_Offset << size << size << size - Metrics::Shadow_Offset;

        }

        xcb_change_property( Helper::connection(), XCB_PROP_MODE_REPLACE, widget->winId(), _atom, XCB_ATOM_CARDINAL, 32, data.size(), data.constData() );
        xcb_flush( Helper::connection() );

        return true;

        #endif
        #endif

        return false;

    }

    //_______________________________________________________
    void ShadowHelper::uninstallX11Shadows( QWidget* widget ) const
    {

        #if HAVE_X11
        if( !_supported ) return;
        if( !Helper::isX11() ) return;
        if( !( widget && widget->testAttribute(Qt::WA_WState_Created) ) ) return;
        xcb_delete_property( Helper::connection(), widget->winId(), _atom);
        #else
        Q_UNUSED( widget )
        #endif

    }

}
