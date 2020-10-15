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
#include <QInputDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "event.h"
#include "task.h"
#include "team.h"
#include "log.h"
#include "delegate.h"
#include <QDebug>
#include <QFileDialog>
#include <QSqlQuery>
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
#include "about.h"

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ),
    ui( new Ui::MainWindow ),
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

    // setup pixmaps
    this->ui->eventPixmap->setPixmap( QIcon::fromTheme( "name" ).pixmap( 16, 16 ));
    this->ui->teamPixmap->setPixmap( QIcon::fromTheme( "teams" ).pixmap( 16, 16 ));
    this->ui->findPixmap->setPixmap( QIcon::fromTheme( "find" ).pixmap( 16, 16 ));

    // setup task/logView
    this->ui->taskView->setModel( Task::instance());
    this->ui->taskView->setModelColumn( Task::Name );
    this->ui->taskView->setItemDelegate( new Delegate( this->ui->taskView ));

    // bind event/team variables to comboBoxes
    Variable::instance()->bind( "eventId", this->ui->eventCombo );
    Variable::instance()->bind( "teamId", this->ui->teamCombo );

    // insert spacer
    this->spacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    this->ui->toolBar->insertWidget( this->ui->actionAbout, spacer );

    // bind for sorting updates
    Variable::instance()->bind( "sortByType", this, SLOT( setTaskFilter()));

    // set up completer
    this->completer.setModel( Task::instance());
    this->ui->findEdit->setCompleter( &this->completer );
    this->completer.setCompletionColumn( Task::Name );
    this->completer.setCaseSensitivity( Qt::CaseInsensitive );
    this->completer.setFilterMode( Qt::MatchContains );
    this->connect( this->ui->findEdit, &QLineEdit::textChanged, [ this ]( const QString & ) {
        this->setTaskFilter( this->isComboModeActive(), this->currentComboId());
        this->setLock();
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
        // here's what we do (MIND that at this point we see the filtered list):
        //   1) we get a list of all comboIds currently visible, such as
        //      task1   -1
        //      task2   13
        //      task3   -1
        //   2) here is what we do next:
        //      a) there are MULTIPLE tasks (as in >1) with valid comboId (not -1) - ALL OK
        //      b) there are NO tasks with valid comboId - ALL OK
        //      c) there is A SINGLE task with a valid comboId (all other's are set to -1):
        //         a single task log CANNOT have a comboId, therefore we must reset it to -1
        //         which is done via simple query
        //
        QList<Id> idList( qobject_cast<Delegate*>( this->ui->taskView->itemDelegate())->combos.values());
        idList.removeAll( Id::Invalid );

#ifdef QT_DEBUG
        qCDebug( Database_::Debug ) << ( idList.count() <= 1 ? "bad list" : "okay list" ) << idList.count();
#endif

        // do a simple query
        if ( idList.count() == 1 ) {
            QSqlQuery query;
            query.exec( QString( "UPDATE %1 SET %2=-1 WHERE %2=%3" )
                        .arg( Log::instance()->tableName(),
                              Log::instance()->fieldName( Log::Combo ),
                              QString::number( static_cast<int>( idList.first()))));

            // must perform SELECT!
            Log::instance()->select();
        }

        this->setTaskFilter();
    } );

    // time updater
    this->connect( this->timeEdit, &QTimeEdit::timeChanged, [ this ]( const QTime &time ) {
        // check for valid event
        const Row event = this->currentEvent();
        if ( event == Row::Invalid )
            return;

        // check for valid team
        const Row team = this->currentTeam();
        if ( event == Row::Invalid )
            return;

        // check for valid time
        const QTime startTime = Event::instance()->startTime( event );
        const QTime finalTime = Event::instance()->finalTime( event );
        const QTime teamTime = Team::instance()->finishTime( team );

        if ( time > finalTime ) {
            this->timeEdit->setTime( finalTime );
            return;
        }

        if ( time < startTime ) {
            this->timeEdit->setTime( startTime );
            return;
        }

        // set new time if anything changed
        if ( teamTime != time )
            Team::instance()->setFinishTime( this->currentTeam(), time );
    } );

    // clear button
    this->connect( this->ui->clearButton, &QToolButton::pressed, [ this ]() {
        this->ui->findEdit->clear();
    } );

    // add to garbage man
    GarbageMan::instance()->add( this );

#ifndef QT_DEBUG
    this->ui->quickBar->removeAction( this->ui->actionExport_logs );
#endif

    // lock/unlock ui elements
    this->setLock();
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
    delete this->ui;
}

/**
 * @brief MainWindow::currentEventRow
 * @return
 */
Row MainWindow::currentEvent() const {
    const int index = this->ui->eventCombo->currentIndex();

    // abort if database has not been initialized
    if ( !Database::instance()->hasInitialised())
        return Row::Invalid;

    if ( index == -1 )
        return Row::Invalid;

    return Event::instance()->row( index );
}

