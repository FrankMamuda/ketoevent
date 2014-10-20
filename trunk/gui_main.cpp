/*
===========================================================================
Copyright (C) 2013-2014 Avotu Briezhaudzetava

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

//
// includes
//
#include "main.h"
#include "gui_main.h"
#include "ui_gui_main.h"
#include "taskwidget.h"
#include "gui_event.h"
#include "gui_combos.h"
#include <QMessageBox>

/*
================
construct
================
*/
Gui_Main::Gui_Main( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::Gui_Main ) {
    ui->setupUi( this );
    this->setCurrentComboIndex();
}

/*
================
initialize
================
*/
void Gui_Main::initialize( bool reload ) {
    // disable actions on partial initialization (debug)
    if ( !m.isInitialized()) {
        this->ui->actionTeams->setDisabled( true );
        this->ui->actionTasks->setDisabled( true );
        this->ui->actionEvents->setDisabled( true );
        this->ui->actionRankings->setDisabled( true );
        this->ui->comboTeams->setDisabled( true );
        this->ui->quickAddButton->setDisabled( true );
        this->ui->timeFinish->setDisabled( true );
        this->ui->lockButton->setDisabled( true );
        this->ui->logButton->setDisabled( true );
    } else {
        // set minimum for time
        this->ui->timeFinish->setMinimumTime( m.currentEvent()->startTime());

        // connect team switcher and finish time editor
        if ( !reload ) {
            this->connect( this->ui->comboTeams, SIGNAL( currentIndexChanged( int )), this, SLOT( teamIndexChanged( int )));
            this->connect( this->ui->taskList, SIGNAL( currentRowChanged( int )), this, SLOT( taskIndexChanged( int )));
            this->connect( this->ui->timeFinish, SIGNAL( timeChanged( QTime )), this, SLOT( updateFinishTime( QTime )));
        }

        // fill in tasks and teams
        this->fillTeams();
        this->fillTasks();
    }

    // set title
    this->setEventTitle();

    // test visibility of the sort button
    this->testSortButton();

    // hide statusbar for now
    this->statusBar()->hide();

    // announce
    this->print( this->tr( "initialization complete\n" ));

#ifdef APPLET_DEBUG
    this->ui->mainToolBar->insertAction( this->ui->actionExit, this->ui->actionConsole );
    m.alloc = 0;
    m.dealloc = 0;
#endif
}

/*
============
setEventTitle
============
*/
void Gui_Main::setEventTitle() {
    QString reviewer;

    // this really should not happen
    if ( m.currentEvent() == NULL ) {
        this->setWindowTitle( this->tr( "Ketoevent logger" ));
        return;
    }

    // get reviewer name
    reviewer = m.cvar( "reviewerName" )->string();

    if ( !reviewer.isEmpty())
        this->setWindowTitle( this->tr( "Ketoevent logger - %1 (%2)" ).arg( m.currentEvent()->name()).arg( reviewer ));
    else
        this->setWindowTitle( this->tr( "Ketoevent logger - %1" ).arg( m.currentEvent()->name()));
}

/*
================
destruct
================
*/
Gui_Main::~Gui_Main() {
    // disconnect ui elements
    this->disconnect( this->ui->comboTeams, SIGNAL( currentIndexChanged( int )));
    this->disconnect( this->ui->timeFinish, SIGNAL( timeChanged( QTime )));

    // get rid of ui
    delete ui;
}

