/*
 * Copyright (C) 2018 Factory #12
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

//
// includes
//
#include <QInputDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "event.h"
#include "task.h"
#include "team.h"
#include "log.h"
#include "delegate.h"
#include <QDebug>
#include "editordialog.h"
#include "tasktoolbar.h"
#include "teamtoolbar.h"
#include "rankings.h"
#include "settings.h"
#include "variable.h"
#include "database.h"
#include "console.h"
#include "combos.h"
#include "main.h"
#include "teamedit.h"
#include "eventedit.h"
#include "eventtoolbar.h"

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ),
    ui( new Ui::MainWindow ),
    filter( new QSortFilterProxyModel()),
    spacer( new QWidget()),
    quickSpacerLeft( new QWidget()),
    quickSpacerRight( new QWidget()),
    timeEdit( new QTimeEdit()) {

    // set up ui
    this->ui->setupUi( this );
    this->ui->eventCombo->setModel( Event::instance());
    this->ui->eventCombo->setModelColumn( Event::Title );

    this->ui->teamCombo->setModel( Team::instance());
    this->ui->teamCombo->setModelColumn( Team::Title );

    // setup task/logView
    // NOTE: must handle proxy model indexes properly!
    this->filter->setFilterKeyColumn( Task::Name );
    this->filter->setFilterCaseSensitivity( Qt::CaseInsensitive );
    this->filter->setSourceModel( Task::instance());
    this->ui->taskView->setModel( this->filter );
    this->ui->taskView->setModelColumn( Task::Name );
    this->ui->taskView->setItemDelegate( new Delegate( this->ui->taskView ));

    // bind event/team variables to comboBoxes
    Variable::instance()->bind( "eventId", this->ui->eventCombo );
    Variable::instance()->bind( "teamId", this->ui->teamCombo );

    // insert spacer
    this->spacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    this->ui->toolBar->insertWidget( this->ui->actionSettings, spacer );

    // bind for sorting updates
    Variable::instance()->bind( "sortByType", this, SLOT( setTaskFilter()));

    // set up completer
    this->completer.setModel( Task::instance());
    this->ui->findEdit->setCompleter( &this->completer );
    this->completer.setCompletionColumn( Task::Name );
    this->completer.setCaseSensitivity( Qt::CaseInsensitive );
    this->connect( this->ui->findEdit, &QLineEdit::textChanged, [ this ]( const QString &text ) {
        this->filter->setFilterRegExp( text );
    } );

    // set up secondary toolBar
    this->ui->quickBar->insertWidget( this->ui->actionLogTime, this->timeEdit );

    // position quickToolbar
    quickSpacerLeft->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    quickSpacerRight->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    this->ui->quickBar->insertWidget( this->ui->actionAddQuick, quickSpacerLeft );
    this->ui->quickBar->addWidget( quickSpacerRight );

    // currentTime button
    this->connect( this->ui->actionLogTime, &QAction::triggered, [ this ]() {
        this->timeEdit->setTime( QTime::currentTime());
    } );

    // done button
    this->connect( this->ui->actionDone, &QAction::triggered, [ this ]() {
        this->setTaskFilter();
    } );

    // time updater
    this->connect( this->timeEdit, &QTimeEdit::timeChanged, [ this ]( const QTime &time ) {
        // check for valid team
        const Id teamId = this->currentTeamId();
        if ( teamId == Id::Invalid )
            return;

        // check for valid event
        const Id eventId = this->currentEventId();
        if ( eventId == Id::Invalid )
            return;

        Row row = Team::instance()->row( teamId );
        if ( row == Row::Invalid )
            return;

        // check for valid time
        const QTime startTime = Event::instance()->startTime( Event::instance()->row( eventId ));
        const QTime finalTime = Event::instance()->finalTime( Event::instance()->row( eventId ));
        if ( time > finalTime ) {
            this->timeEdit->setTime( finalTime );
            return;
        }

        if ( time < startTime ) {
            this->timeEdit->setTime( startTime );
            return;
        }

        // set new time if anything changed
        if ( Team::instance()->finishTime( row ) != time )
            Team::instance()->setFinishTime( Team::instance()->row( this->currentTeamId()), time );
    } );

    // clear button
    this->connect( this->ui->clearButton, &QToolButton::pressed, [ this ]() {
        this->ui->findEdit->clear();
    } );

    // add to garbage man
    GarbageMan::instance()->add( this );
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    // unbind variables
    Variable::instance()->unbind( "eventId", this->ui->eventCombo );
    Variable::instance()->unbind( "teamId", this->ui->teamCombo );

    // disconnect lambdas
    this->disconnect( this->ui->findEdit, SLOT( textChanged( QString )));
    this->disconnect( this->ui->actionLogTime, SLOT( triggered( bool )));
    this->disconnect( this->timeEdit, SLOT( timeChanged( QTime )));
    this->disconnect( this->ui->clearButton, SLOT( pressed()));
    this->disconnect( this->ui->actionDone, SLOT( triggered( bool )));

    // delete ui elements
    delete this->timeEdit;
    delete this->spacer;
    delete this->quickSpacerLeft;
    delete this->quickSpacerRight;
    delete this->filter;
    delete this->ui;
}

/**
 * @brief MainWindow::currentEventId
 * @return
 */
