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

#include "breezeshadowhelper.h"

#include "breeze.h"
#include "breezehelper.h"
#include "breezepropertynames.h"
#include "breezestyleconfigdata.h"

#include <QDockWidget>
#include <QEvent>
#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QToolBar>
#include <QTextStream>

#if BREEZE_HAVE_X11
#include <QX11Info>
#endif

#if BREEZE_HAVE_KWAYLAND
#include <KWayland/Client/buffer.h>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/shadow.h>
#include <KWayland/Client/shm_pool.h>
#include <KWayland/Client/surface.h>
#endif

namespace Breeze
{

    const char ShadowHelper::netWMShadowAtomName[] ="_KDE_NET_WM_SHADOW";

    //_____________________________________________________
    ShadowHelper::ShadowHelper( QObject* parent, Helper& helper ):
        QObject( parent ),
        _helper( helper )
        #if BREEZE_HAVE_X11
        ,_gc( 0 ),
        _atom( 0 )
        #endif
        #if BREEZE_HAVE_KWAYLAND
        , _shadowManager( Q_NULLPTR )
        , _shmPool( Q_NULLPTR )
        #endif
    {
        // delay till event dispatcher is running as Wayland is highly async
        QMetaObject::invokeMethod(this, "initializeWayland", Qt::QueuedConnection);
    }

    //_______________________________________________________
    ShadowHelper::~ShadowHelper( void )
    {

        #if BREEZE_HAVE_X11
        if( Helper::isX11() )
        { foreach( const quint32& value, _pixmaps  ) xcb_free_pixmap( Helper::connection(), value ); }
        #endif

    }

    //_______________________________________________________
    void ShadowHelper::initializeWayland()
    {
        #if BREEZE_HAVE_KWAYLAND
        if( !Helper::isWayland() ) return;

        using namespace KWayland::Client;
        auto connection = ConnectionThread::fromApplication( this );
        if( !connection ) {
            return;
        }
        Registry *registry = new Registry( this );
        registry->create( connection );
        connect(registry, &Registry::interfacesAnnounced, this,
            [registry, this] {
                const auto interface = registry->interface( Registry::Interface::Shadow );
                if( interface.name != 0 ) {
                    _shadowManager = registry->createShadowManager( interface.name, interface.version, this );
                }
                const auto shmInterface = registry->interface( Registry::Interface::Shm );
                if( shmInterface.name != 0 ) {
                    _shmPool = registry->createShmPool( shmInterface.name, shmInterface.version, this );
                }
            }
        );

        registry->setup();
        connection->roundtrip();
        #endif
    }

    //______________________________________________
    void ShadowHelper::reset( void )
    {
        #if BREEZE_HAVE_X11
        if( Helper::isX11() )
        { foreach( const quint32& value, _pixmaps  ) xcb_free_pixmap( Helper::connection(), value ); }
        #endif

        _pixmaps.clear();
        _shadowTiles = TileSet();

    }

    //_______________________________________________________
    bool ShadowHelper::registerWidget( QWidget* widget, bool force )
    {
        // make sure widget is not already registered
        if( _widgets.contains( widget ) ) return false;

        // check if widget qualifies
        if( !( force || acceptWidget( widget ) ) )
        { return false; }

        // try create shadow directly
        if( installShadows( widget ) ) _widgets.insert( widget, widget->winId() );
        else _widgets.insert( widget, 0 );

        // install event filter
        widget->removeEventFilter( this );
        widget->installEventFilter( this );

        // connect destroy signal
        connect( widget, SIGNAL(destroyed(QObject*)), SLOT(objectDeleted(QObject*)) );

        return true;

    }

    //_______________________________________________________
    void ShadowHelper::unregisterWidget( QWidget* widget )
    {
        if( _widgets.remove( widget ) )
        { uninstallShadows( widget ); }
    }

    //_______________________________________________________
    void ShadowHelper::loadConfig( void )
    {

        // reset
        reset();

        // update property for registered widgets
        for( QMap<QWidget*,WId>::const_iterator iter = _widgets.constBegin(); iter != _widgets.constEnd(); ++iter )
        { installShadows( iter.key() ); }

    }

    //_______________________________________________________
    bool ShadowHelper::eventFilter( QObject* object, QEvent* event )
    {

        // check event type
        if( event->type() != QEvent::WinIdChange ) return false;

        // cast widget
        QWidget* widget( static_cast<QWidget*>( object ) );

        // install shadows and update winId
        if( installShadows( widget ) )
        { _widgets.insert( widget, widget->winId() ); }

        return false;

    }

