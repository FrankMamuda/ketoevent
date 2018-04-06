/*
 * Copyright (C) 2013-2018 Factory #12
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
#include "main.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "taskwidget.h"
#include "eventdialog.h"
#include "combodialog.h"
#include "about.h"
#include <QMessageBox>
#ifdef Q_OS_UNIX
#include <sys/types.h>
#include <unistd.h>
#endif

// TODO: SWITCH to new event after addition

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ) {
    this->ui->setupUi( this );
    this->setCurrentComboIndex();

    // disable for now
    this->ui->toolBar->removeAction( this->ui->actionSort );

    // about dialog
    this->connect( this->ui->actionAbout, &QAction::triggered, [ this ]() { About about( this ); about.exec(); } );
}

/**
 * @brief MainWindow::initialise
 * @param reload
 */
void MainWindow::initialise( bool reload ) {
    // disable actions on partial initialisation (debug)
    if ( !Main::instance()->isInitialised()) {
        this->ui->actionTeams->setDisabled( true );
        this->ui->actionTasks->setDisabled( true );
        this->ui->actionEvents->setDisabled( true );
        this->ui->actionRankings->setDisabled( true );
        this->ui->comboTeams->setDisabled( true );
        this->ui->actionQuickAdd->setDisabled( true );
        this->ui->timeFinish->setDisabled( true );
        this->ui->actionLockTeam->setDisabled( true );
        this->ui->actionLogTime->setDisabled( true );
    } else {
        // set minimum for time
        this->ui->timeFinish->setMinimumTime( EventManager::instance()->active()->startTime());

        //
        this->ui->comboTeams->setModel( Main::instance()->teamModel );

        // connect team switcher and finish time editor
        if ( !reload ) {
            this->connect( this->ui->comboTeams, SIGNAL( currentIndexChanged( int )), this, SLOT( teamIndexChanged( int )));
            this->connect( this->ui->taskList, SIGNAL( currentRowChanged( int )), this, SLOT( taskIndexChanged( int )));
            this->connect( this->ui->timeFinish, SIGNAL( timeChanged( QTime )), this, SLOT( updateFinishTime( QTime )));
        }

        // fill in tasks and teams
        this->selectTeam();
        this->fillTasks();
    }

    // set title
    this->setEventTitle();

    // test visibility of the sort button
    this->testSortButton();

    // hide statusbar for now
    this->statusBar()->hide();

    // init dialogs
    this->eventDialog = new EventDialog( this );
    this->connect( this->eventDialog, SIGNAL( closeSignal( int )), this, SLOT( eventDialogClosed( int )));
    this->teamDialog = new TeamDialog( this );
    this->connect( this->teamDialog, SIGNAL( closeSignal( int )), this, SLOT( teamDialogClosed( int )));
    this->taskDialog = new TaskDialog( this );
    this->connect( this->taskDialog, SIGNAL( closeSignal( int )), this, SLOT( taskDialogClosed( int )));
    this->rankingsDialog = new Rankings( this );
    this->settingsDialog = new Settings( this );
    this->connect( this->settingsDialog, SIGNAL( closeSignal( int )), this, SLOT( settingsDialogClosed( int )));

    // announce
    qDebug() << this->tr( "initialisation complete" );

#ifdef APPLET_DEBUG
    this->ui->mainToolBar->insertAction( this->ui->actionExit, this->ui->actionConsole );
    Main::instance()->alloc = 0;
    Main::instance()->dealloc = 0;
#endif

    // prevent context menu on toolbars
    this->ui->mainToolBar->setContextMenuPolicy( Qt::PreventContextMenu );
    this->ui->toolBar->setContextMenuPolicy( Qt::PreventContextMenu );
    this->insertToolBarBreak( this->ui->toolBar );
    this->ui->toolBar->addWidget( this->ui->comboTeams );

    // add widgets to secondary toolbar
    QAction *timeFinish = this->ui->toolBar->addWidget( this->ui->timeFinish );
    this->ui->toolBar->insertAction( timeFinish, this->ui->actionLockTeam );
    this->ui->toolBar->addAction( this->ui->actionLogTime );
}

/**
 * @brief MainWindow::setEventTitle
 */