Id MainWindow::currentEventId() const {
    const int index = this->ui->eventCombo->currentIndex();

    // abort if database has not been initialized
    if ( !Database::instance()->hasInitialised())
        return Id::Invalid;

    if ( index == -1 )
        return Id::Invalid;

    return Event::instance()->id( Event::instance()->indexToRow( index ));
}

/**
 * @brief MainWindow::currentTeamId
 * @return
 */
Id MainWindow::currentTeamId() const {
    const int index = this->ui->teamCombo->currentIndex();

    // abort if database has not been initialized
    if ( !Database::instance()->hasInitialised())
        return Id::Invalid;

    if ( index == -1 )
        return Id::Invalid;

    return Team::instance()->id( Team::instance()->indexToRow( index ));
}

/**
 * @brief MainWindow::sourceIndex
 * @param index
 * @return
 */
QModelIndex MainWindow::sourceIndex( const QModelIndex &index ) const {
    if ( index.model() != this->filter ) {
        qDebug() << "bad model index" << index.row() << index.column();
        return QModelIndex();
    }

    return this->filter->mapToSource( index );
}

/**
 * @brief MainWindow::setCurrentTeam
 * @param id
 */
void MainWindow::setCurrentTeam( const Id &id ) {
    if ( id == Id::Invalid )
        return;

    const Row row = Team::instance()->row( id );
    if ( row == Row::Invalid )
        return;

    this->ui->teamCombo->setCurrentIndex( static_cast<int>( row ));
}

/**
 * @brief MainWindow::on_eventCombo_currentIndexChanged
 * @param index
 */
void MainWindow::on_eventCombo_currentIndexChanged( int index ) {
    // abort if database has not been initialized
    if ( !Database::instance()->hasInitialised())
        return;

    // failsafe
    const Row row = Event::instance()->indexToRow( index );
    if ( row == Row::Invalid ) {
        Team::instance()->setFilter( "eventId=-1" );
        return;
    }

    // filter tasks
    Team::instance()->setFilter( QString( "eventId=%1" ).arg( static_cast<int>( Event::instance()->id( row ))));
    this->setTaskFilter();
}

/**
 * @brief MainWindow::on_teamCombo_currentIndexChanged
 * @param index
 */
void MainWindow::on_teamCombo_currentIndexChanged( int index ) {
    // abort if database has not been initialized
    if ( !Database::instance()->hasInitialised())
        return;

    // reset item delegate if any
    if ( this->ui->taskView->itemDelegate() != nullptr ) {
        Delegate *delegate( qobject_cast<Delegate*>( this->ui->taskView->itemDelegate()));

        if ( delegate != nullptr )
            delegate->reset();
    } else {
        return;
    }

    // update view
    this->ui->taskView->viewport()->update();
    this->ui->taskView->setEnabled( index == -1 ? false : true );

    // update time edit
    const Id teamId = this->currentTeamId();
    QTime time( QTime::currentTime());
    if ( teamId != Id::Invalid )
        time = Team::instance()->finishTime( Team::instance()->row( teamId ));
    this->timeEdit->setTime( time );

    // reset task filter
    this->setTaskFilter();
}

/**
 * @brief MainWindow::on_actionEvents_triggered
 */
void MainWindow::on_actionEvents_triggered() {
    EditorDialog *editor( EditorDialog::instance());

    editor->show();
    editor->container->clearSelection();
    editor->container->setModel( Event::instance());
    editor->container->setModelColumn( Event::Title );
    editor->setToolBar( EventToolBar::instance());
    editor->setWindowTitle( this->tr( "Event manager" ));
    editor->setWindowIcon( QIcon( ":/icons/ketone" ));

    EventToolBar::instance()->buttonTest();
    EventToolBar::instance()->show();
}

/**
 * @brief MainWindow::on_actionTeams_triggered
 */
void MainWindow::on_actionTeams_triggered() {
    EditorDialog *editor( EditorDialog::instance());

    editor->show();
    editor->container->clearSelection();
    editor->container->setModel( Team::instance());
    editor->container->setModelColumn( Team::Title );
    editor->setToolBar( TeamToolBar::instance());
    editor->setWindowTitle( this->tr( "Team manager" ));
    editor->setWindowIcon( QIcon( ":/icons/teams" ));

    TeamToolBar::instance()->buttonTest();
    TeamToolBar::instance()->show();
}

/**
 * @brief MainWindow::on_actionTasks_triggered
 */
