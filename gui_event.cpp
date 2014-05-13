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
#include "gui_event.h"
#include "ui_gui_event.h"
#include "gui_main.h"
#include "evententry.h"
#include "gui_addevent.h"
#include <QMessageBox>
#include <QSqlQuery>

/*
================
construct
================
*/
Gui_Event::Gui_Event( QWidget *parent ) : Gui_SettingsDialog( parent ), ui( new Ui::Gui_Event ) {
    // set up gui
    ui->setupUi( this );

    // bind settings vars
    if ( m.isInitialized())
        this->bindVars();

    this->fillEvents();
}

/*
================
fillEvents
================
*/
void Gui_Event::fillEvents() {
    int y = 0, id = -1;
    EventEntry *eventPtr;

    if ( this->variablesLocked())
        return;

    // clear event list
    this->ui->eventCombo->clear();

    // set last event
    eventPtr = m.eventForId( m.cvar( "currentEvent" )->integer());
    if ( eventPtr != NULL )
        id = eventPtr->id();

    // fill the combobox with events
    foreach ( EventEntry *ePtr, m.eventList ) {
        this->ui->eventCombo->addItem( ePtr->name(), ePtr->id());
        if ( ePtr->id() == id )
            this->ui->eventCombo->setCurrentIndex( y );
        y++;
    }
}

/*
================
destruct
================
*/
Gui_Event::~Gui_Event() {
    this->disconnect( this->ui->titleEdit, SIGNAL( textChanged( QString )));
    this->unbindVars();
    delete ui;
}

/*
================
bindVars
================
*/
void Gui_Event::bindVars() {
    Gui_Main *gui;

    // lock vars
    this->lockVariables();

    // bind vars
    //this->bindVariable( "key", object );
    this->bindVariable( "startTime", this->ui->startTime );
    this->bindVariable( "finishTime", this->ui->finishTime );
    this->bindVariable( "finalTime", this->ui->finalTime );
    this->bindVariable( "penalty", this->ui->penalty );
    this->bindVariable( "comboOfTwo", this->ui->sCombo );
    this->bindVariable( "comboOfThree", this->ui->dCombo );
    this->bindVariable( "comboOfFourAndMore", this->ui->tCombo );
    this->bindVariable( "minMembers", this->ui->min );
    this->bindVariable( "maxMembers", this->ui->max );
    this->bindVariable( "name", this->ui->titleEdit );

    // connect for updates
    gui = qobject_cast<Gui_Main*>( this->parent());
    if ( gui != NULL ) {
        this->connect( this->ui->titleEdit, SIGNAL( textChanged( QString )), gui, SLOT( setEventTitle( QString )));
        this->connect( this->ui->titleEdit, SIGNAL( textChanged( QString )), this, SLOT( fillEvents()));
    }

    // unlock vars
    this->lockVariables( false );
}

/*
================
buttonClose->clicked
================
*/
void Gui_Event::on_buttonClose_clicked() {
    this->validate();
    this->accept();
}

/*
================
eventCombo->currentIndexChanged
================
*/
void Gui_Event::on_eventCombo_currentIndexChanged( int index ) {
    if ( index == -1 )
        return;

    // check for errors just in case
    this->validate();

    // set current event
    EventEntry *eventPtr = m.eventForId( this->ui->eventCombo->itemData( index ).toInt());
    if ( eventPtr != NULL && eventPtr != m.currentEvent()) {
        m.setCurrentEvent( eventPtr );
        this->lockVariables();
        this->updateVars();
        this->lockVariables( false );
    }
}

/*
================
buttonAdd->clicked
================
*/
void Gui_Event::on_buttonAdd_clicked() {
    Gui_AddEvent evAdd( this );
    evAdd.exec();
}

/*
================
validate
================
*/
void Gui_Event::validate() {
    // start time must be earlier than finish/final times
    if ( this->ui->startTime->time() > this->ui->finishTime->time() || this->ui->startTime->time() > this->ui->finalTime->time())
        this->ui->startTime->setTime( this->ui->finishTime->time().addSecs( -60 ));

    // finish time might match final time, but not a minute more
    if ( this->ui->finishTime->time() > this->ui->finalTime->time())
        this->ui->finishTime->setTime( this->ui->finalTime->time());

    // minMembers <= maxMembers!
    if ( this->ui->min->value() > this->ui->max->value())
        this->ui->min->setValue( this->ui->max->value());

    // as for combos - do as you please
}

/*
================
buttonRemove->clicked
================
*/
void Gui_Event::on_buttonRemove_clicked() {
    QMessageBox msgBox;
    int state;
    EventEntry *eventPtr = m.currentEvent();
    QSqlQuery query;

    // make sure we cannot delete all events
    if ( m.eventList.count() <= 1 )
        return;

    // allow to reconsider
    msgBox.setText( this->tr( "Do you really want to remove \"%1\"?" ).arg( m.currentEvent()->name()));
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::Yes );
    msgBox.setIcon( QMessageBox::Warning );
    // TODO: change icon
    msgBox.setWindowIcon( QIcon( ":/icons/team_delete_16" ));
    state = msgBox.exec();

    // check options
    switch ( state ) {
    case QMessageBox::Yes:
        // remove from memory
        m.eventList.removeOne( m.currentEvent());
        m.setCurrentEvent( m.eventList.first());

        // remove from database
        query.exec( QString( "delete from events where id=%1" ).arg( eventPtr->id()));
        delete eventPtr;

        // reset
        this->fillEvents();
        break;

    case QMessageBox::No:
    default:
        return;
    }
}