void MainWindow::setEventTitle() {
    QString reviewer;

    // this really should not happen
    if ( EventManager::instance()->active() == nullptr ) {
        this->setWindowTitle( this->tr( "Ketoevent logger" ));
        return;
    }

    // get reviewer name
    reviewer = Variable::instance()->string( "reviewerName" );

    if ( !reviewer.isEmpty())
        this->setWindowTitle( this->tr( "Ketoevent logger - %1 (%2)" ).arg( EventManager::instance()->active()->name()).arg( reviewer ));
    else
        this->setWindowTitle( this->tr( "Ketoevent logger - %1" ).arg( EventManager::instance()->active()->name()));
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    // disconnect ui elements
    this->disconnect( this->ui->comboTeams, SIGNAL( currentIndexChanged( int )));
    this->disconnect( this->ui->timeFinish, SIGNAL( timeChanged( QTime )));
    this->disconnect( this->eventDialog, SIGNAL( closeSignal( int )));
    this->disconnect( this->teamDialog, SIGNAL( closeSignal( int )));
    this->disconnect( this->taskDialog, SIGNAL( closeSignal( int )));
    this->disconnect( this->settingsDialog, SIGNAL( closeSignal( int )));

    // get rid of ui
    delete this->eventDialog;
    delete this->teamDialog;
    delete this->taskDialog;
    delete this->rankingsDialog;
    delete this->settingsDialog;
    delete this->ui;
}

/**
 * @brief MainWindow::teamIndexChanged
 * @param index
 */
void MainWindow::teamIndexChanged( int index ) {
    Team *team = Team::forId( this->ui->comboTeams->itemData( index ).toInt());
    QListWidget *lw = this->ui->taskList;
    int y;

    this->ui->comboTeams->update();

    // recalculate last team if required
    if ( this->currentTeamId() != -1 ) {
        Team *recalcPtr = Team::forId( this->currentTeamId());
        if ( recalcPtr != nullptr ) {
            if ( !recalcPtr->combosCalculated()) {
                recalcPtr->calculateCombos();
                recalcPtr->setCombosCalculated( true );
            }
        }
    }

    // disable combine button
    this->ui->actionCombine->setDisabled( true );

    if ( team != nullptr ) {
        this->ui->comboTeams->setEnabled( true );
        this->ui->actionLockTeam->setEnabled( true );

        // set time
        this->ui->timeFinish->setTime( team->finishTime());

        // clean up values
        for ( y = 0; y < lw->count(); y++ ) {
            TaskWidget *task = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( y )));
            if ( task == nullptr )
                continue;

            task->resetTeam();
        }

        // display new values
        for ( y = 0; y < lw->count(); y++ ) {
            TaskWidget *task = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( y )));
            if ( task == nullptr )
                continue;

            task->setTeam( team );

            // trigger id change
            task->comboIdChanged();
        }

        if ( team->isLocked()) {
            this->ui->actionLockTeam->setIcon( QIcon( ":/icons/unlock.png" ));
            this->ui->actionLockTeam->setText( this->tr( "Unlock" ));
            this->ui->taskList->setDisabled( true );
            this->ui->timeFinish->setDisabled( true );
            this->ui->actionLogTime->setDisabled( true );
            this->ui->actionLockTeam->setChecked( true );
        } else {
            this->ui->actionLockTeam->setIcon( QIcon( ":/icons/lock.png" ));
            this->ui->actionLockTeam->setText( this->tr( "Lock" ));
            this->ui->taskList->setEnabled( true );
            this->ui->timeFinish->setEnabled( true );
            this->ui->actionLogTime->setEnabled( true );
            this->ui->actionLockTeam->setChecked( false );
        }
        this->setCurrentTeamIndex( index );
    } else {
        this->ui->timeFinish->setDisabled( true );
        this->ui->taskList->setDisabled( true );
        this->ui->comboTeams->setDisabled( true );
        this->ui->actionLockTeam->setDisabled( true );
        this->ui->actionLogTime->setDisabled( true );

        this->setCurrentTeamIndex();
    }
}

/**
 * @brief MainWindow::taskIndexChanged checks if selected task/log can be combined and enables/disables the "combine button"
 * @param row
 */
void MainWindow::taskIndexChanged( int row ) {
    QListWidget *lw = this->ui->taskList;

    // hack to allow triggering outside class
    if ( row == -1 && this->ui->taskList->currentRow() != -1 )
        row = this->ui->taskList->currentRow();

    // get task widget
    TaskWidget *task = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( row )));
    if ( task == nullptr )
        return;

    // should get things going faster
    if ( task->task()->type() == Task::Check )
        task->check->setFocus();
    else if ( task->task()->type() == Task::Multi ) {
        task->multi->setFocus();
        task->multi->selectAll();
    }

    // get log
    Log *lPtr = task->log();

    // check for value
    if ( task->hasLog()) {
        if ( lPtr->value()) {
            // is logged - can be combined
            this->ui->actionCombine->setEnabled( true );
            return;
        }
    }

    // not logged - cannot be combined
    this->ui->actionCombine->setDisabled( true );
}