    //_______________________________________________________
    TileSet ShadowHelper::shadowTiles( void )
    {
        if( !_shadowTiles.isValid() )
        {

            const QPalette palette( QApplication::palette() );
            const QColor shadowColor( StyleConfigData::shadowColor() );

            // metrics
            const int shadowSize = StyleConfigData::shadowSize()*12/16;
            const int shadowOffset = qMax( shadowSize/2, Metrics::Shadow_Overlap*2 );
            const int shadowStrength = StyleConfigData::shadowStrength();

            // pixmap
            QPixmap pixmap = _helper.highDpiPixmap( shadowSize*2 );
            pixmap.fill( Qt::transparent );

            // create gradient
            // gaussian delta function
            auto alpha = [](qreal x) { return std::exp( -x*x/0.15 ); };

            // color calculation delta function
            auto gradientStopColor = [](QColor color, int alpha)
            {
                color.setAlpha(alpha);
                return color;
            };

            QRadialGradient radialGradient( shadowSize, shadowSize, shadowSize);
            for( int i = 0; i < 10; ++i )
            {
                const qreal x( qreal( i )/9 );
                radialGradient.setColorAt(x,  gradientStopColor(shadowColor, alpha(x)*shadowStrength));
            }

            radialGradient.setColorAt(1, gradientStopColor( shadowColor, 0 ) );

            // fill
            QPainter p(&pixmap);
            p.setRenderHint( QPainter::Antialiasing, true );
            p.fillRect( pixmap.rect(), radialGradient);

            p.setPen( Qt::NoPen );
            p.setBrush( Qt::black );

            QRectF innerRect(
                shadowSize - shadowOffset - Metrics::Shadow_Overlap, shadowSize - shadowOffset - Metrics::Shadow_Overlap,
                shadowOffset + 2*Metrics::Shadow_Overlap,shadowOffset + 2*Metrics::Shadow_Overlap );

            p.setCompositionMode(QPainter::CompositionMode_DestinationOut );

            const qreal radius( _helper.frameRadius() );
            p.drawRoundedRect( innerRect, radius, radius );
            p.end();

            // create tiles from pixmap
            _shadowTiles = TileSet( pixmap,
                shadowSize,
                shadowSize,
                1, 1 );

        }

        return _shadowTiles;

    }


    //_______________________________________________________
    void ShadowHelper::objectDeleted( QObject* object )
    { _widgets.remove( static_cast<QWidget*>( object ) ); }

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
        if( widget->property( PropertyNames::netWMSkipShadow ).toBool() ) return false;
        if( widget->property( PropertyNames::netWMForceShadow ).toBool() ) return true;

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
    const QVector<quint32>& ShadowHelper::createPixmapHandles( void )
    {

        /**
        shadow atom and property specification available at
        http://community.kde.org/KWin/Shadow
        */

        // create atom
        #if BREEZE_HAVE_X11
        if( !_atom && Helper::isX11() ) _atom = _helper.createAtom( QLatin1String( netWMShadowAtomName ) );
        #endif

        shadowTiles();

        // make sure size is valid
        if( _pixmaps.empty() && _shadowTiles.isValid() )
        {

            _pixmaps.append( createPixmap( _shadowTiles.pixmap( 1 ) ) );
            _pixmaps.append( createPixmap( _shadowTiles.pixmap( 2 ) ) );
            _pixmaps.append( createPixmap( _shadowTiles.pixmap( 5 ) ) );
            _pixmaps.append( createPixmap( _shadowTiles.pixmap( 8 ) ) );
            _pixmaps.append( createPixmap( _shadowTiles.pixmap( 7 ) ) );
            _pixmaps.append( createPixmap( _shadowTiles.pixmap( 6 ) ) );
            _pixmaps.append( createPixmap( _shadowTiles.pixmap( 3 ) ) );
            _pixmaps.append( createPixmap( _shadowTiles.pixmap( 0 ) ) );

        }

        // return relevant list of pixmap handles
        return _pixmaps;

    }