/*
================
teamIndexChanged
================
*/
void Gui_Main::teamIndexChanged( int index ) {
    TeamEntry *teamPtr = m.teamForId( this->ui->comboTeams->itemData( index ).toInt());
    QListWidget *lw = this->ui->taskList;
    int y;

    // recalculate last team if required
    if ( this->currentTeamId() != -1 ) {
        TeamEntry *recalcPtr = m.teamForId( this->currentTeamId());
        if ( recalcPtr != NULL ) {
            if ( !recalcPtr->combosCalculated()) {
                recalcPtr->calculateCombos();
                recalcPtr->setCombosCalculated( true );
                m.print( QString( "recalc %1\n" ).arg( recalcPtr->name()), Main::System );
            }
        }
    }

    // disable combine button
    this->ui->combineButton->setDisabled( true );

    if ( teamPtr != NULL ) {
        this->ui->comboTeams->setEnabled( true );
        this->ui->lockButton->setEnabled( true );

        // set time
        this->ui->timeFinish->setTime( teamPtr->finishTime());

        // clean up values
        for ( y = 0; y < lw->count(); y++ ) {
            TaskWidget *taskPtr = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( y )));
            if ( taskPtr == NULL )
                continue;

            taskPtr->resetTeam();
        }

        // display new values
        for ( y = 0; y < lw->count(); y++ ) {
            TaskWidget *taskPtr = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( y )));
            if ( taskPtr == NULL )
                continue;

            taskPtr->setTeam( teamPtr );

            // trigger id change
            taskPtr->comboIdChanged();
        }

        if ( teamPtr->isLocked()) {
            this->ui->lockButton->setIcon( QIcon( ":/icons/unlocked_16" ));
            this->ui->lockButton->setText( this->tr( "Unlock" ));
            this->ui->taskList->setDisabled( true );
            this->ui->timeFinish->setDisabled( true );
            this->ui->logButton->setDisabled( true );
        } else {
            this->ui->lockButton->setIcon( QIcon( ":/icons/locked_16" ));
            this->ui->lockButton->setText( this->tr( "Lock" ));
            this->ui->taskList->setEnabled( true );
            this->ui->timeFinish->setEnabled( true );
            this->ui->logButton->setEnabled( true );
        }
        this->setCurrentTeamIndex( index );
    } else {
        this->ui->timeFinish->setDisabled( true );
        this->ui->taskList->setDisabled( true );
        this->ui->comboTeams->setDisabled( true );
        this->ui->lockButton->setDisabled( true );
        this->ui->logButton->setDisabled( true );

        this->setCurrentTeamIndex();
    }
}

/*
================
taskIndexChanged

  checks if selected task/log can be combined and
  enables/disables the "combine button"
================
*/
void Gui_Main::taskIndexChanged( int row ) {
    QListWidget *lw = this->ui->taskList;

    // hack to allow triggering outside class
    if ( row == -1 && this->ui->taskList->currentRow() != -1 )
        row = this->ui->taskList->currentRow();

    // get task widget
    TaskWidget *taskPtr = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( row )));
    if ( taskPtr == NULL )
        return;

    // should get things going faster
    if ( taskPtr->task()->type() == TaskEntry::Check )
        taskPtr->check->setFocus();
    else if ( taskPtr->task()->type() == TaskEntry::Multi ) {
        taskPtr->multi->setFocus();
        taskPtr->multi->selectAll();
    }

    // get log
    LogEntry *lPtr = taskPtr->log();

    // check for value
    if ( taskPtr->hasLog()) {
        if ( lPtr->value()) {
            // is logged - can be combined
            this->ui->combineButton->setEnabled( true );
            return;
        }
    }

    // not logged - cannot be combined
    this->ui->combineButton->setDisabled( true );
}

/*
================
updateFinishTime
================
*/
void Gui_Main::updateFinishTime( QTime time ) {
    TeamEntry *teamPtr = m.teamForId( this->ui->comboTeams->itemData( this->ui->comboTeams->currentIndex()).toInt());
    if ( teamPtr != NULL ) {
        if ( time == teamPtr->finishTime())
            return;

        teamPtr->setFinishTime( time );
    }
}

/*
================
fillTeams
================
*/
void Gui_Main::fillTeams( int forcedId ) {
    int lastId = forcedId;

    // abort if partially initialized
    if ( !m.isInitialized())
        return;

    // store last team id
    if ( this->ui->comboTeams->count() && forcedId == -1 ) {
        TeamEntry *teamPtr = m.teamForId( this->ui->comboTeams->itemData( this->ui->comboTeams->currentIndex()).toInt());
        if ( teamPtr != NULL )
            lastId = teamPtr->id();
    }

    // clear list
    this->ui->comboTeams->clear();

    // repopulate list
    foreach ( TeamEntry *teamPtr, m.currentEvent()->teamList ) {
        this->ui->comboTeams->addItem( teamPtr->name(), teamPtr->id());

        // resore last id if any
        if ( lastId != -1 && lastId == teamPtr->id())
            this->ui->comboTeams->setCurrentIndex( this->ui->comboTeams->count()-1 );
    }
}

/*
================
clearTasks
================
*/
void Gui_Main::clearTasks() {
    QListWidget *lw = this->ui->taskList;

    // cannot use for loop, since items are removed from list
    while ( lw->count()) {
        TaskWidget *taskPtr = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( 0 )));
        if ( taskPtr == NULL )
            continue;

        this->disconnect( taskPtr->combo, SIGNAL( toggled( bool )));
        delete taskPtr;
        delete lw->item( 0 );
#ifdef APPLET_DEBUG
        m.dealloc +=2;
#endif
    }
    lw->clear();
}