/**
 * @brief MainWindow::updateFinishTime
 * @param time
 */
void MainWindow::updateFinishTime( QTime time ) {
    Team *team = Team::forId( this->ui->comboTeams->itemData( this->ui->comboTeams->currentIndex()).toInt());
    if ( team != nullptr ) {
        if ( time == team->finishTime())
            return;

        team->setFinishTime( time );
    }
}

/**
 * @brief MainWindow::selectTeam
 * @param id
 */
void MainWindow::selectTeam( int id ) {
    int y;

    // abort if partially initialised
    if ( !Main::instance()->isInitialised())
        return;

    // select something
    if ( id == -1 ) {
        this->ui->comboTeams->setCurrentIndex( this->ui->comboTeams->count() - 1 );
        return;
    }

    // select id
    for ( y = 0; y < this->ui->comboTeams->count(); y++ )
        if ( id == this->ui->comboTeams->itemData( y, Qt::UserRole ).toInt()) {
            this->ui->comboTeams->setCurrentIndex( y );
    }
}

/**
 * @brief MainWindow::clearTasks
 */
void MainWindow::clearTasks() {
    QListWidget *lw = this->ui->taskList;

    // cannot use for loop, since items are removed from list
    while ( lw->count()) {
        TaskWidget *task = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( 0 )));
        if ( task == nullptr )
            continue;

        this->disconnect( task->combo, SIGNAL( toggled( bool )));
        delete task;
        delete lw->item( 0 );
#ifdef APPLET_DEBUG
        Main::instance()->dealloc +=2;
#endif
    }
    lw->clear();
}

/**
 * @brief MainWindow::lock
 */
void MainWindow::lock() {
    this->ui->actionAbout->setDisabled( true );
    this->ui->actionCombos->setDisabled( true );
    this->ui->actionEvents->setDisabled( true );
    this->ui->actionLogTime->setDisabled( true );
    this->ui->actionRankings->setDisabled( true );
    this->ui->actionSettings->setDisabled( true );
    this->ui->actionSort->setDisabled( true );
    this->ui->actionTasks->setDisabled( true );
    this->ui->actionTeams->setDisabled( true );
    this->ui->toolBar->removeAction( this->ui->actionQuickAdd );
    this->ui->toolBar->removeAction( this->ui->actionLockTeam );
    this->ui->toolBar->removeAction(  this->ui->actionCombine );
}

/**
 * @brief MainWindow::fillTasks
 */
void MainWindow::fillTasks() {
    QListWidget *lw = this->ui->taskList;
    QList <Task*>taskList;

    // abort if partially initialised
    if ( !Main::instance()->isInitialised())
        return;

    // clear tasks
    this->clearTasks();

    // fill with either sorted or unsorted list
    if ( Variable::instance()->isEnabled( "misc/sortTasks" )/* || m.cvar( "misc/hilightLogged" )->isEnabled()*/)
        taskList = Main::instance()->taskListSorted();
    else
        taskList = EventManager::instance()->active()->taskList;

    foreach ( Task *task, taskList ) {
        QListWidgetItem *itemPtr = new QListWidgetItem();
        itemPtr->setSizeHint( QSize( 0 , 34 ));
        lw->addItem( itemPtr );
        TaskWidget *widgetPtr = new TaskWidget( task );
        lw->setItemWidget( itemPtr, widgetPtr );

#ifdef APPLET_DEBUG
        Main::instance()->alloc +=2;
#endif
    }

    // update
    this->teamIndexChanged( this->ui->comboTeams->currentIndex());
}

/**
 * @brief MainWindow::on_clearButton_clicked
 */
void MainWindow::on_clearButton_clicked() {
    this->ui->findTaskEdit->clear();
}

/**
 * @brief MainWindow::on_findTaskEdit_textChanged
 */
void MainWindow::on_findTaskEdit_textChanged( const QString & ) {
    if ( this->ui->findTaskEdit->palette().color( QPalette::Base ) == QColor( 255, 0, 0, 64 )) {
        QPalette p( this->ui->findTaskEdit->palette());
        p.setColor( QPalette::Base, Qt::white );
        this->ui->findTaskEdit->setPalette( p );
    }
}

