//////////////////////////////////////////////////////////////////////////////
// breezeexceptionmodel.cpp
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

#include "breezeexceptionmodel.h"

#include <KLocalizedString>

namespace Breeze
{

    //_______________________________________________
    const QString ExceptionModel::m_columnTitles[ ExceptionModel::nColumns ] =
    {
        QStringLiteral( "" ),
        i18n("Exception Type"),
        i18n("Regular Expression")
    };

    //__________________________________________________________________
    QVariant ExceptionModel::data( const QModelIndex& index, int role ) const
    {

        // check index, role and column
        if( !index.isValid() ) return QVariant();

        // retrieve associated file info
        const InternalSettingsPtr& configuration( get(index) );

        // return text associated to file and column
        if( role == Qt::DisplayRole )
        {

            switch( index.column() )
            {
                case ColumnType:
                {
                    switch( configuration->exceptionType() )
                    {

                        case InternalSettings::ExceptionWindowTitle:
                        return i18n( "Window Title" );

                        default:
                        case InternalSettings::ExceptionWindowClassName:
                        return i18n( "Window Class Name" );
                    }

                }

                case ColumnRegExp: return configuration->exceptionPattern();
                default: return QVariant();
                break;
            }

        } else if( role == Qt::CheckStateRole &&  index.column() == ColumnEnabled ) {

            return configuration->enabled() ? Qt::Checked : Qt::Unchecked;

        } else if( role == Qt::ToolTipRole &&  index.column() == ColumnEnabled ) {

            return i18n("Enable/disable this exception");

        }


        return QVariant();
    }

    //__________________________________________________________________
    QVariant ExceptionModel::headerData(int section, Qt::Orientation orientation, int role) const
    {

        if(
            orientation == Qt::Horizontal &&
            role == Qt::DisplayRole &&
            section >= 0 &&
            section < nColumns )
        { return m_columnTitles[section]; }

        // return empty
        return QVariant();

    }

}
