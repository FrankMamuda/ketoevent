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

/*
================
construct
================
*/
Gui_Reviewers::Gui_Reviewers( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_Reviewers ) {
    ui->setupUi( this );
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
void Gui_Reviewers::on_addButton_clicked(){
}

/*
================
renameButton->clicked
================
*/
void Gui_Reviewers::on_renameButton_clicked(){
}

/*
================
removeButton->clicked
================
*/
void Gui_Reviewers::on_removeButton_clicked() {
}