/**
 * @brief MainWindow::on_findTaskEdit_returnPressed
 */
void MainWindow::on_findTaskEdit_returnPressed() {
    int y;
    QString matchString;
    bool match = false;

    matchString = this->ui->findTaskEdit->text();

    // failsafe
    if ( matchString.isEmpty())
        return;

    // advance
    if ( this->currentMatch() >= this->ui->taskList->count() - 1 || this->currentMatch() < 0 )
        this->setCurrentMatch();
    else
        this->setCurrentMatch( this->currentMatch() + 1 );

    // find item from current position
    for ( y = this->currentMatch(); y < this->ui->taskList->count(); y++ ) {
        TaskWidget *task = qobject_cast<TaskWidget *>( this->ui->taskList->itemWidget( this->ui->taskList->item( y )));
        if ( task == nullptr )
            continue;

        QString taskName = task->taskName->text();

        if ( taskName.contains( matchString, Qt::CaseInsensitive )) {
            match = true;
            this->setCurrentMatch( y );
            break;
        }
    }

    // no match, try again from beginning
    if ( !match ) {
        for ( y = 0; y < this->ui->taskList->count(); y++ ) {
            TaskWidget *task = qobject_cast<TaskWidget *>( this->ui->taskList->itemWidget( this->ui->taskList->item( y )));
            if ( task == nullptr )
                continue;

            QString taskName = task->taskName->text();

            if ( taskName.contains( matchString, Qt::CaseInsensitive )) {
                match = true;
                this->setCurrentMatch( y );
                break;
            }
        }
    }

    // matched?
    if ( match ) {
        this->ui->taskList->setCurrentRow( y );
        this->ui->findTaskEdit->setFocus();
    } else {
        this->ui->taskList->clearSelection();
        QPalette p( this->ui->findTaskEdit->palette());
        p.setColor( QPalette::Base, QColor( 255, 0, 0, 64 ));
        this->ui->findTaskEdit->setPalette( p );
    }
}

/**
 * @brief MainWindow::on_actionEvents_triggered
 */
void MainWindow::on_actionEvents_triggered() {
    // store last event id
    this->setLastEventId( Variable::instance()->integer( "currentEvent" ));

    // show dialog
    this->eventDialog->show();
}

/**
 * @brief MainWindow::eventDialogClosed
 * @param signal
 */
void MainWindow::eventDialogClosed( int signal ) {
    int newEventId;

    // get current id
    newEventId = Variable::instance()->integer( "currentEvent" );

    if ( signal == Dialog::Accepted ) {
        // compare these two
        if ( newEventId != this->lastEventId()) {
            this->fillTasks();
            this->selectTeam();
        }

        this->setEventTitle();
    } else {
        EventManager::instance()->setActive( Event::forId( this->lastEventId()));
        this->fillTasks();
    }
}

/**
 * @brief MainWindow::teamDialogClosed
 */
void MainWindow::teamDialogClosed( int ) {
    if ( this->teamDialog->state() == TeamDialog::AddQuick )
        this->selectTeam( this->teamDialog->lastId());
    else
        this->selectTeam();
}

/**
 * @brief MainWindow::taskDialogClosed
 */
void MainWindow::taskDialogClosed( int ) {
    this->fillTasks();
}

/**
 * @brief MainWindow::settingsDialogClosed
 */
void MainWindow::settingsDialogClosed( int ) {
    this->selectTeam();
    this->fillTasks();
    this->setEventTitle();
    this->testSortButton();
}

/**
 * @brief MainWindow::on_actionCombos_triggered
 */
void MainWindow::on_actionCombos_triggered() {
    // construct dialog
    ComboDialog combos( this );
    combos.exec();
}

/**
 * @brief MainWindow::on_actionConsole_toggled
 * @param visible
 */
void MainWindow::on_actionConsole_toggled( bool visible ) {
#ifdef APPLET_DEBUG
    if ( visible )
        Console::show();
    else
        Console::hide();
#else
    Q_UNUSED( visible )
#endif
}

/**
 * @brief MainWindow::currentTeamId
 * @return
 */
int MainWindow::currentTeamId() {
    if ( this->currentTeamIndex() == -1 )
        return -1;

    return this->ui->comboTeams->itemData( this->currentTeamIndex()).toInt();
}

/**
 * @brief MainWindow::on_actionSettings_triggered
 */
void MainWindow::on_actionSettings_triggered() {
    this->settingsDialog->show();
}

/**
 * @brief MainWindow::testSortButton
 */
