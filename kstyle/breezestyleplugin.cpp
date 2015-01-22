
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

#include "breezestyleplugin.h"
#include "breezestyleplugin.moc"
#include "breezestyle.h"

#include <QApplication>

namespace Breeze
{

    //_________________________________________________
    QStyle* StylePlugin::create( const QString &key )
    {
        if( key.toLower() == QStringLiteral( "breeze" ) )
        {
            QStyle* style = new Style();
            #if QT_VERSION >= 0x050000
            // Delete style when using ::exit() otherwise it'll outlive the unloaded plugin and we'll get a crash
            // (see 2ffe20e1bfe93c921c5372b4d21447b1de308d4b in oxygen repo)
            connect(this, &QObject::destroyed, style, [style]() {
                delete style;
            });
            #endif
            return style;
        }
        return nullptr;
    }

    //_________________________________________________
    StylePlugin::~StylePlugin()
    {
    }

    //_________________________________________________
    QStringList StylePlugin::keys() const
    { return QStringList( QStringLiteral( "Breeze" ) ); }

}
