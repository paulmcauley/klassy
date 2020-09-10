//////////////////////////////////////////////////////////////////////////////
// itemmodel.cpp
// -------------------
//
// SPDX-FileCopyrightText: 2009-2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
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
