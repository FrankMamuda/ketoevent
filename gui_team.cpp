/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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
#include "gui_team.h"
#include "ui_gui_team.h"
#include "team.h"
#include "main.h"
#include <QMessageBox>
#include <QSqlQuery>
#include "gui_main.h"

/**
 * @brief Gui_Team::Gui_Team
 * @param parent
 */
Gui_Team::Gui_Team( QWidget *parent ) : Gui_Dialog( parent ), ui( new Ui::Gui_Team ) {
    ui->setupUi( this );

    // set up view
    this->listModelPtr = new Gui_TeamListModel( this );
    this->ui->teamList->setModel( listModelPtr );
    this->ui->teamList->setAlternatingRowColors( true );
    this->ui->finishTimeEdit->setMinimumTime( Event::active()->startTime());
    this->ui->teamMembersEdit->setMinimum( Event::active()->minMembers());
    this->ui->teamMembersEdit->setMaximum( Event::active()->maxMembers());

    // hide add/edit widget
    this->toggleAddEditWidget( NoState );

    // connect
    this->connect( this->ui->closeButton, SIGNAL( clicked()), this, SLOT( close()));

    // reset team id
    this->resetLastId();

    // set focus
    this->ui->closeButton->setFocus();
}

/**
 * @brief Gui_Team::enableView
 */
void Gui_Team::enableView() {
    this->ui->actionAdd->setEnabled( true );
    this->ui->actionRemove->setEnabled( true );
    this->ui->actionEdit->setEnabled( true );
    this->ui->closeButton->setEnabled( true );
    this->ui->teamList->setEnabled( true );
    this->ui->teamList->setSelectionMode( QAbstractItemView::SingleSelection );
    this->ui->closeButton->setDefault( true );
}

/**
 * @brief Gui_Team::~Gui_Team
 */
Gui_Team::~Gui_Team() {
    this->disconnect( this->ui->closeButton, SIGNAL( clicked()));
    delete ui;
    delete this->listModelPtr;
}

/**
 * @brief Gui_Team::toggleAddEditWidget
 * @param state
 */
void Gui_Team::toggleAddEditWidget( AddEditState state ) {
    this->setState( state );

    if ( !this->ui->addEditWidget->isHidden() || state == NoState ) {
        this->ui->addEditWidget->close();
        this->ui->teamList->setEnabled( true );
        this->enableView();
    } else {
        Team *teamPtr = NULL;

        // disable everything
        this->ui->addEditWidget->show();
        this->ui->actionAdd->setDisabled( true );
        this->ui->actionRemove->setDisabled( true );
        this->ui->actionEdit->setDisabled( true );
        this->ui->closeButton->setDisabled( true );
        this->ui->doneButton->setDefault( true );
        this->ui->teamList->setDisabled( true );

        switch ( state ) {
        case Add:
        case AddQuick:
        {
            this->ui->teamNameEdit->clear();
            this->ui->finishTimeEdit->setTime( Event::active()->finishTime().addSecs( -60 ));
            this->ui->teamMembersEdit->setValue( Event::active()->minMembers());
            this->ui->addEditWidget->setWindowTitle( this->tr( "Add team" ));
            this->ui->reviewerEdit->setText( Variable::string( "reviewerName" ));

            // refocus
            if ( state == Add )
                this->ui->doneButton->setFocus();
            else
                this->ui->teamNameEdit->setFocus();
        }
            break;

        case Edit:
            // match by id
            teamPtr = Team::forId( this->ui->teamList->model()->data( this->ui->teamList->currentIndex(), Qt::UserRole ).toInt());

            if ( teamPtr == NULL ) {
                this->toggleAddEditWidget( NoState );
                return;
            }

            this->ui->teamNameEdit->setText( teamPtr->name());
            this->ui->finishTimeEdit->setTime( teamPtr->finishTime());
            this->ui->teamMembersEdit->setValue( teamPtr->members());
            this->ui->reviewerEdit->setText( teamPtr->reviewer());
            this->ui->addEditWidget->setWindowTitle( this->tr( "Edit team" ));

            break;

            // bad state
        case NoState:
        default:
            // refocus
            this->ui->closeButton->setFocus();
            return;
        }
    }
}