    //______________________________________________
    quint32 ShadowHelper::createPixmap( const QPixmap& source )
    {

        // do nothing for invalid pixmaps
        if( source.isNull() ) return 0;
        if( !Helper::isX11() ) return 0;

        /*
        in some cases, pixmap handle is invalid. This is the case notably
        when Qt uses to RasterEngine. In this case, we create an X11 Pixmap
        explicitly and draw the source pixmap on it.
        */

        #if BREEZE_HAVE_X11

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
    bool ShadowHelper::installShadows( QWidget* widget )
    {
        if( !widget ) return false;

        /*
        From bespin code. Supposibly prevent playing with some 'pseudo-widgets'
        that have winId matching some other -random- window
        */
        if( !(widget->testAttribute(Qt::WA_WState_Created) && widget->internalWinId() ))
        { return false; }

        if( Helper::isX11() ) return installX11Shadows( widget );
        if( Helper::isWayland() ) return installWaylandShadows( widget );

        return false;
    }

    //_______________________________________________________
    bool ShadowHelper::installX11Shadows( QWidget* widget )
    {
        #if BREEZE_HAVE_X11
        #ifndef QT_NO_XRENDER

        // create pixmap handles if needed
        const QVector<quint32>& pixmaps( createPixmapHandles() );
        if( pixmaps.size() != numPixmaps ) return false;

        // create data
        // add pixmap handles
        QVector<quint32> data;
        foreach( const quint32& value, pixmaps )
        { data.append( value ); }


        const QMargins margins = shadowMargins( widget );
        const int topSize = margins.top();
        const int bottomSize = margins.bottom();
        const int leftSize( margins.left() );
        const int rightSize( margins.right() );

        // assign to data and xcb property
        data << topSize << rightSize << bottomSize << leftSize;
        xcb_change_property( Helper::connection(), XCB_PROP_MODE_REPLACE, widget->winId(), _atom, XCB_ATOM_CARDINAL, 32, data.size(), data.constData() );
        xcb_flush( Helper::connection() );

        return true;

        #endif
        #endif

        return false;

    }

    //_______________________________________________________
    bool ShadowHelper::installWaylandShadows( QWidget* widget )
    {
        #if BREEZE_HAVE_KWAYLAND
        if( widget->windowHandle()->parent() ) return false;
        if( !_shadowManager || !_shmPool ) return false;

        if( !_shadowTiles.isValid() ) return false;

        // create shadow
        using namespace KWayland::Client;
        auto s = Surface::fromWindow( widget->windowHandle() );
        if( !s ) return false;

        auto shadow = _shadowManager->createShadow( s, widget );
        if( !shadow->isValid() ) return false;

        // add the shadow elements
        shadow->attachTop( _shmPool->createBuffer( _shadowTiles.pixmap( 1 ).toImage() ) );
        shadow->attachTopRight( _shmPool->createBuffer( _shadowTiles.pixmap( 2 ).toImage() ) );
        shadow->attachRight( _shmPool->createBuffer( _shadowTiles.pixmap( 5 ).toImage() ) );
        shadow->attachBottomRight( _shmPool->createBuffer( _shadowTiles.pixmap( 8 ).toImage() ) );
        shadow->attachBottom( _shmPool->createBuffer( _shadowTiles.pixmap( 7 ).toImage() ) );
        shadow->attachBottomLeft( _shmPool->createBuffer( _shadowTiles.pixmap( 6 ).toImage() ) );
        shadow->attachLeft( _shmPool->createBuffer( _shadowTiles.pixmap( 3 ).toImage() ) );
        shadow->attachTopLeft( _shmPool->createBuffer( _shadowTiles.pixmap( 0 ).toImage() ) );

        shadow->setOffsets( shadowMargins( widget ) );
        shadow->commit();
        s->commit( Surface::CommitFlag::None );

        return true;
        #else
        Q_UNUSED( widget );
        #endif

        return false;
    }

    //_______________________________________________________
    QMargins ShadowHelper::shadowMargins( QWidget* widget ) const
    {
        // get devicePixelRatio
        // for testing purposes only
        const qreal devicePixelRatio( _helper.devicePixelRatio( _shadowTiles.pixmap( 0 ) ) );

        // metrics
        const int shadowSize = StyleConfigData::shadowSize()*12/16;
        const int shadowOffset = qMax( shadowSize/2, Metrics::Shadow_Overlap*2 );

        // define shadows padding
        int size( shadowSize - Metrics::Shadow_Overlap );
        int topSize = ( size - shadowOffset ) * devicePixelRatio;
        int bottomSize = size * devicePixelRatio;
        const int leftSize( (size - shadowOffset) * devicePixelRatio );
        const int rightSize( size * devicePixelRatio );

        if( widget->inherits( "QBalloonTip" ) )
        {

            // balloon tip needs special margins to deal with the arrow
            int top = 0;
            int bottom = 0;
            widget->getContentsMargins( nullptr, &top, nullptr, &bottom );

            // also need to decrement default size further due to extra hard coded round corner
            size -= 2 * devicePixelRatio;

            // it seems arrow can be either to the top or the bottom. Adjust margins accordingly
            if( top > bottom ) topSize -= (top - bottom);
            else bottomSize -= (bottom - top );

        }

        return QMargins( leftSize, topSize, rightSize, bottomSize );
    }

    //_______________________________________________________
    void ShadowHelper::uninstallShadows( QWidget* widget ) const
    {
        if( !( widget && widget->testAttribute(Qt::WA_WState_Created) ) ) return;
        if( Helper::isX11() ) uninstallX11Shadows( widget );
        if( Helper::isWayland() ) uninstallWaylandShadows( widget );
    }

    //_______________________________________________________
    void ShadowHelper::uninstallX11Shadows( QWidget* widget ) const
    {
        #if BREEZE_HAVE_X11
        xcb_delete_property( Helper::connection(), widget->winId(), _atom);
        #else
        Q_UNUSED( widget )
        #endif

    }

    //_______________________________________________________
    void ShadowHelper::uninstallWaylandShadows( QWidget* widget ) const
    {
        #if BREEZE_HAVE_KWAYLAND
        if( widget->windowHandle() && widget->windowHandle()->parent() ) return;
        if( !_shadowManager ) return;

        using namespace KWayland::Client;
        auto s = Surface::fromWindow( widget->windowHandle() );
        if( !s ) return;

        _shadowManager->removeShadow( s );
        s->commit( Surface::CommitFlag::None );
        #else
        Q_UNUSED( widget )
        #endif
    }

}