void MainWindow::testSortButton() {
    /*if ( Variable::isEnabled( "misc/sortLogged" ))
        this->ui->actionSort->setEnabled( true );
    else
        this->ui->actionSort->setDisabled( true );*/

    this->ui->actionSort->setEnabled( Variable::instance()->isEnabled( "misc/sortLogged" ));
}

//
// Stress test for debugging purposes
//
#ifdef APPLET_DEBUG

/**
 * @brief irand
 * @param min
 * @param max
 * @return
 */
static int irand( int min, int max ) {
    qsrand( static_cast<unsigned int>( QTime::currentTime().msec())
#ifdef Q_OS_UNIX
            * getpid()
#endif
            );

    if ( min > max ) {
        int temp = min;
        min = max;
        max = temp;
    }
    return (( rand() % ( max-min + 1 )) + min );
}

/**
 * @brief MainWindow::testTeam
 * @param team
 */
void MainWindow::testTeam( Team *team ) {
    int shouldBe = 0;
    int rand = 0;
    QListWidget *lw = this->ui->taskList;
    int y;

    // select the team
    this->selectTeam( team->id());

    // log random values
    for ( y = 0; y < lw->count(); y++ ) {
        TaskWidget *task = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( y )));
        if ( task == nullptr )
            continue;

        if ( task->task()->type() == Task::Check ) {
            rand = irand( 0, 1 );

            if ( rand ) {
                task->check->setChecked( static_cast<bool>( rand ));
                shouldBe += task->task()->points();
            }
        } else if ( task->task()->type() == Task::Multi ) {
            rand = irand( 0, task->task()->multi());

            if ( rand ) {
                task->multi->setValue( rand );
                shouldBe += task->task()->points() * rand;
            }
        }
    }

    // subtract penalty
    shouldBe -= team->penalty();

    // report
    team->calculateCombos();
    qInfo() << QString( "Team \"%1\" has %2 points (should be %3)" ).arg( team->name()).arg( team->points() - team->penalty()).arg( shouldBe );
}

/**
 * @brief MainWindow::stressTest
 * @param numTeams
 */
void MainWindow::stressTest( int numTeams ) {
    Team *team;
    int k;

    // clear command has been given
    if ( numTeams == -1 ) {
        foreach ( Team *team, Main::instance()->teamList ) {
            if ( team->name().startsWith( "Stress test" ))
                Team::remove( team->name());
        }
        this->selectTeam();
        return;
    } else if ( numTeams == -2 ) {
        qInfo() << ( !EventManager::instance()->active()->teamList.isEmpty() ? this->tr( "performing stress test for %1 custom teams" ).arg( EventManager::instance()->active()->teamList.count()) : this->tr( "no teams to perform stress test on" ));

        foreach ( Team *team, EventManager::instance()->active()->teamList )
            this->testTeam( team );
        return;
    }

    // limits
    if ( numTeams > 100 )
        numTeams = 100;
    else if ( numTeams < 1 )
        numTeams = 1;

    // add a few teams with random logs
    for ( k = 0; k < numTeams; k++ ) {
        int maxSeconds = EventManager::instance()->active()->startTime().secsTo( EventManager::instance()->active()->finalTime());
        QTime finishTime = EventManager::instance()->active()->startTime().addSecs( irand( 1, maxSeconds ));
        QString teamName = QString( "Stress test %1" ).arg( k );

        // remove duplicates
        if ( Team::forName( teamName ) != nullptr )
            Team::remove( teamName );

        // add a stress test team
        Team::add( teamName, irand( 1, 2 ), finishTime, "Stress Test", false );
        team = Team::forName( QString( "Stress test %1" ).arg( k ));

        if ( team != nullptr )
            this->testTeam( team );
    }
}
#endif

/**
 * @brief MainWindow::on_actionLogTime_triggered
 */
void MainWindow::on_actionLogTime_triggered() {
    this->ui->timeFinish->setTime( QTime::currentTime());
}

/**
 * @brief MainWindow::on_actionCombine_toggled
 * @param checked
 */
