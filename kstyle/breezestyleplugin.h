#ifndef breezestyleplugin_h
#define breezestyleplugin_h

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

#include <QStylePlugin>

namespace Breeze
{

    class StylePlugin : public QStylePlugin
    {

        Q_OBJECT

        #if QT_VERSION >= 0x050000
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "breeze.json" )
        #endif

        public:

        //! constructor
        StylePlugin(QObject *parent = 0):
            QStylePlugin(parent)
        {}

        //! destructor
        ~StylePlugin();

        //! returns list of valid keys
        QStringList keys() const;

        //! create style
        QStyle* create( const QString& );

    };

}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2( breeze-qt, Breeze::StylePlugin )
#endif

#endif
