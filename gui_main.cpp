/*
===========================================================================
Copyright (C) 2013 Avotu Briezhaudzetava

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
#include <QVariant>
#include "taskwidget.h"

/*
================
construct
================
*/
Gui_Main::Gui_Main( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::Gui_Main ) {
    ui->setupUi(this);
}

/*
================
initialize
================
*/
void Gui_Main::initialize() {
    // set minimum for time
    this->ui->timeFinish->setMinimumTime( m.var( "time/start" )->time());

    // connect team switcher and finish time editor
    this->connect( this->ui->comboTeams, SIGNAL( currentIndexChanged( int )), this, SLOT( teamIndexChanged( int )));
    this->connect( this->ui->timeFinish, SIGNAL( timeChanged( QTime )), this, SLOT( updateFinishTime( QTime )));

    // fill in tasks and teams
    this->fillTeams();
    this->fillTasks();
}

/*
================
destruct
================
*/
Gui_Main::~Gui_Main() {
    this->disconnect( this->ui->comboTeams, SIGNAL( currentIndexChanged( int )));
    this->disconnect( this->ui->timeFinish, SIGNAL( timeChanged( QTime )));
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

    if ( teamPtr != NULL ) {
        this->ui->timeFinish->setEnabled( true );
        this->ui->taskList->setEnabled( true );
        this->ui->comboTeams->setEnabled( true );

        // set time
        this->ui->timeFinish->setTime( teamPtr->finishTime());

        // clean up values
        for ( y = 0; y < lw->count(); y++ ) {
            TaskWidget *taskLogPtr = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( y )));
            taskLogPtr->resetTeam();
        }

        // display new values
        for ( y = 0; y < lw->count(); y++ ) {
            TaskWidget *taskLogPtr = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( y )));
            taskLogPtr->setTeam( teamPtr );
        }
    } else {
        this->ui->timeFinish->setDisabled( true );
        this->ui->taskList->setDisabled( true );
        this->ui->comboTeams->setDisabled( true );
    }
}

/*
================
updateFinishTime
================
*/
void Gui_Main::updateFinishTime( QTime time ) {
    TeamEntry *teamPtr = m.teamForId( this->ui->comboTeams->itemData( this->ui->comboTeams->currentIndex()).toInt());
    if ( teamPtr != NULL )
        if ( time == teamPtr->finishTime())
            return;

    teamPtr->setFinishTime( time );
}

/*
================
closeEvent
================
*/
void Gui_Main::closeEvent( QCloseEvent *eventPtr ) {
    m.shutdown();
    QWidget::closeEvent( eventPtr );
}

/*
================
fillTeams
================
*/
void Gui_Main::fillTeams() {
    this->ui->comboTeams->clear();

    foreach ( TeamEntry *teamPtr, m.teamList )
        this->ui->comboTeams->addItem( teamPtr->name(), teamPtr->id());
}

/*
================
fillTasks
================
*/
void Gui_Main::fillTasks() {
    int y;
    QListWidget *lw = this->ui->taskList;

    for ( y = 0; y < lw->count(); y++ ) {
        TaskWidget *taskPtr = qobject_cast<TaskWidget *>( lw->itemWidget( lw->item( y )));
        delete taskPtr;
        delete lw->item( y );
    }
    lw->clear();
    foreach ( TaskEntry *taskPtr, m.taskList ) {
        QListWidgetItem *itemPtr = new QListWidgetItem();
        itemPtr->setSizeHint( QSize( 0 , 34 ));
        lw->addItem( itemPtr );
        TaskWidget *widgetPtr = new TaskWidget( taskPtr );
        lw->setItemWidget( itemPtr, widgetPtr );
    }

    // update
    this->teamIndexChanged( this->ui->comboTeams->currentIndex());
}

/*
================
actionExit->triggered
================
*/
void Gui_Main::on_actionExit_triggered() {
    m.shutdown();
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
    this->fillTeams();
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
    if ( this->currentMatch >= this->ui->taskList->count() - 1 || this->currentMatch <= 0 )
        this->currentMatch = 0;
    else
        this->currentMatch++;

    // find item from current position
    for ( y = this->currentMatch; y < this->ui->taskList->count(); y++ ) {
        if ( m.taskList.at( y )->name().contains( matchString, Qt::CaseInsensitive )) {
            match = true;
            currentMatch = y;
            break;
        }
    }

    // no match, try again from beginning
    if ( !match ) {
        for ( y = 0; y < this->ui->taskList->count(); y++ ) {
            if ( m.taskList.at( y )->name().contains( matchString, Qt::CaseInsensitive )) {
                match = true;
                currentMatch = y;
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
