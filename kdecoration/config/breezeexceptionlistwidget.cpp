//////////////////////////////////////////////////////////////////////////////
// breezeexceptionlistwidget.cpp
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

#include "breezeexceptionlistwidget.h"
#include "breezeexceptiondialog.h"

#include <KLocalizedString>

#include <QMessageBox>
#include <QPointer>
#include <QIcon>

//__________________________________________________________
namespace Breeze
{

    //__________________________________________________________
    ExceptionListWidget::ExceptionListWidget( QWidget* parent ):
        QWidget( parent )
    {

        // ui
        m_ui.setupUi( this );

        // list
        m_ui.exceptionListView->setAllColumnsShowFocus( true );
        m_ui.exceptionListView->setRootIsDecorated( false );
        m_ui.exceptionListView->setSortingEnabled( false );
        m_ui.exceptionListView->setModel( &model() );
        m_ui.exceptionListView->sortByColumn( ExceptionModel::ColumnType, Qt::AscendingOrder );
        m_ui.exceptionListView->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Ignored ) );

        m_ui.moveUpButton->setIcon( QIcon::fromTheme( QStringLiteral( "arrow-up" ) ) );
        m_ui.moveDownButton->setIcon( QIcon::fromTheme( QStringLiteral( "arrow-down" ) ) );
        m_ui.addButton->setIcon( QIcon::fromTheme( QStringLiteral( "list-add" ) ) );
        m_ui.removeButton->setIcon( QIcon::fromTheme( QStringLiteral( "list-remove" ) ) );
        m_ui.editButton->setIcon( QIcon::fromTheme( QStringLiteral( "edit-rename" ) ) );

        connect( m_ui.addButton, &QAbstractButton::clicked, this, &ExceptionListWidget::add );
        connect( m_ui.editButton, &QAbstractButton::clicked, this, &ExceptionListWidget::edit );
        connect( m_ui.removeButton, &QAbstractButton::clicked, this, &ExceptionListWidget::remove );
        connect( m_ui.moveUpButton, &QAbstractButton::clicked, this, &ExceptionListWidget::up );
        connect( m_ui.moveDownButton, &QAbstractButton::clicked, this, &ExceptionListWidget::down );

        connect( m_ui.exceptionListView, &QAbstractItemView::activated, this, &ExceptionListWidget::edit );
        connect( m_ui.exceptionListView, &QAbstractItemView::clicked, this, &ExceptionListWidget::toggle );
        connect( m_ui.exceptionListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ExceptionListWidget::updateButtons );

        updateButtons();
        resizeColumns();


    }

    //__________________________________________________________
    void ExceptionListWidget::setExceptions( const InternalSettingsList& exceptions )
    {
        model().set( exceptions );
        resizeColumns();
        setChanged( false );
    }

    //__________________________________________________________
    InternalSettingsList ExceptionListWidget::exceptions()
    {
        return model().get();
        setChanged( false );
    }

    //__________________________________________________________
    void ExceptionListWidget::updateButtons()
    {

        bool hasSelection( !m_ui.exceptionListView->selectionModel()->selectedRows().empty() );
        m_ui.removeButton->setEnabled( hasSelection );
        m_ui.editButton->setEnabled( hasSelection );

        m_ui.moveUpButton->setEnabled( hasSelection && !m_ui.exceptionListView->selectionModel()->isRowSelected( 0, QModelIndex() ) );
        m_ui.moveDownButton->setEnabled( hasSelection && !m_ui.exceptionListView->selectionModel()->isRowSelected( model().rowCount()-1, QModelIndex() ) );

    }


    //_______________________________________________________
    void ExceptionListWidget::add()
    {


        QPointer<ExceptionDialog> dialog = new ExceptionDialog( this );
        dialog->setWindowTitle( i18n( "New Exception - Breeze Settings" ) );
        InternalSettingsPtr exception( new InternalSettings() );

        exception->load();

        dialog->setException( exception );

        // run dialog and check existence
        if( !dialog->exec() )
        {
            delete dialog;
            return;
        }

        dialog->save();
        delete dialog;

        // check exceptions
        if( !checkException( exception ) ) return;

        // create new item
        model().add( exception );
        setChanged( true );

        // make sure item is selected
        QModelIndex index( model().index( exception ) );
        if( index != m_ui.exceptionListView->selectionModel()->currentIndex() )
        {
            m_ui.exceptionListView->selectionModel()->select( index,  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
            m_ui.exceptionListView->selectionModel()->setCurrentIndex( index,  QItemSelectionModel::Current|QItemSelectionModel::Rows );
        }

        resizeColumns();

    }

    //_______________________________________________________
    void ExceptionListWidget::edit()
    {

        // retrieve selection
        QModelIndex current( m_ui.exceptionListView->selectionModel()->currentIndex() );
        if( ! model().contains( current ) ) return;

        InternalSettingsPtr exception( model().get( current ) );

        // create dialog
        QPointer<ExceptionDialog> dialog( new ExceptionDialog( this ) );
        dialog->setWindowTitle( i18n( "Edit Exception - Breeze Settings" ) );
        dialog->setException( exception );

        // map dialog
        if( !dialog->exec() )
        {
            delete dialog;
            return;
        }

        // check modifications
        if( !dialog->isChanged() ) return;

        // retrieve exception
        dialog->save();
        delete dialog;

        // check new exception validity
        checkException( exception );
        resizeColumns();

        setChanged( true );

    }

    //_______________________________________________________
    void ExceptionListWidget::remove()
    {

        // confirmation dialog
        {
            QMessageBox messageBox( QMessageBox::Question, i18n("Question - Breeze Settings" ), i18n("Remove selected exception?"), QMessageBox::Yes | QMessageBox::Cancel );
            messageBox.button( QMessageBox::Yes )->setText( i18n("Remove") );
            messageBox.setDefaultButton( QMessageBox::Cancel );
            if( messageBox.exec() == QMessageBox::Cancel ) return;
        }

        // remove
        model().remove( model().get( m_ui.exceptionListView->selectionModel()->selectedRows() ) );
        resizeColumns();
        updateButtons();

        setChanged( true );

    }

    //_______________________________________________________
    void ExceptionListWidget::toggle( const QModelIndex& index )
    {

        if( !model().contains( index ) ) return;
        if( index.column() != ExceptionModel::ColumnEnabled ) return;

        // get matching exception
        InternalSettingsPtr exception( model().get( index ) );
        exception->setEnabled( !exception->enabled() );
        setChanged( true );

    }

    //_______________________________________________________
    void ExceptionListWidget::up()
    {

        InternalSettingsList selection( model().get( m_ui.exceptionListView->selectionModel()->selectedRows() ) );
        if( selection.empty() ) { return; }

        // retrieve selected indexes in list and store in model
        QModelIndexList selectedIndices( m_ui.exceptionListView->selectionModel()->selectedRows() );
        InternalSettingsList selectedExceptions( model().get( selectedIndices ) );

        InternalSettingsList currentException( model().get() );
        InternalSettingsList newExceptions;

        for( InternalSettingsList::const_iterator iter = currentException.constBegin(); iter != currentException.constEnd(); ++iter )
        {

            // check if new list is not empty, current index is selected and last index is not.
            // if yes, move.
            if(
                !( newExceptions.empty() ||
                selectedIndices.indexOf( model().index( *iter ) ) == -1 ||
                selectedIndices.indexOf( model().index( newExceptions.back() ) ) != -1
                ) )
            {
                InternalSettingsPtr last( newExceptions.back() );
                newExceptions.removeLast();
                newExceptions.append( *iter );
                newExceptions.append( last );
            } else newExceptions.append( *iter );

        }

        model().set( newExceptions );

        // restore selection
        m_ui.exceptionListView->selectionModel()->select( model().index( selectedExceptions.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
        for( InternalSettingsList::const_iterator iter = selectedExceptions.constBegin(); iter != selectedExceptions.constEnd(); ++iter )
        { m_ui.exceptionListView->selectionModel()->select( model().index( *iter ), QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

        setChanged( true );

    }

    //_______________________________________________________
    void ExceptionListWidget::down()
    {

        InternalSettingsList selection( model().get( m_ui.exceptionListView->selectionModel()->selectedRows() ) );
        if( selection.empty() )
        { return; }

        // retrieve selected indexes in list and store in model
        QModelIndexList selectedIndices( m_ui.exceptionListView->selectionModel()->selectedIndexes() );
        InternalSettingsList selectedExceptions( model().get( selectedIndices ) );

        InternalSettingsList currentExceptions( model().get() );
        InternalSettingsList newExceptions;

        InternalSettingsListIterator iter( currentExceptions );
        iter.toBack();
        while( iter.hasPrevious() )
        {

            InternalSettingsPtr current( iter.previous() );

            // check if new list is not empty, current index is selected and last index is not.
            // if yes, move.
            if(
                !( newExceptions.empty() ||
                selectedIndices.indexOf( model().index( current ) ) == -1 ||
                selectedIndices.indexOf( model().index( newExceptions.front() ) ) != -1
                ) )
            {

                InternalSettingsPtr first( newExceptions.front() );
                newExceptions.removeFirst();
                newExceptions.prepend( current );
                newExceptions.prepend( first );

            } else newExceptions.prepend( current );
        }

        model().set( newExceptions );

        // restore selection
        m_ui.exceptionListView->selectionModel()->select( model().index( selectedExceptions.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
        for( InternalSettingsList::const_iterator iter = selectedExceptions.constBegin(); iter != selectedExceptions.constEnd(); ++iter )
        { m_ui.exceptionListView->selectionModel()->select( model().index( *iter ), QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

        setChanged( true );

    }

    //_______________________________________________________
    void ExceptionListWidget::resizeColumns() const
    {
        m_ui.exceptionListView->resizeColumnToContents( ExceptionModel::ColumnEnabled );
        m_ui.exceptionListView->resizeColumnToContents( ExceptionModel::ColumnType );
        m_ui.exceptionListView->resizeColumnToContents( ExceptionModel::ColumnRegExp );
    }

    //_______________________________________________________
    bool ExceptionListWidget::checkException( InternalSettingsPtr exception )
    {

        while( exception->exceptionPattern().isEmpty() || !QRegExp( exception->exceptionPattern() ).isValid() )
        {

            QMessageBox::warning( this, i18n( "Warning - Breeze Settings" ), i18n("Regular Expression syntax is incorrect") );
            QPointer<ExceptionDialog> dialog( new ExceptionDialog( this ) );
            dialog->setException( exception );
            if( dialog->exec() == QDialog::Rejected )
            {
                delete dialog;
                return false;
            }

            dialog->save();
            delete dialog;
        }

        return true;
    }

}
