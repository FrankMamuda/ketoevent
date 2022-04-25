/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020 Armands Aleksejevs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

/*
 * includes
 */
#include "editordialog.h"
#include "main.h"
#include "task.h"
#include "taskedit.h"
#include "tasktoolbar.h"
#include "variable.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#ifdef XLSX_SUPPORT
#include <xlsxdocument.h>
#endif

/**
 * @brief TaskToolBar::TaskToolBar
 */
TaskToolBar::TaskToolBar( QWidget *parent ) : ToolBar( parent ) {
    // add action
    this->addAction( QIcon::fromTheme( "add" ), this->tr( "Add Task" ), [ this ]() {
        if ( !EditorDialog::instance().isDockVisible()) {
            EditorDialog::instance().showDock( &TaskEdit::instance(), this->tr( "Add Task " ));
            TaskEdit::instance().reset();
        }
    } );

    // edit action
    this->edit = this->addAction( QIcon::fromTheme( "edit" ), this->tr( "Edit Task" ), [ this ]() {
        if ( !EditorDialog::instance().isDockVisible()) {
            EditorDialog::instance().showDock( &TaskEdit::instance(), this->tr( "Edit Task " ));
            TaskEdit::instance().reset( true );
        }
    } );

    // remove action
    this->remove = this->addAction( QIcon::fromTheme( "remove" ), this->tr( "Remove Task" ), [ this ]() {
        const QModelIndex index( EditorDialog::instance().container->currentIndex());

        if ( EditorDialog::instance().isDockVisible() || !index.isValid())
            return;

        const Row row = Task::instance().row( index );
        if ( row == Row::Invalid )
            return;

        const QString name( Task::instance().name( row ));
        if ( QMessageBox::question( this, this->tr( "Remove task" ), this->tr( "Do you really want to remove \"%1\"?" ).arg( name )) == QMessageBox::Yes )
            Task::instance().remove( row );
    } );
    this->remove->setEnabled( false );

    // move up/down lambda
    auto move = [ this ]( bool up ) {
        // NOTE: reordering is required for duplicates
        //       (there should not be any, if all works as intended)
        //       non-sequential order does not cause problems however

        // test integrity
        QSet<int> orderSet;
        bool reindex = false;
        int y;
        for ( y = 0; y < Task::instance().count(); y++ ) {
            const int order = Task::instance().order( Task::instance().row( y ));
            if ( orderSet.contains( order )) {
                if ( QMessageBox::question( this, this->tr( "Corrupted order" ),
                                            this->tr( "Tasks have corrupted order. Perform reindexing? This cannot be undone." )) == QMessageBox::Yes ) {
                    reindex = true;
                }
                break;
            } else {
                orderSet << order;
            }
        }

        // reindex tasks if requested
        if ( reindex ) {
            QList<Id> idList;

            // get id list
            for ( int y = 0; y < Task::instance().count(); y++ )
                idList << Task::instance().id( Task::instance().row( y ));

            // reorder tasks accordint to id list
            y = 0;
            for ( const Id id : qAsConst( idList )) {
                Task::instance().setOrder( Task::instance().row( id ), y );
                y++;
            }
        }

        // get container pointer and order indexes
        QListView *container( EditorDialog::instance().container );
        const QModelIndex index( container->currentIndex());
        const QModelIndex other( container->model()->index( container->currentIndex().row() + ( up ? -1 : 1 ), 0 ));

        if ( EditorDialog::instance().isDockVisible() || !index.isValid() || !other.isValid())
            return;

        // use ids in lookup (QPersistentModel index should work too?)
        const Id id0 = Task::instance().id( Task::instance().row( index ));
        const Id id1 = Task::instance().id( Task::instance().row( other ));
        const int order0 = Task::instance().order( Task::instance().row( index ));
        const int order1 = Task::instance().order( Task::instance().row( other ));

        // swap order
        Task::instance().setOrder( Task::instance().row( id0 ), order1 );
        Task::instance().setOrder( Task::instance().row( id1 ), order0 );

        Task::instance().select();
        const QModelIndex current( container->model()->index( static_cast<int>( Task::instance().row( id0 )), 0 ));
        container->setCurrentIndex( current );
        container->setFocus();
        this->buttonTest( current );
    };

    // move up action
    this->moveUp = this->addAction( QIcon::fromTheme( "up" ), this->tr( "Move up" ), [ move ]() {
        move( true );
    } );
    moveUp->setEnabled( false );

    // move down action
    this->moveDown = this->addAction( QIcon::fromTheme( "down" ), this->tr( "Move down" ), [ move ]() {
        move( false );
    } );
    moveDown->setEnabled( false );

    // export action
    this->addAction( QIcon::fromTheme( "export" ), this->tr( "Export tasks" ), [ this ]() {
        QString path( QFileDialog::getSaveFileName( this, this->tr( "Export tasks to CSV format" ), QDir::homePath(), this->tr( "CSV file (*.csv)" )));
    #ifdef Q_OS_WIN
        const bool win32 = true;
    #else
        const bool win32 = false;
    #endif

        // check for empty filenames
        if ( path.isEmpty())
            return;

        // add extension
        if ( !path.endsWith( ".csv" ))
            path.append( ".csv" );

        // create file
        QFile csv( path );

        if ( csv.open( QFile::WriteOnly | QFile::Truncate )) {
            QTextStream out( &csv );
            out.setCodec( win32 ? "Windows-1257" : "UTF-8" );
            out << this->tr( "Task name;Description;Type;Style;Multi;Points" ).append( win32 ? "\r" : "\n" );

            for ( int y = 0; y < Task::instance().count(); y++ ) {
                const Row row = Task::instance().row( y );
                if ( row == Row::Invalid )
                    break;

                QString style;
                switch ( Task::instance().style( row )) {
                case Task::Styles::Regular:
                    style = this->tr( "Simple" );
                    break;

                case Task::Styles::Bold:
                    style = this->tr( "Difficult" );
                    break;

                case Task::Styles::Italic:
                    style = this->tr( "Other" );
                    break;

                case Task::Styles::NoStyle:
                    break;
                }

                out << QString( "%1;%2;%3;%4;%5;%6%7" )
                       .arg( Task::instance().name( row ).replace( ";", " |" ),
                             Task::instance().description( row ).replace( ";", "  |" ),
                             Task::instance().type( row ) == Task::Types::Multi ? this->tr( "Multi" ) : this->tr( "Regular" ),
                             style,
                             Task::instance().type( row ) == Task::Types::Multi ? QString::number( Task::instance().multi( row )) : "",
                             QString::number( Task::instance().points( row )),
                             win32 ? "\r" : "\n" );
            }
        }
        csv.close();
    } );

#ifdef XLSX_SUPPORT
    // export action (xlsx)
    this->addAction( QIcon::fromTheme( "export" ), this->tr( "Export as xlsx" ), [ this ]() {
        QString path( QFileDialog::getSaveFileName( this, this->tr( "Export tasks to XLSX format" ), QDir::homePath(), this->tr( "XLSX spreadsheet (*.xlsx)" )));

        // check for empty filenames
        if ( path.isEmpty())
            return;

        // add extension
        if ( !path.endsWith( ".xlsx" ))
            path.append( ".xlsx" );

        // create file
        QXlsx::Document xlsx;

        QStringList simple;
        QStringList difficult;
        QStringList other;

        for ( int y = 0; y < Task::instance().count(); y++ ) {
            const Row row = Task::instance().row( y );
            if ( row == Row::Invalid )
                break;

            const int points = Task::instance().points( row );
            const QString description( Task::instance().description( row ));
            const QString out( QString( "%1%2 %3 %4 " )
                   .arg( Task::instance().name( row ),
                         description.isEmpty() ? "" : QString( " (%1)" ).arg( Task::instance().description( row )),
                         QChar( 0x2013 ),
                         ( Task::instance().type( row ) == Task::Types::Multi ) ?
                         QString( "%1-%2" ).arg( QString::number( points ), QString::number( points * Task::instance().multi( row ))) : QString::number( points ))
                               + this->tr( "points" ));

            QString style;
            switch ( Task::instance().style( row )) {
            case Task::Styles::Regular:
                simple << out;
                break;

            case Task::Styles::Bold:
                difficult << out;
                break;

            case Task::Styles::Italic:
                other << out;
                break;

            case Task::Styles::NoStyle:
                break;
            }
        }

        std::sort( simple.begin(), simple.end(), []( const QString &left, const QString &right ) { return QString::localeAwareCompare( left, right ) < 0; } );
        std::sort( difficult.begin(), difficult.end(), []( const QString &left, const QString &right ) { return QString::localeAwareCompare( left, right ) < 0; } );
        std::sort( other.begin(), other.end(), []( const QString &left, const QString &right ) { return QString::localeAwareCompare( left, right ) < 0; } );

        QXlsx::Format boldFormat;
        boldFormat.setFontBold( true );

        int row = 1;
        xlsx.write( row++, 1, this->tr( "Simple tasks" ), boldFormat );
        for ( const QString &str : simple ) {
            xlsx.write( row, 1, str );
            row++;
        }
        row++;

        xlsx.write( row++, 1, this->tr( "Difficult tasks" ), boldFormat );
        for ( const QString &str : difficult ) {
            xlsx.write( row, 1, str );
            row++;
        }
        row++;

        xlsx.write( row++, 1, this->tr( "Other tasks" ), boldFormat );
        for ( const QString &str : other ) {
            xlsx.write( row, 1, str );
            row++;
        }

        xlsx.setDocumentProperty( "title", "Tasks" );
        xlsx.setDocumentProperty( "creator", "Ketoevent" );
        xlsx.setDocumentProperty( "description", "Exported with ketoevent via qtxlsx" );

        xlsx.saveAs( path );
    } );
#endif

    // button test (disconnected in ~EditorDialog)
    this->connect( EditorDialog::instance().container, SIGNAL( clicked( QModelIndex )), this, SLOT( buttonTest( QModelIndex )));
    this->buttonTest();
}

/**
 * @brief TaskToolBar::buttonTest
 * @param index
 */
void TaskToolBar::buttonTest( const QModelIndex &index ) {
    if ( Variable::isEnabled( "sortByType" )) {
        this->moveUp->setDisabled( true );
        this->moveDown->setDisabled( true );
    } else {
        this->moveUp->setEnabled( index.isValid() && index.row() != 0 );
        this->moveDown->setEnabled( index.isValid() && index.row() != Task::instance().count() - 1 );
    }
    this->edit->setEnabled( index.isValid());
    this->remove->setEnabled( index.isValid());
};
