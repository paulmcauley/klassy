/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 * Copyright (C) 2018 by Vlad Zagorodniy <vlad.zahorodnii@kde.org>       *
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
#include "breezeboxshadowrenderer.h"
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

namespace
{
    using Breeze::CompositeShadowParams;
    using Breeze::ShadowParams;

    const CompositeShadowParams s_shadowParams[] = {
        // None
        CompositeShadowParams(),
        // Small
        CompositeShadowParams(
            QPoint(0, 3),
            ShadowParams(QPoint(0, 0), 12, 0.26),
            ShadowParams(QPoint(0, -2), 6, 0.16)),
        // Medium
        CompositeShadowParams(
            QPoint(0, 4),
            ShadowParams(QPoint(0, 0), 16, 0.24),
            ShadowParams(QPoint(0, -2), 8, 0.14)),
        // Large
        CompositeShadowParams(
            QPoint(0, 5),
            ShadowParams(QPoint(0, 0), 20, 0.22),
            ShadowParams(QPoint(0, -3), 10, 0.12)),
        // Very Large
        CompositeShadowParams(
            QPoint(0, 6),
            ShadowParams(QPoint(0, 0), 24, 0.2),
            ShadowParams(QPoint(0, -3), 12, 0.1))
    };
}

namespace Breeze
{

    const char ShadowHelper::netWMShadowAtomName[] ="_KDE_NET_WM_SHADOW";

    //_____________________________________________________
    CompositeShadowParams ShadowHelper::lookupShadowParams(int shadowSizeEnum)
    {

        switch (shadowSizeEnum) {
        case StyleConfigData::ShadowNone:
            return s_shadowParams[0];
        case StyleConfigData::ShadowSmall:
            return s_shadowParams[1];
        case StyleConfigData::ShadowMedium:
            return s_shadowParams[2];
        case StyleConfigData::ShadowLarge:
            return s_shadowParams[3];
        case StyleConfigData::ShadowVeryLarge:
            return s_shadowParams[4];
        default:
            // Fallback to the Large size.
            return s_shadowParams[3];
        }

    }

    //_____________________________________________________
    ShadowHelper::ShadowHelper( QObject* parent, Helper& helper ):
        QObject( parent ),
        _helper( helper )
    {
        // delay till event dispatcher is running as Wayland is highly async
        QMetaObject::invokeMethod(this, "initializeWayland", Qt::QueuedConnection);
    }

    //_______________________________________________________
    ShadowHelper::~ShadowHelper()
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
        auto registry = new Registry( connection );
        registry->create( connection );
        connect(registry, &Registry::interfacesAnnounced, this,
            [registry, this] {
                const auto interface = registry->interface( Registry::Interface::Shadow );
                if( interface.name != 0 ) {
                    _shadowManager = registry->createShadowManager( interface.name, interface.version, registry );
                }
                const auto shmInterface = registry->interface( Registry::Interface::Shm );
                if( shmInterface.name != 0 ) {
                    _shmPool = registry->createShmPool( shmInterface.name, shmInterface.version, registry );
                }
            }
        );

