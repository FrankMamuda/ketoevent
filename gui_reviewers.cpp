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
#include "gui_reviewers.h"
#include "ui_gui_reviewers.h"
#include "gui_addedit.h"
#include <QMessageBox>
#include <QSqlQuery>

/*
================
construct
================
*/
Gui_Reviewers::Gui_Reviewers( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_Reviewers ) {
    ui->setupUi( this );

    // set up view
    this->listModelPtr = new Gui_ReviewerModel( this );
    this->ui->reviewerList->setModel( listModelPtr );
    this->ui->reviewerList->setAlternatingRowColors( true );
}

/*
================
destruct
================
*/
Gui_Reviewers::~Gui_Reviewers() {
    delete ui;
}

/*
================
closeButton->clicked
================
*/
void Gui_Reviewers::on_closeButton_clicked() {
    this->accept();
}

/*
================
addButton->clicked
================
*/
void Gui_Reviewers::on_addButton_clicked() {
    Gui_AddEdit evAdd( Gui_AddEdit::ReviewerDialog, Gui_AddEdit::Add, -1, this );

    // exec and reset data just in case
    evAdd.exec();
    this->listModelPtr->beginReset();
    this->listModelPtr->endReset();
}

/*
================
renameButton->clicked
================
*/
void Gui_Reviewers::on_renameButton_clicked() {
    // match by id
    ReviewerEntry *reviewerPtr = m.reviewerForId( this->ui->reviewerList->model()->data( this->ui->reviewerList->currentIndex(), Qt::UserRole ).toInt());
    if ( reviewerPtr == NULL )
        return;

    // construct dialog
    Gui_AddEdit rwAdd( Gui_AddEdit::ReviewerDialog, Gui_AddEdit::Edit, reviewerPtr->id(), this );
    rwAdd.exec();
}

/*
================
removeButton->clicked
================
*/
void Gui_Reviewers::on_removeButton_clicked() {
    QMessageBox msgBox;
    int state;
    ReviewerEntry *reviewerPtr;
    QSqlQuery query;

    // make sure we cannot delete all reviewers
    if ( m.base.reviewerList.count() == 1 ) {
        QMessageBox::warning( this, this->tr( "Reviewer" ), this->tr( "Cannot remove the last reviewer" ));
        return;
    }

    // get reviewer
    reviewerPtr = m.reviewerForId( this->ui->reviewerList->model()->data( this->ui->reviewerList->currentIndex(), Qt::UserRole ).toInt());
    if ( reviewerPtr == NULL )
        return;

    // allow to reconsider
    msgBox.setText( this->tr( "Do you really want to remove \"%1\"?" ).arg( reviewerPtr->name()));
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::Yes );
    msgBox.setIcon( QMessageBox::Warning );
    msgBox.setWindowIcon( QIcon( ":/icons/reviewer_remove_16" ));
    state = msgBox.exec();

    // check options
    switch ( state ) {
    case QMessageBox::Yes:
        // begin reset
        this->listModelPtr->beginReset();

        // remove from memory
        m.base.reviewerList.removeOne( reviewerPtr );

        // remove from database
        query.exec( QString( "delete from reviewers where id=%1" ).arg( reviewerPtr->id()));
        delete reviewerPtr;

        // end reset
        this->listModelPtr->endReset();
        break;

    case QMessageBox::No:
    default:
        return;
    }
}
