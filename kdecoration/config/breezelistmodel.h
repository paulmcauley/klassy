#ifndef ListModel_h
#define ListModel_h
//////////////////////////////////////////////////////////////////////////////
// listmodel.h
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

#include "breezeitemmodel.h"

#include <QSet>
#include <QList>

#include <algorithm>

namespace Breeze
{
    //! Job model. Stores job information for display in lists
    template<class T> class ListModel : public ItemModel
    {

        public:

        //! value type
        typedef T ValueType;

        //! reference
        typedef T& Reference;

        //! pointer
        typedef T* Pointer;

        //! value list and iterators
        typedef QList<ValueType> List;
        typedef QListIterator<ValueType> ListIterator;
        typedef QMutableListIterator<ValueType> MutableListIterator;

        //! list of vector
        // typedef QSet<ValueType> Set;

        //! constructor
        ListModel(QObject *parent = 0):
            ItemModel( parent )
        {}

        //! destructor
        virtual ~ListModel()
        {}

        //!@name methods reimplemented from base class
        //@{

        //! flags
        Qt::ItemFlags flags(const QModelIndex &index) const override
        {
            if (!index.isValid()) return 0;
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }

        //! unique index for given row, column and parent index
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
        {

            // check if index is valid
            if( !hasIndex( row, column, parent ) ) return QModelIndex();

            // return invalid index if parent is valid
            if( parent.isValid() ) return QModelIndex();

            // check against _values
            return ( row < (int) _values.size() ) ? createIndex( row, column ):QModelIndex();

        }

        //! index of parent
        QModelIndex parent(const QModelIndex &) const override
        { return QModelIndex(); }

        //! number of rows below given index
        int rowCount(const QModelIndex &parent = QModelIndex()) const override
        { return parent.isValid() ? 0:_values.size(); }

        //@}

        //!@name selection
        //@{

        //! clear internal list selected items
        virtual void clearSelectedIndexes( void )
        { _selection.clear(); }

        //! store index internal selection state
        virtual void setIndexSelected( const QModelIndex& index, bool value )
        {
            if( value ) _selection.push_back( get(index) );
            else _selection.erase( std::remove( _selection.begin(), _selection.end(), get(index) ), _selection.end() );
        }

        //! get list of internal selected items
        virtual QModelIndexList selectedIndexes( void ) const
        {

            QModelIndexList out;
            for( typename List::const_iterator iter = _selection.begin(); iter != _selection.end(); iter++ )
            {
                QModelIndex index( ListModel::index( *iter ) );
                if( index.isValid() ) out.push_back( index );
            }
            return out;

        }

        //@}

        //!@name interface
        //@{

        //! add value
        virtual void add( const ValueType& value )
        {

            emit layoutAboutToBeChanged();
            _add( value );
            privateSort();
            emit layoutChanged();

        }

        //! add values
        virtual void add( const List& values )
        {

            // check if not empty
            // this avoids sending useless signals
            if( values.empty() ) return;

            emit layoutAboutToBeChanged();

            for( typename List::const_iterator iter = values.begin(); iter != values.end(); iter++ )
            { _add( *iter ); }

            privateSort();
            emit layoutChanged();

        }


        //! insert values
        virtual void insert( const QModelIndex& index, const ValueType& value )
        {
            emit layoutAboutToBeChanged();
            _insert( index, value );
            emit layoutChanged();
        }

        //! insert values
        virtual void insert( const QModelIndex& index, const List& values )
        {
            emit layoutAboutToBeChanged();

            // need to loop in reverse order so that the "values" ordering is preserved
            ListIterator iter( values );
            iter.toBack();
            while( iter.hasPrevious() )
            { _insert( index, iter.previous() ); }

            emit layoutChanged();

        }

        //! insert values
        virtual void replace( const QModelIndex& index, const ValueType& value )
        {
            if( !index.isValid() ) add( value );
            else {
                emit layoutAboutToBeChanged();
                setIndexSelected( index, false );
                _values[index.row()] = value;
                setIndexSelected( index, true );
                emit layoutChanged();
            }
        }

        //! remove
        virtual void remove( const ValueType& value )
        {

            emit layoutAboutToBeChanged();
            _remove( value );
            emit layoutChanged();
            return;

        }

        //! remove
        virtual void remove( const List& values )
        {

            // check if not empty
            // this avoids sending useless signals
            if( values.empty() ) return;

            emit layoutAboutToBeChanged();
            for( typename List::const_iterator iter = values.begin(); iter != values.end(); iter++ )
            { _remove( *iter ); }
            emit layoutChanged();
            return;

        }

        //! clear
        virtual void clear( void )
        { set( List() ); }

        //! update values from list
        /*!
        values that are not found in current are removed
        new values are set to the end.
        This is slower than the "set" method, but the selection is not cleared in the process
        */
        virtual void update( List values )
        {

            emit layoutAboutToBeChanged();

            // store values to be removed
            List removed_values;

            // update values that are common to both lists
            for( typename List::iterator iter = _values.begin(); iter != _values.end(); iter++ )
            {

                // see if iterator is in list
                typename List::iterator found_iter( std::find( values.begin(), values.end(), *iter ) );
                if( found_iter == values.end() ) removed_values.push_back( *iter );
                else {
                    *iter = *found_iter;
                    values.erase( found_iter );
                }

            }

            // remove values that have not been found in new list
            for( typename List::const_iterator iter = removed_values.constBegin(); iter != removed_values.constEnd(); iter++ )
            { _remove( *iter ); }

            // add remaining values
            for( typename List::const_iterator iter = values.constBegin(); iter != values.constEnd(); iter++ )
            { _add( *iter ); }

            privateSort();
            emit layoutChanged();

        }

        //! set all values
        virtual void set( const List& values )
        {

            emit layoutAboutToBeChanged();
            _values = values;
            _selection.clear();
            privateSort();
            emit layoutChanged();

            return;
        }

        //! return all values
        const List& get( void ) const
        { return _values; }

        //! return value for given index
        virtual ValueType get( const QModelIndex& index ) const
        { return (index.isValid() && index.row() < int(_values.size()) ) ? _values[index.row()]:ValueType(); }

        //! return value for given index
        virtual ValueType& get( const QModelIndex& index )
        {
            Q_ASSERT( index.isValid() && index.row() < int( _values.size() ) );
            return _values[index.row()];
        }

        //! return all values
        List get( const QModelIndexList& indexes ) const
        {
            List out;
            for( QModelIndexList::const_iterator iter = indexes.begin(); iter != indexes.end(); iter++ )
            { if( iter->isValid() && iter->row() < int(_values.size()) ) out.push_back( get( *iter ) ); }
            return out;
        }

        //! return index associated to a given value
        virtual QModelIndex index( const ValueType& value, int column = 0 ) const
        {
            for( int row = 0; row < _values.size(); ++row )
            { if( value == _values[row] ) return index( row, column ); }
            return QModelIndex();
        }

        //@}

        //! return true if model contains given index
        virtual bool contains( const QModelIndex& index ) const
        { return index.isValid() && index.row() < _values.size(); }

        protected:

        //! return all values
        List& _get( void )
        { return _values; }

        //! add, without update
        virtual void _add( const ValueType& value )
        {
            typename List::iterator iter = std::find( _values.begin(), _values.end(), value );
            if( iter == _values.end() ) _values.push_back( value );
            else *iter = value;
        }

        //! add, without update
        virtual void _insert( const QModelIndex& index, const ValueType& value )
        {
            if( !index.isValid() ) add( value );
            int row = 0;
            typename List::iterator iter( _values.begin() );
            for( ;iter != _values.end() && row != index.row(); iter++, row++ )
            {}

            _values.insert( iter, value );
        }

        //! remove, without update
        virtual void _remove( const ValueType& value )
        {
            _values.erase( std::remove( _values.begin(), _values.end(), value ), _values.end() );
            _selection.erase( std::remove( _selection.begin(), _selection.end(), value ), _selection.end() );
        }

        private:

        //! values
        List _values;

        //! selection
        List _selection;

    };
}
#endif