/*
================
fillTasks
================
*/
void Gui_Main::fillTasks() {
    QListWidget *lw = this->ui->taskList;
    QList <TaskEntry*>taskList;

    // abort if partially initialized
    if ( !m.isInitialized())
        return;

    // clear tasks
    this->clearTasks();

    // fill with either sorted or unsorted list
    if ( m.cvar( "misc/sortTasks" )->isEnabled() || m.cvar( "misc/hilightLogged" )->isEnabled())
        taskList = m.taskListSorted();
    else
        taskList = m.currentEvent()->taskList;

    foreach ( TaskEntry *taskPtr, taskList ) {
        QListWidgetItem *itemPtr = new QListWidgetItem();
        itemPtr->setSizeHint( QSize( 0 , 34 ));
        lw->addItem( itemPtr );
        TaskWidget *widgetPtr = new TaskWidget( taskPtr );
        lw->setItemWidget( itemPtr, widgetPtr );
        widgetPtr->combo->hide();
#ifdef APPLET_DEBUG
        m.alloc +=2;
#endif
    }

    // update
    this->teamIndexChanged( this->ui->comboTeams->currentIndex());
}

/*
================
logButton->clicked
================
*/
void Gui_Main::on_logButton_clicked() {
    this->ui->timeFinish->setTime( QTime::currentTime());
}

/*
================
quickAddButton->clicked
================
*/
void Gui_Main::on_quickAddButton_clicked() {
    Gui_TeamEdit teamEdit( this );
    teamEdit.toggleAddEditWidget( Gui_TeamEdit::AddQuick );
    teamEdit.exec();
    this->fillTeams( teamEdit.lastId());
}

/*
================
clearButton->clicked
================
*/
void Gui_Main::on_clearButton_clicked() {
    this->ui->findTaskEdit->clear();
}

/*
================
findTaskEdit->textChanged
================
*/
void Gui_Main::on_findTaskEdit_textChanged( const QString & ) {
    if ( this->ui->findTaskEdit->palette().color( QPalette::Base ) == QColor( 255, 0, 0, 64 )) {
        QPalette p( this->ui->findTaskEdit->palette());
        p.setColor( QPalette::Base, Qt::white );
        this->ui->findTaskEdit->setPalette( p );
    }
}

/*
================
findTaskEdit->returnPressed
================
*/
void Gui_Main::on_findTaskEdit_returnPressed() {
    int y;
    QString matchString;
    bool match = false;

    matchString = this->ui->findTaskEdit->text();

    // failsafe
    if ( matchString.isEmpty())
        return;

    // advance
    if ( this->currentMatch() >= this->ui->taskList->count() - 1 || this->currentMatch() <= 0 )
        this->setCurrentMatch();
    else
        this->setCurrentMatch( this->currentMatch() + 1 );

    // find item from current position
    for ( y = this->currentMatch(); y < this->ui->taskList->count(); y++ ) {
        if ( m.currentEvent()->taskList.at( y )->name().contains( matchString, Qt::CaseInsensitive )) {
            match = true;
            this->setCurrentMatch( y );
            break;
        }
    }

    // no match, try again from beginning
    if ( !match ) {
        for ( y = 0; y < this->ui->taskList->count(); y++ ) {
            if ( m.currentEvent()->taskList.at( y )->name().contains( matchString, Qt::CaseInsensitive )) {
                match = true;
                this->setCurrentMatch( y );
                break;
            }
        }
    }

    // matched?
    if ( match ) {
        this->ui->taskList->setCurrentRow( y );
    } else {
        this->ui->taskList->clearSelection();
        QPalette p( this->ui->findTaskEdit->palette());
        p.setColor( QPalette::Base, QColor( 255, 0, 0, 64 ));
        this->ui->findTaskEdit->setPalette( p );
    }
}

/*
================
lockButton->clicked
================
*/
void Gui_Main::on_lockButton_clicked() {
    TeamEntry *teamPtr = m.teamForId( this->ui->comboTeams->itemData( this->ui->comboTeams->currentIndex()).toInt());
    if ( teamPtr == NULL )
        return;

    if ( teamPtr->isLocked()) {
        teamPtr->unlock();
        this->ui->lockButton->setIcon( QIcon( ":/icons/locked_16" ));
        this->ui->lockButton->setText( this->tr( "Lock" ));
        this->ui->timeFinish->setEnabled( true );
        this->ui->taskList->setEnabled( true );
        this->ui->logButton->setEnabled( true );
        //this->ui->combineButton->setEnabled( true );
    } else {
        teamPtr->lock();
        this->ui->lockButton->setIcon( QIcon( ":/icons/unlocked_16" ));
        this->ui->lockButton->setText( this->tr( "Unlock" ));
        this->ui->taskList->setDisabled( true );
        this->ui->timeFinish->setDisabled( true );
        this->ui->logButton->setDisabled( true );
        this->ui->combineButton->setDisabled( true );
    }
}

