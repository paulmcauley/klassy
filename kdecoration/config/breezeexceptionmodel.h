#ifndef breezeexceptionmodel_h
#define breezeexceptionmodel_h
//////////////////////////////////////////////////////////////////////////////
// breezeexceptionmodel.h
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

#include "breezelistmodel.h"
#include "breezesettings.h"
#include "breeze.h"

namespace Breeze
{

    //* qlistview for object counters
    class ExceptionModel: public ListModel<InternalSettingsPtr>
    {

        public:

        //* number of columns
        enum { nColumns = 3 };

        //* column type enumeration
        enum ColumnType {
            ColumnEnabled,
            ColumnType,
            ColumnRegExp
        };


        //*@name methods reimplemented from base class
        //@{

        //* return data for a given index
        QVariant data(const QModelIndex &index, int role) const override;

        //* header data
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        //* number of columns for a given index
        int columnCount(const QModelIndex& ) const override
        { return nColumns; }

        //@}

        protected:

        //* sort
        void privateSort( int, Qt::SortOrder ) override
        {}

        private:

        //* column titles
        static const QString m_columnTitles[ nColumns ];

    };

}
#endif
