//////////////////////////////////////////////////////////////////////////////
// breezeexceptionlist.cpp
// window decoration exceptions
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezeexceptionlist.h"


namespace Breeze
{

    //______________________________________________________________
    void ExceptionList::readConfig( KSharedConfig::Ptr config )
    {

        _exceptions.clear();

        QString groupName;
        for( int index = 0; config->hasGroup( groupName = exceptionGroupName( index ) ); ++index )
        {

            // create exception
            InternalSettings exception;

            // reset group
            readConfig( &exception, config.data(), groupName );

            // create new configuration
            InternalSettingsPtr configuration( new InternalSettings() );
            configuration.data()->load();

            // apply changes from exception
            configuration->setEnabled( exception.enabled() );
            configuration->setExceptionType( exception.exceptionType() );
            configuration->setExceptionPattern( exception.exceptionPattern() );
            configuration->setMask( exception.mask() );

            // propagate all features found in mask to the output configuration
            if( exception.mask() & BorderSize ) configuration->setBorderSize( exception.borderSize() );
            configuration->setHideTitleBar( exception.hideTitleBar() );

            // append to exceptions
            _exceptions.append( configuration );

        }

    }

    //______________________________________________________________
    void ExceptionList::writeConfig( KSharedConfig::Ptr config )
    {

        // remove all existing exceptions
        QString groupName;
        for( int index = 0; config->hasGroup( groupName = exceptionGroupName( index ) ); ++index )
        { config->deleteGroup( groupName ); }

        // rewrite current exceptions
        int index = 0;
        foreach( const InternalSettingsPtr& exception, _exceptions )
        {

            writeConfig( exception.data(), config.data(), exceptionGroupName( index ) );
            ++index;

        }

    }

    //_______________________________________________________________________
    QString ExceptionList::exceptionGroupName( int index )
    { return QString( "Windeco Exception %1" ).arg( index ); }

    //______________________________________________________________
    void ExceptionList::writeConfig( KCoreConfigSkeleton* skeleton, KConfig* config, const QString& groupName )
    {

        // list of items to be written
        QStringList keys = { "Enabled", "ExceptionPattern", "ExceptionType", "HideTitleBar", "Mask", "BorderSize"};

        // write all items
        foreach( auto key, keys )
        {
            KConfigSkeletonItem* item( skeleton->findItem( key ) );
            if( !item ) continue;

            if( !groupName.isEmpty() ) item->setGroup( groupName );
            KConfigGroup configGroup( config, item->group() );
            configGroup.writeEntry( item->key(), item->property() );

        }

    }

    //______________________________________________________________
    void ExceptionList::readConfig( KCoreConfigSkeleton* skeleton, KConfig* config, const QString& groupName )
    {

        foreach( KConfigSkeletonItem* item, skeleton->items() )
        {
            if( !groupName.isEmpty() ) item->setGroup( groupName );
            item->readConfig( config );
        }

    }

}
