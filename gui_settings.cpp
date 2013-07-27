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
#include "gui_settings.h"
#include "ui_gui_settings.h"
#include <QFileDialog>

/*
================
construct
================
*/
Gui_Settings::Gui_Settings( QWidget *parent ) : QDialog(parent), ui( new Ui::Gui_Settings ) {
    ui->setupUi( this );
    this->intializeVariables();
}

/*
================
destruct
================
*/
Gui_Settings::~Gui_Settings() {
    delete ui;
}

/*
================
intializeVariables
================
*/
void Gui_Settings::intializeVariables() {
    // lock cvars
    this->lockVariables();

    // set default values
    this->addVariable( "time/start", SettingsVariable::TimeEdit, this->ui->startTime );
    this->addVariable( "time/finish", SettingsVariable::TimeEdit, this->ui->finishTime );
    this->addVariable( "time/final", SettingsVariable::TimeEdit, this->ui->finalTime );
    this->addVariable( "penaltyMultiplier", SettingsVariable::SpinBox, this->ui->penalty );
    this->addVariable( "combo/single", SettingsVariable::SpinBox, this->ui->sCombo );
    this->addVariable( "combo/double", SettingsVariable::SpinBox, this->ui->dCombo );
    this->addVariable( "combo/triple", SettingsVariable::SpinBox, this->ui->tCombo );
    this->addVariable( "members/min", SettingsVariable::SpinBox, this->ui->min );
    this->addVariable( "members/max", SettingsVariable::SpinBox, this->ui->max );

    // set state
    foreach ( SettingsVariable *scPtr, this->varList )
        scPtr->setState();

    // unlock cvars
    this->lockVariables( false );
}

/*
================
updateDrunkMode
================
*/
void Gui_Settings::updateDrunkMode( const QString &, const QString &value ) {
    if ( static_cast<bool>( value.toInt())) {
        this->ui->startTime->setDisabled( true );
        this->ui->finishTime->setDisabled( true );
        this->ui->finalTime->setDisabled( true );
        this->ui->penalty->setDisabled( true );
        this->ui->sCombo->setDisabled( true );
        this->ui->dCombo->setDisabled( true );
        this->ui->tCombo->setDisabled( true );
        this->ui->min->setDisabled( true );
        this->ui->max->setDisabled( true );
        this->ui->buttonImport->setDisabled( true );
        //this->ui->buttonExport->setDisabled( true );
    } else {
        this->ui->startTime->setEnabled( true );
        this->ui->finishTime->setEnabled( true );
        this->ui->finalTime->setEnabled( true );
        this->ui->penalty->setEnabled( true );
        this->ui->sCombo->setEnabled( true );
        this->ui->dCombo->setEnabled( true );
        this->ui->tCombo->setEnabled( true );
        this->ui->min->setEnabled( true );
        this->ui->max->setEnabled( true );
        this->ui->buttonImport->setEnabled( true );
        //this->ui->buttonExport->setEnabled( true );
    }
}

/*
================
buttonImport->clicked
================
*/
void Gui_Settings::on_buttonImport_clicked() {
    //
    // TODO: update gui!!!!
    //

    // this is somewhat risky
    QString filename = QFileDialog::getOpenFileName( this, this->tr( "Load log database" ), QDir::currentPath(), this->tr( "sqlite database (*.db)" ));
    m.importDatabase( filename );
}