/*
================
actionEvents->triggered
================
*/
void Gui_Main::on_actionEvents_triggered() {
    int currentEventId, newEventId;

    // store last event id
    currentEventId = m.cvar( "currentEvent" )->integer();

    // construct dialog
    Gui_Event events( this );
    events.exec();

    // get current id
    newEventId = m.cvar( "currentEvent" )->integer();

    // compare these two
    if ( newEventId != currentEventId ) {
        this->fillTasks();
        this->fillTeams();
    }

    if ( events.importPerformed()) {
        QMessageBox::warning( this, this->tr( "Database import" ), this->tr( "Database import requires restart" ));
        m.shutdown();
    } else
        this->setEventTitle();
}

/*
================
actionCombos->triggered
================
*/
void Gui_Main::on_actionCombos_triggered() {
    // construct dialog
    Gui_Combos combos( this );
    combos.exec();
}

/*
================
combineButton->toggled
================
*/
void Gui_Main::on_combineButton_toggled( bool checked ) {
    QListWidget *lw;
    TeamEntry *teamPtr;
    TaskWidget *tw;
    QList<int> indexList;
    bool valueSet;
    int y, k;

    // get task list (gui)
    lw = this->ui->taskList;
    tw = qobject_cast<TaskWidget *>( lw->itemWidget( lw->currentItem()));
    k = 0;

    // failsafe
    if ( tw == NULL ) {
        this->ui->combineButton->setChecked( false );
        this->setCurrentComboIndex();
        return;
    }

    // check if any reason to combine (>= logged tasks & active task is logged)
    if ( checked ) {
        LogEntry *logPtr = NULL;

        // check if the active task is logged
        if ( !tw->hasLog()) {
            this->ui->combineButton->setChecked( false );
            this->setCurrentComboIndex();
            return;
        }

        // check if it has a combo assigned, if not get the next free index
        if ( !tw->hasCombo())
            tw->log()->setComboId( m.getFreeComboHandle());

        this->setCurrentComboIndex( tw->log()->comboId());

        // hilight entry (setCombo or smth?)
        tw->combo->setChecked( true );

        // check for the total number of logs
        teamPtr = m.teamForId( this->ui->comboTeams->itemData( this->ui->comboTeams->currentIndex()).toInt());
        if ( teamPtr != NULL ) {
            int count = 0;

            // cannot use list->count(), since team might have orphaned logs
            foreach ( logPtr, teamPtr->logList ) {
                if ( logPtr->value())
                    count++;
            }

            // less than two tasks - abort!
            if ( count < 2 ) {
                this->ui->combineButton->setChecked( false );
                this->setCurrentComboIndex();
                return;
            }
        }

    }

    int count = 0;

    // go through items one by one
    for ( y = 0; y < lw->count(); y++ ) {
        TaskWidget *taskPtr;

        // get task widget
        taskPtr = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( y )));
        if ( taskPtr == NULL )
            continue;

        // reset value check
        valueSet = true;

        // get log
        LogEntry *lPtr = taskPtr->log();
        // check for value
        if ( lPtr == NULL )
            valueSet = false;
        else {
            if ( !lPtr->value())
                valueSet = false;
        }

        // make sure we remove orphaned combos
        if ( !checked && lPtr != NULL ) {
            if ( lPtr->comboId() == this->currentComboIndex())
                count++;
            continue;
        }

        // for now, remove only widgets with no or empty logs
        // in future check combo conflicts
        if ( !valueSet )
            indexList << y;
        else {
            // enable combo button
            taskPtr->combo->show();

            // disallow modifications
            if ( taskPtr->task()->type() == TaskEntry::Check ) {
                if ( taskPtr->check != NULL )
                    taskPtr->check->setDisabled( true );
            } else if ( taskPtr->task()->type() == TaskEntry::Multi ) {
                if ( taskPtr->multi != NULL )
                    taskPtr->multi->setDisabled( true );
            }

            // hilight combined entries
            if ( taskPtr->hasCombo()) {
                if ( taskPtr->log()->comboId() == this->currentComboIndex())
                    taskPtr->combo->setChecked( true );
                else
                    indexList << y;
            }
        }
    }

    if ( !checked ) {
        if ( count < 2 ) {
            for ( y = 0; y < lw->count(); y++ ) {
                TaskWidget *taskPtr;

                // get task widget
                taskPtr = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( y )));
                if ( taskPtr == NULL )
                    continue;

                // get log
                LogEntry *lPtr = taskPtr->log();
                // check for value
                if ( lPtr == NULL )
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
        lw->takeItem( indexList.at( y ) - k );
        k++;
    }

    // one task cannot be combined, so disable the button
    if ( lw->count() == 1 ) {
        TaskWidget *taskPtr;

        // get the first item
        taskPtr = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( 0 )));
        if ( taskPtr != NULL )
            taskPtr->combo->setDisabled( true );
    }
}

