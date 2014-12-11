//////////////////////////////////////////////////////////////////////////////
// itemmodel.cpp
// -------------------
//
// Copyright (c) 2009-2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
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

#include "breezeitemmodel.h"

namespace Breeze
{

    //_______________________________________________________________
    ItemModel::ItemModel( QObject* parent ):
        QAbstractItemModel( parent )
    {}

    //____________________________________________________________
    void ItemModel::sort( int column, Qt::SortOrder order )
    {

        // store column and order
        m_sortColumn = column;
        m_sortOrder = order;

        // emit signals and call private methods
        emit layoutAboutToBeChanged();
        privateSort( column, order );
        emit layoutChanged();

    }

    //____________________________________________________________
    QModelIndexList ItemModel::indexes( int column, const QModelIndex& parent ) const
    {
        QModelIndexList out;
        int rows( rowCount( parent ) );
        for( int row = 0; row < rows; row++ )
        {
            QModelIndex index( this->index( row, column, parent ) );
            if( !index.isValid() ) continue;
            out.append( index );
            out += indexes( column, index );
        }

        return out;

    }

}
