
//////////////////////////////////////////////////////////////////////////////
// breezedetectwidget.cpp
// Note: this class is a stripped down version of
// /kdebase/workspace/kwin/kcmkwin/kwinrules/detectwidget.cpp
// Copyright (c) 2004 Lubos Lunak <l.lunak@kde.org>
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
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

#include "breezedetectwidget.h"

#include "breeze.h"

#include <KWindowInfo>

#include <QPushButton>
#include <QMouseEvent>
#include <config-breeze.h>
#if BREEZE_HAVE_X11
#include <QX11Info>
#include <xcb/xcb.h>
#endif

namespace Breeze
{

    //_________________________________________________________
    DetectDialog::DetectDialog( QWidget* parent ):
        QDialog( parent )
    {

        // setup
        m_ui.setupUi( this );

        connect( m_ui.buttonBox->button( QDialogButtonBox::Cancel ), &QAbstractButton::clicked, this, &QWidget::close );
        m_ui.windowClassCheckBox->setChecked( true );

#if BREEZE_HAVE_X11
        if (QX11Info::isPlatformX11()) {
            // create atom
            xcb_connection_t* connection( QX11Info::connection() );
            const QString atomName( QStringLiteral( "WM_STATE" ) );
            xcb_intern_atom_cookie_t cookie( xcb_intern_atom( connection, false, atomName.size(), qPrintable( atomName ) ) );
            QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> reply( xcb_intern_atom_reply( connection, cookie, nullptr) );
            m_wmStateAtom = reply ? reply->atom : 0;
        }
#endif

    }

    //_________________________________________________________
    void DetectDialog::detect(  WId window )
    {
        if( window == 0 ) selectWindow();
        else readWindow( window );
    }

    //_________________________________________________________
    void DetectDialog::readWindow( WId window )
    {

        if( window == 0 )
        {
            emit detectionDone( false );
            return;
        }

        m_info.reset(new KWindowInfo( window, NET::WMAllProperties, NET::WM2AllProperties ));
        if( !m_info->valid())
        {
            emit detectionDone( false );
            return;
        }

        const QString wmClassClass( QString::fromUtf8( m_info->windowClassClass() ) );
        const QString wmClassName( QString::fromUtf8( m_info->windowClassName() ) );

        m_ui.windowClass->setText( QStringLiteral( "%1 (%2 %3)" ).arg( wmClassClass ).arg( wmClassName ).arg( wmClassClass ) );
        m_ui.windowTitle->setText( m_info->name() );
        emit detectionDone( exec() == QDialog::Accepted );

    }

    //_________________________________________________________
    void DetectDialog::selectWindow()
    {

        // use a dialog, so that all user input is blocked
        // use WX11BypassWM and moving away so that it's not actually visible
        // grab only mouse, so that keyboard can be used e.g. for switching windows
        m_grabber = new QDialog( nullptr, Qt::X11BypassWindowManagerHint );
        m_grabber->move( -1000, -1000 );
        m_grabber->setModal( true );
        m_grabber->show();

        // need to explicitly override cursor for Qt5
        qApp->setOverrideCursor( Qt::CrossCursor );
        m_grabber->grabMouse( Qt::CrossCursor );
        m_grabber->installEventFilter( this );

    }

    //_________________________________________________________
    bool DetectDialog::eventFilter( QObject* o, QEvent* e )
    {
        // check object and event type
        if( o != m_grabber ) return false;
        if( e->type() != QEvent::MouseButtonRelease ) return false;

        // need to explicitly release cursor for Qt5
        qApp->restoreOverrideCursor();

        // delete old m_grabber
        delete m_grabber;
        m_grabber = nullptr;

        // check button
        if( static_cast< QMouseEvent* >( e )->button() != Qt::LeftButton ) return true;

        // read window information
        readWindow( findWindow() );

        return true;
    }

    //_________________________________________________________
    WId DetectDialog::findWindow()
    {

        #if BREEZE_HAVE_X11
        if (!QX11Info::isPlatformX11()) {
            return 0;
        }
        // check atom
        if( !m_wmStateAtom ) return 0;

        xcb_connection_t* connection( QX11Info::connection() );
        xcb_window_t parent( QX11Info::appRootWindow() );

        // why is there a loop of only 10 here
        for( int i = 0; i < 10; ++i )
        {

            // query pointer
            xcb_query_pointer_cookie_t pointerCookie( xcb_query_pointer( connection, parent ) );
            QScopedPointer<xcb_query_pointer_reply_t, QScopedPointerPodDeleter> pointerReply( xcb_query_pointer_reply( connection, pointerCookie, nullptr ) );
            if( !( pointerReply && pointerReply->child ) ) return 0;

            const xcb_window_t child( pointerReply->child );
            xcb_get_property_cookie_t cookie( xcb_get_property( connection, 0, child, m_wmStateAtom, XCB_GET_PROPERTY_TYPE_ANY, 0, 0 ) );
            QScopedPointer<xcb_get_property_reply_t, QScopedPointerPodDeleter> reply( xcb_get_property_reply( connection, cookie, nullptr ) );
            if( reply  && reply->type ) return child;
            else parent = child;

        }
        #endif

        return 0;

    }

}