/*
================
updateStatusBar
================
*/
void Gui_Main::updateStatusBar() {
#if 0
    TeamEntry *teamPtr = m.teamForId( this->ui->comboTeams->itemData( this->ui->comboTeams->currentIndex()).toInt());
    if ( teamPtr != NULL )
        this->statusBar()->showMessage( this->tr( "Tasks - %1, combos - %2 (%3), points - %3" ).arg( teamPtr->logList.count()));
    else
        this->statusBar()->clearMessage();
#endif
}

/*
================
actionConsole->toggled
================
*/
void Gui_Main::on_actionConsole_toggled( bool visible ) {
    if ( visible )
        m.showConsole();
    else
        m.hideConsole();
}

/*
================
currentTeamId
================
*/
int Gui_Main::currentTeamId() {
    if ( this->currentTeamIndex() == -1 )
        return -1;

    return this->ui->comboTeams->itemData( this->currentTeamIndex()).toInt();
}

/*
================
sortButton->clicked
================
*/
void Gui_Main::on_sortButton_clicked() {
    this->fillTasks();
}

/*
================
actionSettings->triggered
================
*/
void Gui_Main::on_actionSettings_triggered() {
    Gui_Settings settings( this );
    settings.exec();
    this->fillTeams();
    this->fillTasks();
    this->setEventTitle();
    this->testSortButton();
}

/*
================
testSortButton
================
*/
void Gui_Main::testSortButton() {
   if ( m.cvar( "misc/sortLogged" )->isEnabled())
       this->ui->sortButton->setEnabled( true );
   else
       this->ui->sortButton->setDisabled( true );
}

/*
================
stressTest

 TODO: add should-be values (calculated on the fly)
       add combos?
================
*/
#ifdef APPLET_DEBUG

static int irand( int min, int max ) {
    qsrand( QTime::currentTime().msec() * getpid());

    if ( min > max ) {
        int temp = min;
        min = max;
        max = temp;
    }
    return (( rand() % ( max-min + 1 )) + min );
}

void Gui_Main::stressTest( int numTeams ) {
    TeamEntry *teamPtr;
    QListWidget *lw = this->ui->taskList;
    int y, k;

    // clear command has been given
    if ( numTeams == -1 ) {
        foreach ( TeamEntry *teamPtr, m.base.teamList ) {
            if ( teamPtr->name().startsWith( "Stress test" ))
                m.removeTeam( teamPtr->name());
        }
        this->fillTeams();
        return;
    }

    // limits
    if ( numTeams > 100 )
        numTeams = 100;
    else if ( numTeams < 1 )
        numTeams = 1;

    // add a few teams with random logs
    for ( k = 0; k < numTeams; k++ ) {
        int maxSeconds = m.currentEvent()->startTime().secsTo( m.currentEvent()->finalTime());
        QTime finishTime = m.currentEvent()->startTime().addSecs( irand( 1, maxSeconds ));

        // add a stress test team
        m.addTeam( QString( "Stress test %1" ).arg( k ), irand( 1, 2 ), finishTime, "Stress Test", false );
        teamPtr = m.teamForName( QString( "Stress test %1" ).arg( k ));

        if ( teamPtr != NULL ) {
            // select the team
            this->fillTeams( teamPtr->id());

            // log random values
            for ( y = 0; y < lw->count(); y++ ) {
                TaskWidget *taskPtr = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( y )));
                if ( taskPtr == NULL )
                    continue;

                if ( taskPtr->task()->type() == TaskEntry::Check )
                    taskPtr->check->setChecked( irand( 0, 1 ));
                else if ( taskPtr->task()->type() == TaskEntry::Multi )
                    taskPtr->multi->setValue( irand( 0, taskPtr->task()->multi()));
            }

            // report
            teamPtr->calculateCombos();
            m.print( QString( "Team \"%1\" has %2 points" ).arg( teamPtr->name()).arg( teamPtr->points() - teamPtr->penalty()), Main::System );
        }
    }
}

#endif