        registry->setup();
        connection->roundtrip();
        #endif
    }

    //______________________________________________
    void ShadowHelper::reset()
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
    void ShadowHelper::loadConfig()
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
        if( Helper::isWayland() )
        {

            #if BREEZE_HAVE_KWAYLAND
            QWidget* widget( static_cast<QWidget*>( object ) );
            if( event->type() == QEvent::Paint )
            {

                auto iter = _widgetSurfaces.constFind( widget );
                if( iter == _widgetSurfaces.constEnd() )
                {
                    // install shadows and update winId
                    installShadows( widget );
                }

            } else if( event->type() == QEvent::Hide ) {

                auto iter = _widgetSurfaces.find( widget );
                if( iter != _widgetSurfaces.end() )
                {
                    _widgetSurfaces.erase( iter );
                }

            }
            #endif

        } else if( Helper::isX11() ) {

            // check event type
            if( event->type() != QEvent::WinIdChange ) return false;

            // cast widget
            QWidget* widget( static_cast<QWidget*>( object ) );

            // install shadows and update winId
            if( installShadows( widget ) )
            { _widgets.insert( widget, widget->winId() ); }
        }

        return false;

    }

    //_______________________________________________________
    TileSet ShadowHelper::shadowTiles()
    {
        const CompositeShadowParams params = lookupShadowParams(StyleConfigData::shadowSize());

        if (params.isNone()) {
            return TileSet();
        } else if (_shadowTiles.isValid()) {
            return _shadowTiles;
        }

        auto withOpacity = [](const QColor &color, qreal opacity) -> QColor {
            QColor c(color);
            c.setAlphaF(opacity);
            return c;
        };

        const QColor color = StyleConfigData::shadowColor();
        const qreal strength = static_cast<qreal>(StyleConfigData::shadowStrength()) / 255.0;

        const QSize boxSize = BoxShadowRenderer::calculateMinimumBoxSize(params.shadow1.radius)
            .expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(params.shadow2.radius));

        const qreal dpr = qApp->devicePixelRatio();
        const qreal frameRadius = _helper.frameRadius();

        BoxShadowRenderer shadowRenderer;
        shadowRenderer.setBorderRadius(frameRadius);
        shadowRenderer.setBoxSize(boxSize);
        shadowRenderer.setDevicePixelRatio(dpr);

        shadowRenderer.addShadow(params.shadow1.offset, params.shadow1.radius,
            withOpacity(color, params.shadow1.opacity * strength));
        shadowRenderer.addShadow(params.shadow2.offset, params.shadow2.radius,
            withOpacity(color, params.shadow2.opacity * strength));

        QImage shadowTexture = shadowRenderer.render();

        const QRect outerRect(QPoint(0, 0), shadowTexture.size() / dpr);

        QRect boxRect(QPoint(0, 0), boxSize);
        boxRect.moveCenter(outerRect.center());

        // Mask out inner rect.
        QPainter painter(&shadowTexture);
        painter.setRenderHint(QPainter::Antialiasing);

        const QMargins margins = QMargins(
            boxRect.left() - outerRect.left() - Metrics::Shadow_Overlap - params.offset.x(),
            boxRect.top() - outerRect.top() - Metrics::Shadow_Overlap - params.offset.y(),
            outerRect.right() - boxRect.right() - Metrics::Shadow_Overlap + params.offset.x(),
            outerRect.bottom() - boxRect.bottom() - Metrics::Shadow_Overlap + params.offset.y());

        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        painter.drawRoundedRect(
            outerRect - margins,
            frameRadius,
            frameRadius);

        // We're done.
        painter.end();

        const QPoint innerRectTopLeft = outerRect.center();
        _shadowTiles = TileSet(
            QPixmap::fromImage(shadowTexture),
            innerRectTopLeft.x(),
            innerRectTopLeft.y(),
            1, 1);

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
    const QVector<quint32>& ShadowHelper::createPixmapHandles()
    {

        /**
        shadow atom and property specification available at
        https://community.kde.org/KWin/Shadow
        */

        // create atom
        #if BREEZE_HAVE_X11
        if( !_atom && Helper::isX11() ) _atom = _helper.createAtom( QLatin1String( netWMShadowAtomName ) );
        #endif

        // make sure size is valid
        if( _pixmaps.empty() )
        {
            _pixmaps = QVector<quint32> {
                createPixmap( _shadowTiles.pixmap( 1 ) ),
                createPixmap( _shadowTiles.pixmap( 2 ) ),
                createPixmap( _shadowTiles.pixmap( 5 ) ),
                createPixmap( _shadowTiles.pixmap( 8 ) ),
                createPixmap( _shadowTiles.pixmap( 7 ) ),
                createPixmap( _shadowTiles.pixmap( 6 ) ),
                createPixmap( _shadowTiles.pixmap( 3 ) ),
                createPixmap( _shadowTiles.pixmap( 0 ) )
            };
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
            xcb_create_gc( Helper::connection(), _gc, pixmap, 0, nullptr );
        }

        // create image from QPixmap and assign to pixmap
        QImage image( source.toImage() );


        #if QT_VERSION >= 0x051000
        xcb_put_image( Helper::connection(), XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap, _gc, image.width(), image.height(), 0, 0, 0, 32, image.sizeInBytes(), image.constBits());
        #else
        xcb_put_image( Helper::connection(), XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap, _gc, image.width(), image.height(), 0, 0, 0, 32, image.byteCount(), image.constBits());
        #endif

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
        From bespin code. Supposedly prevent playing with some 'pseudo-widgets'
        that have winId matching some other -random- window
        */
        if( !(widget->testAttribute(Qt::WA_WState_Created) && widget->internalWinId() ))
        { return false; }

        // create shadow tiles if needed
        shadowTiles();

        if( !_shadowTiles.isValid() ) return false;

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
        QVector<quint32> data( createPixmapHandles() );
        if( data.size() != numPixmaps ) return false;

        const QMargins margins = shadowMargins( widget );
        const quint32 topSize = margins.top();
        const quint32 bottomSize = margins.bottom();
        const quint32 leftSize( margins.left() );
        const quint32 rightSize( margins.right() );

        // assign to data and xcb property
        data << QVector<quint32>{topSize, rightSize, bottomSize, leftSize};
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
        _widgetSurfaces.insert(widget, s);

        return true;
        #else
        Q_UNUSED( widget );
        #endif

        return false;
    }

    //_______________________________________________________
    QMargins ShadowHelper::shadowMargins( QWidget* widget ) const
    {
        const CompositeShadowParams params = lookupShadowParams(StyleConfigData::shadowSize());
        if (params.isNone()) {
            return QMargins();
        }

        const QSize boxSize = BoxShadowRenderer::calculateMinimumBoxSize(params.shadow1.radius)
            .expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(params.shadow2.radius));

        const QSize shadowSize = BoxShadowRenderer::calculateMinimumShadowTextureSize(boxSize, params.shadow1.radius, params.shadow1.offset)
            .expandedTo(BoxShadowRenderer::calculateMinimumShadowTextureSize(boxSize, params.shadow2.radius, params.shadow2.offset));

        const QRect shadowRect(QPoint(0, 0), shadowSize);

        QRect boxRect(QPoint(0, 0), boxSize);
        boxRect.moveCenter(shadowRect.center());

        QMargins margins(
            boxRect.left() - shadowRect.left() - Metrics::Shadow_Overlap - params.offset.x(),
            boxRect.top() - shadowRect.top() - Metrics::Shadow_Overlap - params.offset.y(),
            shadowRect.right() - boxRect.right() - Metrics::Shadow_Overlap + params.offset.x(),
            shadowRect.bottom() - boxRect.bottom() - Metrics::Shadow_Overlap + params.offset.y());

        if (widget->inherits("QBalloonTip")) {
            // Balloon tip needs special margins to deal with the arrow.
            int top = widget->contentsMargins().top();
            int bottom = widget->contentsMargins().bottom();

            // Need to decrement default size further due to extra hard coded round corner.
            margins -= 1;

            // Arrow can be either to the top or the bottom. Adjust margins accordingly.
            const int diff = qAbs(top - bottom);
            if (top > bottom) {
                margins.setTop(margins.top() - diff);
            } else {
                margins.setBottom(margins.bottom() - diff);
            }
        }

        margins *= _helper.devicePixelRatio(_shadowTiles.pixmap(0));

        return margins;
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