void MainWindow::on_actionTasks_triggered() {
    EditorDialog *editor( EditorDialog::instance());

    editor->show();
    editor->container->clearSelection();
    editor->container->setModel( Task::instance());
    editor->container->setModelColumn( Task::Name );
    editor->setToolBar( TaskToolBar::instance());
    editor->setWindowTitle( this->tr( "Task manager" ));
    editor->setWindowIcon( QIcon( ":/icons/tasks" ));

    TaskToolBar::instance()->buttonTest();
    TaskToolBar::instance()->show();
}

/**
 * @brief MainWindow::on_actionRankings_triggered
 */
void MainWindow::on_actionRankings_triggered() {
    Rankings::instance()->show();
}

/**
 * @brief MainWindow::on_actionSettings_triggered
 */
void MainWindow::on_actionSettings_triggered() {
    Settings::instance()->show();
}

/**
 * @brief MainWindow::on_actionConsole_triggered
 */
void MainWindow::on_actionConsole_triggered() {
    Console::instance()->show();
}

/**
 * @brief MainWindow::setTaskFilter
 */
void MainWindow::setTaskFilter( bool filterByCombo, Id comboId ) {
    const bool sort = Variable::instance()->isEnabled( "sortByType" );

    // make sure to store this variable
    this->m_comboMode = filterByCombo;
    this->m_currentCombo = comboId;

    // disable ui components
    this->setUiLock( filterByCombo );

    if ( filterByCombo )
        this->ui->toolBar->insertAction( this->ui->actionTeams, this->ui->actionDone );
    else
        this->ui->toolBar->removeAction( this->ui->actionDone );

    // NOTE: THIS!
    Task::instance()->setFilter(
                QString( "eventId=%1 %2"
                         "order by %3 %4" )
                /*1*/.arg( static_cast<int>( this->currentEventId()))
                /*2*/ .arg( filterByCombo ?
                                QString( "and %1 in ( select %2 from %3 where %4=%5 and ( %6=%7 or %6=-1 ) and %8>0 )" )
                                /*2.1*/.arg( Task::instance()->fieldName( Task::ID ))
                                /*2.2*/.arg( Log::instance()->fieldName( Log::Task ))
                                /*2.3*/.arg( Log::instance()->tableName())
                                /*2.4*/.arg( Log::instance()->fieldName( Log::Team ))
                                /*2.5*/.arg( static_cast<int>( this->currentTeamId()))
                                /*2.6*/.arg( Log::instance()->fieldName( Log::Combo ))
                                /*2.7*/.arg( static_cast<int>( comboId ))
                                /*2.8*/.arg( Log::instance()->fieldName( Log::Multi )) :
                                "" )
                /*3*/.arg( sort ?
                               Task::instance()->fieldName( Task::Type ) :
                               Task::instance()->fieldName( Task::Order ))
                /*4*/.arg( sort ?
                               QString( ", %1 asc" )
                               /*4.1*/.arg( Task::instance()->fieldName( Task::Name )) :
                               "" ));
}

/**
 * @brief MainWindow::setUiLock
 * @param lock
 */
void MainWindow::setUiLock( bool lock ) {
    const bool noTeams = !this->ui->teamCombo->count();
    const bool noEvents = !this->ui->eventCombo->count();

    if ( noEvents || noTeams )
        lock = true;

    this->ui->taskView->setDisabled( lock && !this->isComboModeActive());
    this->ui->actionTeams->setDisabled( lock && noEvents );
    this->ui->actionTasks->setDisabled( lock );
    this->ui->actionRankings->setDisabled( lock );
    this->ui->actionSettings->setDisabled( lock );
    //this->ui->actionConsole->setDisabled( lock );
    this->ui->actionCombos->setDisabled( lock );
    this->ui->actionAddQuick->setDisabled( lock );
    this->ui->actionLogTime->setDisabled( lock );
    this->ui->actionCombine->setDisabled( lock );
    this->ui->teamCombo->setDisabled( lock );
    this->ui->eventCombo->setDisabled( lock );
    this->timeEdit->setDisabled( lock );
}

/**
 * @brief MainWindow::closeEvent
 * @param event
 */
void MainWindow::closeEvent( QCloseEvent *event ) {
    // disallow closing when modal windows are open
    if ( !this->isEnabled()) {
        event->ignore();
        return;
    }

    QMainWindow::closeEvent( event );
}

/**
 * @brief MainWindow::on_actionCombos_triggered
 */
void MainWindow::on_actionCombos_triggered() {
    Combos::instance()->show();
}

/**
 * @brief MainWindow::on_actionAddQuick_triggered
 */
void MainWindow::on_actionAddQuick_triggered() {
    TeamEdit *edit( TeamEdit::instance());

    EditorDialog::instance()->dock->setWidget( nullptr );
    edit->setParent( nullptr );
    edit->reset( false );
    edit->show();
    edit->setCurrentTime();
    edit->move( this->geometry().x() + this->geometry().width() / 2 - edit->geometry().width() / 2, this->geometry().y() + this->geometry().height() / 2 - edit->geometry().height() / 2 );
}