/**
 * @brief MainWindow::currentTeamRow
 * @return
 */
Row MainWindow::currentTeam() const {
    const int index = this->ui->teamCombo->currentIndex();

    // abort if database has not been initialized
    if ( !Database::instance()->hasInitialised())
        return Row::Invalid;

    if ( index == -1 )
        return Row::Invalid;

    return Team::instance()->row( index );
}

/**
 * @brief MainWindow::setCurrentTeam
 * @param id
 */
void MainWindow::setCurrentTeam( const Row &row ) {
    if ( row == Row::Invalid )
        return;

    this->ui->teamCombo->setCurrentIndex( static_cast<int>( row ));
}

/**
 * @brief MainWindow::setCurrentEvent
 * @param id
 */
void MainWindow::setCurrentEvent( const Row &row ) {
    if ( row == Row::Invalid )
        return;

    this->ui->eventCombo->setCurrentIndex( static_cast<int>( row ));
}

/**
 * @brief MainWindow::on_eventCombo_currentIndexChanged
 * @param index
 */
void MainWindow::on_eventCombo_currentIndexChanged( int index ) {
    // abort if database has not been initialized
    if ( !Database::instance()->hasInitialised()) {
        this->setLock();
        return;
    }

    // failsafe
    const Row row = Event::instance()->row( index );
    if ( row == Row::Invalid ) {
        Team::instance()->setFilter( "eventId=-1" );

        this->setLock();
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
    if ( !Database::instance()->hasInitialised()) {
        this->setLock();
        return;
    }

    // reset item delegate if any
    if ( this->ui->taskView->itemDelegate() != nullptr ) {
        Delegate *delegate( qobject_cast<Delegate*>( this->ui->taskView->itemDelegate()));

        if ( delegate != nullptr )
            delegate->reset();
    } else {
        this->setLock();
        return;
    }

    // update view
    this->ui->taskView->viewport()->update();

    // failsafe
    const Row team = Event::instance()->row( index );
    if ( team == Row::Invalid ) {
        this->setLock();

        this->ui->actionEvents->setDisabled( false );
        this->ui->actionTeams->setDisabled( this->currentEvent() == Row::Invalid );
        this->timeEdit->setTime( QTime());
        return;
    }

    // update time edit
    this->timeEdit->setTime( Team::instance()->finishTime( team ));

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
    editor->setWindowIcon( QIcon::fromTheme( "ketone" ));

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
    editor->setWindowIcon( QIcon::fromTheme( "teams" ));

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
    editor->setWindowIcon( QIcon::fromTheme( "tasks" ));

    TaskToolBar::instance()->buttonTest();
    TaskToolBar::instance()->show();

    // clear this, so that task editor is not empty
    this->ui->findEdit->clear();
}

/**
 * @brief MainWindow::setTaskFilter SQL-heavy task filter
 * @param filterByCombo
 * @param comboId
 */
void MainWindow::setTaskFilter( bool filterByCombo, const Id &comboId ) {
    const bool sort = Variable::isEnabled( "sortByType" );
    const Id eventId = this->currentEvent() == Row::Invalid ? Id::Invalid : Event::instance()->id( this->currentEvent());
    const Id teamId = this->currentTeam() == Row::Invalid ? Id::Invalid : Team::instance()->id( this->currentTeam());
    const QString find( this->ui->findEdit->text());

    // make sure to store this variable
    this->m_comboMode = filterByCombo;
    this->m_currentCombo = comboId;

    // disable ui components
    this->setLock();

    // add/remove done action
    if ( filterByCombo )
        this->ui->toolBar->insertAction( this->ui->actionEvents, this->ui->actionDone );
    else
        this->ui->toolBar->removeAction( this->ui->actionDone );

    // selects tasks from current event
    const QString eventFilter( QString( "%1=%2" )
                               .arg( Task::instance()->fieldName( Task::Event ))
                               .arg( static_cast<int>( eventId )));

    // selects matching tasks from quick search
    const QString comboFind( !filterByCombo ? "" :
                                              QString( "OR %1=%2" )
                                              .arg( Log::instance()->fieldName( Log::Fields::Combo ))
                                              .arg( static_cast<int>( comboId ))
                                              );
    const QString findFilter( find.isEmpty() ?
                                  "" :
                                  QString( "AND %1 LIKE '%%2%' %3 " )
                                  .arg( Task::instance()->fieldName( Task::Name ),
                                        find,
                                        comboFind ));

    // selects tasks for combo mode
    const QString comboFilter( filterByCombo ?
                                   QString( "and %1.%2 in "
                                            "( SELECT %3 FROM %4 WHERE %5=%6 AND ( %7=%8 OR %7=-1 ) AND %9>0 )" )
                                   .arg( Task::instance()->tableName(),
                                         Task::instance()->fieldName( Task::ID ),
                                         Log::instance()->fieldName( Log::Task ),
                                         Log::instance()->tableName(),
                                         Log::instance()->fieldName( Log::Team ),
                                         QString::number( static_cast<int>( teamId )),
                                         Log::instance()->fieldName( Log::Combo ),
                                         QString::number( static_cast<int>( comboId )),
                                         Log::instance()->fieldName( Log::Multi )) :
                                   "" );

    // orders tasks according to settings
    const QString comboOrder( QString( "%1 DESC," ).arg( Log::instance()->fieldName( Log::Fields::Combo )));
    const QString orderFilter(
                QString( "ORDER BY %1 %2 ASC %3" )
                .arg( !filterByCombo ?
                          "" :
                          comboOrder )
                .arg( sort ?
                          Task::instance()->fieldName( Task::Style ) :
                          Task::instance()->fieldName( Task::Order ))
                .arg( sort ?
                      #ifdef SQLITE_CUSTOM
                          QString( ", %1 COLLATE localeCompare ASC" )
                      #else
                          QString( ", %1 COLLATE NOCASE ASC" )
                      #endif
                          .arg( Task::instance()->fieldName( Task::Name )) :
                          "" ));

    // put all filters together
    const QString filter(
                QString( "%1 %2 %3 %4" )
                .arg( eventFilter, comboFilter, findFilter, orderFilter ));

    //qDebug() << filter;

    // set filter
    Task::instance()->setFilter( filter );

    // scroll to the beginning
    this->ui->taskView->scrollToTop();
}

/**
 * @brief MainWindow::setUiLock
 * @param lock
 */
void MainWindow::setLock() {
    const bool comboMode = this->isComboModeActive();
    const bool noEvents = !this->ui->eventCombo->count();
    const bool noTeams = !this->ui->teamCombo->count();
    const bool noTasks = !Task::instance()->count();

    // lock common ui elements
    this->ui->actionTasks->setDisabled( comboMode || noEvents || noTeams );
    this->ui->actionRankings->setDisabled( comboMode || noEvents || noTeams );
    //this->ui->actionSettings->setDisabled( comboMode || noEvents || noTeams );
    this->ui->actionCombos->setDisabled( comboMode || noEvents || noTeams );
    this->ui->actionAddQuick->setDisabled( comboMode || noEvents || noTeams );
    this->ui->actionLogTime->setDisabled( comboMode || noEvents || noTeams );
    this->timeEdit->setDisabled( comboMode || noEvents || noTeams );
    this->ui->taskView->setDisabled( noEvents || noTeams || noTasks );

    // team disabler/enabler
    this->ui->teamCombo->setDisabled( comboMode || noEvents || noTeams );
    this->ui->actionTeams->setDisabled( comboMode || noEvents );

    // event disabler/enabler
    this->ui->eventCombo->setDisabled( comboMode || noEvents );
    this->ui->actionEvents->setDisabled( comboMode );
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

/**
 * @brief MainWindow::on_actionRankings_triggered
 */
void MainWindow::on_actionRankings_triggered() { Rankings::instance()->show(); }

/**
 * @brief MainWindow::on_actionSettings_triggered
 */
void MainWindow::on_actionSettings_triggered() { Settings::instance()->show(); }

/**
 * @brief MainWindow::on_actionConsole_triggered
 */
void MainWindow::on_actionConsole_triggered() { Console::instance()->show(); }

/**
 * @brief MainWindow::on_actionCombos_triggered
 */
void MainWindow::on_actionCombos_triggered() { Combos::instance()->show(); }

/**
 * @brief MainWindow::on_actionAbout_triggered
 */
void MainWindow::on_actionAbout_triggered() { About( this ).exec(); }

/**
 * @brief MainWindow::on_actionExport_logs_triggered
 */
void MainWindow::on_actionExport_logs_triggered() {
    QSqlQuery query;

    const Row team = this->currentTeam();
    if ( team == Row::Invalid )
        return;

    //qDebug() << Team::instance()
    query.exec( QString( "select * from %1 where %2=%3" )
                .arg( Log::instance()->tableName(),
                      Log::instance()->fieldName( Log::Team ),
                      QString::number( static_cast<int>( Team::instance()->id( team )))));

    QString path( QFileDialog::getSaveFileName( this, this->tr( "Export logs to CSV format" ), QDir::homePath() + "/" + Team::instance()->title( team ) + ".csv", this->tr( "CSV file (*.csv)" )));
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
        out << this->tr( "Name;Style;Points" ).append( win32 ? "\r" : "\n" );

        while ( query.next()) {
            const Id id = static_cast<Id>( query.value( Log::Task ).toInt());
            if ( id == Id::Invalid )
                continue;

            const Row row = Task::instance()->row( id );
            if ( row == Row::Invalid )
                continue;

            out << QString( "%1;%2;%3%4" )
                   .arg( Task::instance()->name( row ))
                   .arg( static_cast<int>( Task::instance()->style( row )))
                   .arg( query.value( Log::Multi ).toInt())
                   .arg( win32 ? "\r" : "\n" );

        }
    }
    csv.close();
}
