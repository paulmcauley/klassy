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

#include "breezewidgetexplorer.h"

#include "breeze.h"

#include <QTextStream>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

namespace Breeze
{

    //________________________________________________
    WidgetExplorer::WidgetExplorer( QObject* parent ):
        QObject( parent )
    {

        _eventTypes.insert( QEvent::Enter, QStringLiteral( "Enter" ) );
        _eventTypes.insert( QEvent::Leave, QStringLiteral( "Leave" ) );

        _eventTypes.insert( QEvent::HoverMove, QStringLiteral( "HoverMove" ) );
        _eventTypes.insert( QEvent::HoverEnter, QStringLiteral( "HoverEnter" ) );
        _eventTypes.insert( QEvent::HoverLeave, QStringLiteral( "HoverLeave" ) );

        _eventTypes.insert( QEvent::MouseMove, QStringLiteral( "MouseMove" ) );
        _eventTypes.insert( QEvent::MouseButtonPress, QStringLiteral( "MouseButtonPress" ) );
        _eventTypes.insert( QEvent::MouseButtonRelease, QStringLiteral( "MouseButtonRelease" ) );

        _eventTypes.insert( QEvent::FocusIn, QStringLiteral( "FocusIn" ) );
        _eventTypes.insert( QEvent::FocusOut, QStringLiteral( "FocusOut" ) );

        // _eventTypes.insert( QEvent::Paint, "Paint" );

    }

    //________________________________________________
    void WidgetExplorer::setEnabled( bool value )
    {
        if( value == _enabled ) return;
        _enabled = value;

        qApp->removeEventFilter( this );
        if( _enabled )  qApp->installEventFilter( this );
    }

    //________________________________________________
    bool WidgetExplorer::eventFilter( QObject* object, QEvent* event )
    {

//         if( object->isWidgetType() )
//         {
//             QString type( _eventTypes[event->type()] );
//             if( !type.isEmpty() )
//             {
//                 QTextStream( stdout ) << "Breeze::WidgetExplorer::eventFilter - widget: " << object << " (" << object->metaObject()->className() << ")";
//                 QTextStream( stdout ) << " type: " << type  << endl;
//             }
//         }

        switch( event->type() )
        {
            case QEvent::Paint:
            if( _drawWidgetRects )
            {
                QWidget* widget( qobject_cast<QWidget*>( object ) );
                if( !widget ) return false;

                QPainter painter( widget );
                painter.setRenderHints(QPainter::Antialiasing);
                painter.setBrush( Qt::NoBrush );
                painter.setPen( Qt::red );
                painter.drawRect( widget->rect() );
                painter.end();
            }
            break;

            case QEvent::MouseButtonPress:
            {

                // cast event and check button
                QMouseEvent* mouseEvent( static_cast<QMouseEvent*>( event ) );
                if( mouseEvent->button() != Qt::LeftButton ) break;

                // case widget and check (should not be necessary)
                QWidget* widget( qobject_cast<QWidget*>(object) );
                if( !widget ) return false;

                QTextStream( stdout )
                    << "Breeze::WidgetExplorer::eventFilter -"
                    << " event: " << event << " type: " << eventType( event->type() )
                    << " widget: " << widgetInformation( widget )
                    << endl;

                // print parent information
                QWidget* parent( widget->parentWidget() );
                while( parent )
                {
                    QTextStream( stdout ) << "    parent: " << widgetInformation( parent ) << endl;
                    parent = parent->parentWidget();
                }
                QTextStream( stdout ) << "" << endl;

            }
            break;

            default: break;

        }

        // always return false to go on with normal chain
        return false;

    }

    //________________________________________________
    QString WidgetExplorer::eventType( const QEvent::Type& type ) const
    {
        switch( type )
        {
            case QEvent::MouseButtonPress: return QStringLiteral( "MouseButtonPress" );
            case QEvent::MouseButtonRelease: return QStringLiteral( "MouseButtonRelease" );
            case QEvent::MouseMove: return QStringLiteral( "MouseMove" );
            default: return QStringLiteral( "Unknown" );
        }
    }

    //________________________________________________
    QString WidgetExplorer::widgetInformation( const QWidget* widget ) const
    {

        QRect r( widget->geometry() );
        const char* className( widget->metaObject()->className() );
        QString out;
        QTextStream( &out ) << widget << " (" << className << ")"
            << " position: " << r.x() << "," << r.y()
            << " size: " << r.width() << "," << r.height()
            << " sizeHint: " << widget->sizeHint().width() << "," << widget->sizeHint().height()
            << " minimumSizeHint: " << widget->minimumSizeHint().width() << "," << widget->minimumSizeHint().height()
            << " hover: " << widget->testAttribute( Qt::WA_Hover );

        return out;
    }

}
