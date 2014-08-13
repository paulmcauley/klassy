
//////////////////////////////////////////////////////////////////////////////
// breezestyleplugin.h
// Breeze widget style for KDE Frameworks
// -------------------
//
// Copyright (C) 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License version 2 as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; see the file COPYING.LIB.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.
//////////////////////////////////////////////////////////////////////////////

#include "breezestyleplugin.h"
#include "breezestyle.h"

#include <QApplication>

namespace Breeze
{

    //_________________________________________________
    QStyle* Breeze::StylePlugin::create( const QString &key )
    {
        if( key.toLower() == QStringLiteral( "breeze" ) ) return new Style();
        else return nullptr;
    }

    //_________________________________________________
    Breeze::StylePlugin::~StylePlugin()
    {
        // Delete style when using ::exit() otherwise it'll outlive the unloaded plugin and we'll get a crash
        if (qApp) delete qApp->style();
    }

}
