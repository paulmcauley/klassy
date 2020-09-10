#ifndef ItemModel_h
#define ItemModel_h

//////////////////////////////////////////////////////////////////////////////
// itemmodel.h
// -------------------
//
// SPDX-FileCopyrightText: 2009-2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include <QAbstractItemModel>

namespace Breeze
{

    //* Job model. Stores job information for display in lists
    class ItemModel : public QAbstractItemModel
    {

        public:

        //* constructor
        explicit ItemModel(QObject *parent = nullptr);

        //* destructor
        virtual ~ItemModel()
        {}

        //* return all indexes in model starting from parent [recursive]
        QModelIndexList indexes( int column = 0, const QModelIndex& parent = QModelIndex() ) const;

        //*@name sorting
        //@{

        //* sort
        virtual void sort()
        { sort( sortColumn(), sortOrder() ); }

        //* sort
        void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) override;

        //* current sorting column
        const int& sortColumn() const
        { return m_sortColumn; }

        //* current sort order
        const Qt::SortOrder& sortOrder() const
        { return m_sortOrder; }

        //@}

        protected:

        //* this sort columns without calling the layout changed callbacks
        void privateSort()
        { privateSort( m_sortColumn, m_sortOrder ); }

        //* private sort, with no signals emitted
        virtual void privateSort( int column, Qt::SortOrder order ) = 0;

        //* used to sort items in list
        class SortFTor
        {

            public:

            //* constructor
            explicit SortFTor( const int& type, Qt::SortOrder order = Qt::AscendingOrder ):
                _type( type ),
                _order( order )
            {}

            protected:

            //* column
            int _type;

            //* order
            Qt::SortOrder _order;

        };

        private:

        //* sorting column
        int m_sortColumn = 0;

        //* sorting order
        Qt::SortOrder m_sortOrder = Qt::AscendingOrder;

    };

}

#endif
