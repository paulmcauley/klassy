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

#include "breezepalettehelper.h"

#include "breezehelper.h"
#include "breezepropertynames.h"
#include "breezestyleconfigdata.h"

#include <QAbstractScrollArea>
#include <QApplication>
#include <QDockWidget>
#include <QGroupBox>
#include <QMenu>
#include <QPalette>
#include <QTabWidget>
#include <QTextStream>
#include <QTimer>

namespace Breeze
{

    //_____________________________________________________
    PaletteHelper::PaletteHelper( QObject* parent, Helper& helper ):
        QObject( parent ),
        _helper( helper )
    {}

    //_____________________________________________________
    PaletteHelper::~PaletteHelper()
    { if( _widget ) _widget->deleteLater(); }

    //_____________________________________________________
    bool PaletteHelper::registerWidget( QWidget* widget )
    {
        if( _registeredWidgets.contains( widget ) ) return false;
        if( acceptWidget( widget ) )
        {

            if( !_widget )
            {
                _widget = new QWidget();
                _widget->installEventFilter( this );
            }

            _pendingWidgets.insert( widget );
            connect( widget, SIGNAL(destroyed(QObject*)), SLOT(unregisterWidget(QObject*)) );

            QTimer::singleShot( 0, this, SLOT(adjustPendingPalettes()) );

            return true;

        } else return false;

    }

    //_____________________________________________________
    void PaletteHelper::unregisterWidget( QObject* object )
    {
        _pendingWidgets.remove( object );
        _registeredWidgets.remove( object );
    }

    //_____________________________________________________
    bool PaletteHelper::eventFilter( QObject*, QEvent* event )
    {
        if( event->type() == QEvent::ApplicationPaletteChange )
        { adjustPalettes( qApp->palette() ); }

        return false;
    }


    //_____________________________________________________
    void PaletteHelper::adjustPendingPalettes( void )
    {
        foreach( QObject* object, _pendingWidgets )
        {
            adjustPalette( static_cast<QWidget*>( object ), qApp->palette() );
            _registeredWidgets.insert( object );
        }
        _pendingWidgets.clear();
    }

    //_____________________________________________________
    void PaletteHelper::adjustPalettes( const QPalette& palette )
    {
        foreach( QObject* object, _registeredWidgets )
        { adjustPalette( static_cast<QWidget*>( object ), palette ); }
    }

    //_____________________________________________________
    bool PaletteHelper::adjustPalette( QWidget* widget, const QPalette& palette ) const
    {

        // force side panels as flat, on option, and change font to not-bold
        QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>( widget );
        if( scrollArea &&
            !StyleConfigData::sidePanelDrawFrame() &&
            widget->property( PropertyNames::sidePanelView ).toBool() )
        {

            scrollArea->setPalette( _helper.sideViewPalette( palette ) );
            if( QWidget *viewport = scrollArea->viewport() )
            { viewport->setPalette( _helper.sideViewPalette( palette ) ); }

            return true;

        } else if( qobject_cast<QGroupBox*>( widget ) ||
            qobject_cast<QMenu*>( widget ) ||
            widget->inherits( "QComboBoxPrivateContainer" ) )
        {

            widget->setPalette( _helper.framePalette( palette ) );
            return true;

        } else if( QTabWidget *tabWidget = qobject_cast<QTabWidget*>( widget ) ) {

            if( !tabWidget->documentMode() )
            {
                widget->setPalette( _helper.framePalette( palette ) );
                return true;
            }

        } else if( qobject_cast<QDockWidget*>( widget ) && StyleConfigData::dockWidgetDrawFrame() ) {

            widget->setPalette( _helper.framePalette( palette ) );
            return true;

        }

        return false;

    }

    //_____________________________________________________
    bool PaletteHelper::acceptWidget( QWidget* widget ) const
    {

        // force side panels as flat, on option, and change font to not-bold
        QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>( widget );
        if( scrollArea &&
            !StyleConfigData::sidePanelDrawFrame() &&
            ( widget->inherits( "KDEPrivate::KPageListView" ) ||
            widget->inherits( "KDEPrivate::KPageTreeView" ) ||
            widget->property( PropertyNames::sidePanelView ).toBool() ) )
        {

            return true;

        } else if( qobject_cast<QGroupBox*>( widget ) ||
            qobject_cast<QMenu*>( widget ) ||
            widget->inherits( "QComboBoxPrivateContainer" ) )
        {

            return true;

        } else if( QTabWidget *tabWidget = qobject_cast<QTabWidget*>( widget ) ) {

            if( !tabWidget->documentMode() ) return true;

        } else if( qobject_cast<QDockWidget*>( widget ) && StyleConfigData::dockWidgetDrawFrame() ) {

            return true;

        }

        return false;

    }

}
