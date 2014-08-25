//////////////////////////////////////////////////////////////////////////////
// breezemdiwindowshadow.cpp
// handle MDI windows' shadows
// -------------------
//
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Largely inspired from skulpture widget style
// Copyright (c) 2007-2009 Christoph Feck <christoph@maxiom.de>
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

#include "breezemdiwindowshadow.h"
#include "breezemdiwindowshadow.moc"

#include "breeze.h"
#include "breezestyleconfigdata.h"

#include <QMdiArea>
#include <QMdiSubWindow>
#include <QPainter>
#include <QTextStream>

namespace Breeze
{

    //____________________________________________________________________
    void MdiWindowShadow::updateGeometry( void )
    {
        if( !_widget ) return;

        const int size( Metrics::Shadow_Size - Metrics::Shadow_Overlap );
        int topSize( size - Metrics::Shadow_Offset );
        int bottomSize( size );
        int rightSize;
        int leftSize;

        switch( StyleConfigData::lightSource() )
        {
            case StyleConfigData::LS_TOPLEFT:
            leftSize = size - Metrics::Shadow_Offset;
            rightSize = size;
            break;

            case StyleConfigData::LS_TOPRIGHT:
            rightSize = size - Metrics::Shadow_Offset;
            leftSize = size;
            break;

            case StyleConfigData::LS_TOP:
            leftSize = size - Metrics::Shadow_Offset/2;
            rightSize = size - Metrics::Shadow_Offset/2;
            break;

        }

        // get tileSet rect
        _shadowTilesRect = _widget->frameGeometry().adjusted( -leftSize, -topSize, rightSize, bottomSize );

        // get parent MDI area's viewport
        QWidget *parent( parentWidget() );
        if (parent && !qobject_cast<QMdiArea *>(parent) && qobject_cast<QMdiArea*>(parent->parentWidget()))
        { parent = parent->parentWidget(); }
        if( qobject_cast<QAbstractScrollArea *>( parent ) )
        { parent = qobject_cast<QAbstractScrollArea *>( parent )->viewport(); }

        // set geometry
        QRect geometry( _shadowTilesRect );
        if( parent ) geometry &= parent->rect();
        setGeometry( geometry );

        // translate rendering rect
        _shadowTilesRect.translate( -geometry.topLeft() );

    }

    //____________________________________________________________________
    void MdiWindowShadow::updateZOrder( void )
    { stackUnder( _widget ); }

    //____________________________________________________________________
    void MdiWindowShadow::paintEvent( QPaintEvent* event )
    {

        if( !_shadowTiles.isValid() ) return;

        QPainter p( this );
        p.setRenderHints( QPainter::Antialiasing );
        p.setClipRegion( event->region() );
        _shadowTiles.render( _shadowTilesRect, &p );

    }

    //____________________________________________________________________
    MdiWindowShadowFactory::MdiWindowShadowFactory( QObject* parent ):
        QObject( parent )
    {}

    //____________________________________________________________________________________
    bool MdiWindowShadowFactory::registerWidget( QWidget* widget )
    {

        // check widget type
        if( !( widget && qobject_cast<QMdiSubWindow*>( widget ) ) ) return false;

        // make sure widget is not already registered
        if( isRegistered( widget ) ) return false;

        // store in set
        _registeredWidgets.insert( widget );

        widget->installEventFilter( this );

        // catch object destruction
        connect( widget, SIGNAL(destroyed(QObject*)), SLOT(widgetDestroyed(QObject*)) );

        return true;

    }

    //____________________________________________________________________________________
    void MdiWindowShadowFactory::unregisterWidget( QWidget* widget )
    {
        if( !isRegistered( widget ) ) return;
        widget->removeEventFilter( this );
        _registeredWidgets.remove( widget );
        removeShadow( widget );
    }

    //____________________________________________________________________________________
    bool MdiWindowShadowFactory::eventFilter( QObject* object, QEvent* event )
    {

        switch( event->type() )
        {
            // TODO: possibly implement ZOrderChange event, to make sure that
            // the shadow is always painted on top
            case QEvent::ZOrderChange:
            updateShadowZOrder( object );
            break;

            case QEvent::Destroy:
            if( isRegistered( object ) )
            {
                _registeredWidgets.remove( object );
                removeShadow( object );
            }
            break;

            case QEvent::Hide:
            hideShadows( object );
            break;

            case QEvent::Show:
            installShadow( object );
            updateShadowGeometry( object );
            updateShadowZOrder( object );
            break;

            case QEvent::Move:
            case QEvent::Resize:
            updateShadowGeometry( object );
            break;

            default: break;
        }

        return QObject::eventFilter( object, event );

    }

    //____________________________________________________________________________________
    MdiWindowShadow* MdiWindowShadowFactory::findShadow( QObject* object ) const
    {

        // check object,
        if( !object->parent() ) return 0L;

        // find existing window shadows
        const QList<QObject* > children = object->parent()->children();
        foreach( QObject *child, children )
        {
            if( MdiWindowShadow* shadow = qobject_cast<MdiWindowShadow*>(child) )
            { if( shadow->widget() == object ) return shadow; }
        }

        return 0L;

    }

    //____________________________________________________________________________________
    void MdiWindowShadowFactory::installShadow( QObject* object )
    {

        // cast
        QWidget* widget( static_cast<QWidget*>( object ) );
        if( !widget->parentWidget() ) return;

        // make sure shadow is not already installed
        if( findShadow( object ) ) return;

        // create new shadow
        MdiWindowShadow* windowShadow( new MdiWindowShadow( widget->parentWidget(), _shadowTiles ) );
        windowShadow->setWidget( widget );
        windowShadow->show();
        return;

    }

    //____________________________________________________________________________________
    void MdiWindowShadowFactory::removeShadow( QObject* object )
    {
        if( MdiWindowShadow* windowShadow = findShadow( object ) )
        {
            windowShadow->hide();
            windowShadow->deleteLater();
        }
    }

    //____________________________________________________________________________________
    void MdiWindowShadowFactory::widgetDestroyed( QObject* object )
    { _registeredWidgets.remove( object ); }

}
