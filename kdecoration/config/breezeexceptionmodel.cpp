//////////////////////////////////////////////////////////////////////////////
// breezeexceptionmodel.cpp
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezeexceptionmodel.h"

#include <KLocalizedString>

namespace Breeze
{

//_______________________________________________
const QString ExceptionModel::m_columnTitles[ExceptionModel::nColumns] = {QStringLiteral(""),
                                                                          i18n("Window Property Regular Expression"),
                                                                          i18n("Application Name Regular Expression")};

//__________________________________________________________________
QVariant ExceptionModel::data(const QModelIndex &index, int role) const
{
    // check index, role and column
    if (!index.isValid())
        return QVariant();

    // retrieve associated file info
    const InternalSettingsPtr &configuration(get(index));

    // return text associated to file and column
    if (role == Qt::DisplayRole) {
        switch (index.column()) {

        case ColumnWindowPropertyRegExp:
            return configuration->exceptionWindowPropertyPattern();

        case ColumnProgramNameRegExp:
            return configuration->exceptionProgramNamePattern();

        default:
            return QVariant();
            break;
        }

    } else if (role == Qt::CheckStateRole && index.column() == ColumnEnabled) {
        return configuration->enabled() ? Qt::Checked : Qt::Unchecked;

    } else if (role == Qt::ToolTipRole && index.column() == ColumnEnabled) {
        return i18n("Enable/disable this exception");
    }

    return QVariant();
}

//__________________________________________________________________
QVariant ExceptionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < nColumns) {
        return m_columnTitles[section];
    }

    // return empty
    return QVariant();
}
}