void MainWindow::on_actionCombine_toggled( bool checked ) {
    QListWidget *lw;
    Team *team;
    TaskWidget *tw;
    QList<int> indexList;
    bool valueSet;
    int y, k;

    // get task list (gui)
    lw = this->ui->taskList;
    tw = qobject_cast<TaskWidget *>( lw->itemWidget( lw->currentItem()));
    k = 0;

    // failsafe
    if ( tw == nullptr ) {
        this->ui->actionCombine->setChecked( false );
        this->setCurrentComboIndex();
        return;
    }

    // check if any reason to combine (>= logged tasks & active task is logged)
    if ( checked ) {
        Log *log = nullptr;

        // check if the active task is logged
        if ( !tw->hasLog()) {
            this->ui->actionCombine->setChecked( false );
            this->setCurrentComboIndex();
            return;
        }

        // check if it has a combo assigned, if not get the next free index
        if ( !tw->hasCombo())
            tw->log()->setComboId( Combo::getFreeHandle());

        this->setCurrentComboIndex( tw->log()->comboId());

        // hilight entry (setCombo or smth?)
        tw->combo->setChecked( true );

        // check for the total number of logs
        team = Team::forId( this->ui->comboTeams->itemData( this->ui->comboTeams->currentIndex()).toInt());
        if ( team != nullptr ) {
            int count = 0;

            // cannot use list->count(), since team might have orphaned logs
            foreach ( log, team->logList ) {
                if ( log->value())
                    count++;
            }

            // less than two tasks - abort!
            if ( count < 2 ) {
                this->ui->actionCombine->setChecked( false );
                this->setCurrentComboIndex();
                return;
            }
        }
    }

    int count = 0;

    // go through items one by one
    for ( y = 0; y < lw->count(); y++ ) {
        TaskWidget *task;

        // get task widget
        task = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( y )));
        if ( task == nullptr )
            continue;

        // reset value check
        valueSet = true;

        // get log
        Log *lPtr = task->log();
        // check for value
        if ( lPtr == nullptr )
            valueSet = false;
        else {
            if ( !lPtr->value())
                valueSet = false;
        }

        // make sure we remove orphaned combos
        if ( !checked && lPtr != nullptr ) {
            if ( lPtr->comboId() == this->currentComboIndex())
                count++;
            continue;
        }

        // for now, remove only widgets with no or empty logs
        // in future check combo conflicts
        if ( !valueSet )
            indexList << y;
        else {
            // switch view state and swap widgets
            task->toggleViewState( TaskWidget::Combine );

            // hilight combined entries
            if ( task->hasCombo()) {
                if ( task->log()->comboId() == this->currentComboIndex())
                    task->combo->setChecked( true );
                else
                    indexList << y;
            }
        }
    }

    if ( !checked ) {
        if ( count < 2 ) {
            for ( y = 0; y < lw->count(); y++ ) {
                TaskWidget *task;

                // get task widget
                task = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( y )));
                if ( task == nullptr )
                    continue;

                // get log
                Log *lPtr = task->log();
                // check for value
                if ( lPtr == nullptr )
                    continue;

                lPtr->setComboId( -1 );
            }
        }

        // just refill the whole list on reset
        this->fillTasks();
        this->setCurrentComboIndex();
        return;
    }

    // remove items that cannot be combined
    for ( y = 0; y < indexList.count(); y++ ) {
        QListWidgetItem *item = lw->takeItem( indexList.at( y ) - k );

        // delete the unused item
        delete item;

#ifdef APPLET_DEBUG
        Main::instance()->dealloc +=2;
#endif

        k++;
    }

    // one task cannot be combined, so disable the button
    if ( lw->count() == 1 ) {
        TaskWidget *task;

        // get the first item
        task = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( 0 )));
        if ( task != nullptr )
            task->combo->setDisabled( true );
    }
}

/**
 * @brief MainWindow::on_actionQuickAdd_triggered
 */
void MainWindow::on_actionQuickAdd_triggered() {
    this->teamDialog->toggleAddEditWidget( TeamDialog::AddQuick );
    this->teamDialog->show();
}

/**
 * @brief MainWindow::on_actionSort_triggered
 */
void MainWindow::on_actionSort_triggered() {
    this->fillTasks();
}

/**
 * @brief MainWindow::on_actionLockTeam_triggered
 */
void MainWindow::on_actionLockTeam_triggered() {
    Team *team = Team::forId( this->ui->comboTeams->itemData( this->ui->comboTeams->currentIndex()).toInt());
    if ( team == nullptr )
        return;

    if ( team->isLocked())
        team->unlock();
    else
        team->lock();

    this->teamIndexChanged( this->currentTeamIndex());
}

/**
 * @brief MainWindow::on_actionCombine_changed
 */
void MainWindow::on_actionCombine_changed() {
    if ( this->ui->actionCombine->isEnabled())
        this->ui->actionLockTeam->setDisabled( true );
    else
        this->ui->actionLockTeam->setEnabled( true );
}
