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

namespace Breeze
{

    //_____________________________________________________
    PaletteHelper::PaletteHelper( QObject* parent, Helper& helper ):
        QObject( parent ),
        _helper( helper )
    {
        _widget = new QWidget();
        _widget->installEventFilter( this );
    }

    //_____________________________________________________
    PaletteHelper::~PaletteHelper()
    { delete _widget; }


    //_____________________________________________________
    bool PaletteHelper::registerWidget( QWidget* widget )
    {
        if( adjustPalette( widget, QApplication::palette() ) )
        {
            _registeredWidgets.insert( widget );
            connect( widget, SIGNAL(destroyed(QObject*)), SLOT(unregisterWidget(QObject*)) );
            return true;

        } else return false;

    }

    //_____________________________________________________
    void PaletteHelper::unregisterWidget( QObject* object )
    { _registeredWidgets.remove( object ); }

    //_____________________________________________________
    bool PaletteHelper::eventFilter( QObject*, QEvent* event )
    {
        if( event->type() == QEvent::ApplicationPaletteChange )
        { adjustPalettes( qApp->palette() ); }

        return false;
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
            ( widget->inherits( "KDEPrivate::KPageListView" ) ||
            widget->inherits( "KDEPrivate::KPageTreeView" ) ||
            widget->property( PropertyNames::sidePanelView ).toBool() ) )
        {

            scrollArea->setPalette( _helper.sideViewPalette( palette ) );
            scrollArea->setProperty( PropertyNames::sidePanelView, true );

            if( QWidget *viewport = scrollArea->viewport() )
            {

                viewport->setPalette( _helper.sideViewPalette( palette ) );
                return true;
            }


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

}