/**
 * @brief Gui_Team::on_doneButton_clicked
 */
void Gui_Team::on_doneButton_clicked() {
    Team *teamPtr;
    QModelIndex lastIndex;

    // failsafe
    if ( this->ui->teamNameEdit->text().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText( this->tr( "Please specify team name" ));
        msgBox.setIcon( QMessageBox::Information );
        msgBox.exec();
        return;
    }

    // begin reset
    this->listModelPtr->beginReset();

    // alternate between Add/Edit states
    if ( this->state() == Add || this->state() == AddQuick ) {
        Team::add( this->ui->teamNameEdit->text(), this->ui->teamMembersEdit->value(), this->ui->finishTimeEdit->time(), this->ui->reviewerEdit->text());
        lastIndex = this->listModelPtr->index( this->listModelPtr->rowCount()-1);
    } else if ( this->state() == Edit ) {
        // match name to be sure
        teamPtr = Team::forId( this->ui->teamList->model()->data( this->ui->teamList->currentIndex(), Qt::UserRole ).toInt());

        if ( teamPtr == NULL ) {
            this->toggleAddEditWidget( NoState );
            return;
        }

        // set edited data
        teamPtr->setName( this->ui->teamNameEdit->text());
        teamPtr->setFinishTime( this->ui->finishTimeEdit->time());
        teamPtr->setMembers( this->ui->teamMembersEdit->value());
        teamPtr->setReviewer( this->ui->reviewerEdit->text());

        // get last index
        lastIndex = this->ui->teamList->currentIndex();
    }

    // quick add
    if ( this->state() == AddQuick ) {
        this->setLastId( m.teamList.last()->id());
        this->close();
    }

    // reset view
    this->toggleAddEditWidget( NoState );
    this->listModelPtr->endReset();

    // select last added/edited value
    this->ui->teamList->setCurrentIndex( lastIndex );
}

/**
 * @brief Gui_Team::on_reviewerButton_clicked
 */
void Gui_Team::on_reviewerButton_clicked() {
    this->ui->reviewerEdit->setText( Variable::string( "reviewerName" ));
}

/**
 * @brief Gui_Team::on_actionRemove_triggered
 */
void Gui_Team::on_actionRemove_triggered() {
    int state;
    Team *teamPtr = Team::forId( this->ui->teamList->model()->data( this->ui->teamList->currentIndex(), Qt::UserRole ).toInt());
    QSqlQuery query;

    if ( teamPtr != NULL ) {
        QMessageBox msgBox;
        msgBox.setText( this->tr( "Do you really want to remove \"%1\"?" ).arg( teamPtr->name()));
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
        msgBox.setDefaultButton( QMessageBox::Yes );
        msgBox.setIcon( QMessageBox::Warning );
        msgBox.setWindowIcon( QIcon( ":/icons/team_remove.png" ));
        state = msgBox.exec();

        // check options
        switch ( state ) {
        case QMessageBox::Yes:
            // begin reset
            this->listModelPtr->beginReset();

            // remove from memory
            m.teamList.removeOne( teamPtr );
            Event::active()->teamList.removeOne( teamPtr );

            // remove from database
            // fortunately teams are listed alphabetically, so there is no need to update order
            query.exec( QString( "delete from teams where id=%1" ).arg( teamPtr->id()));

            // end reset
            this->listModelPtr->endReset();
            break;

        case QMessageBox::No:
        default:
            return;
        }
    }
}

/**
 * @brief Gui_Team::closeEvent
 * @param ePtr
 */
void Gui_Team::closeEvent( QCloseEvent *ePtr ) {
    bool quick = false;

    if ( this->state() == AddQuick )
        quick = true;

    this->toggleAddEditWidget( NoState );
    ePtr->accept();

    Gui_Main *gui = qobject_cast<Gui_Main*>( this->parent());
    if ( gui != NULL ) {
        if ( quick && !Event::active()->teamList.isEmpty())
            gui->fillTeams( Event::active()->teamList.last()->id());
        else
            gui->fillTeams();
    }
}