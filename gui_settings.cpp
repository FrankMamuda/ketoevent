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
#include <QTextStream>

/*
================
construct
================
*/
Gui_Settings::Gui_Settings( QWidget *parent ) : QDialog(parent), ui( new Ui::Gui_Settings ) {
    ui->setupUi( this );
    this->intializeVariables();

#ifdef Q_OS_ANDROID
    // android fullscreen fix
    QWidget *wPtr = qobject_cast<QWidget*>( this->parent());
    this->setGeometry( wPtr->geometry());
#endif
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
    this->addVariable( "backup/changes", SettingsVariable::SpinBox, this->ui->backupChanges );
    this->addVariable( "backup/perform", SettingsVariable::CheckBox, this->ui->backupPerform );

    // set state
    foreach ( SettingsVariable *scPtr, this->varList )
        scPtr->setState();

    // unlock cvars
    this->lockVariables( false );
}

/*
================
buttonImport->clicked
================
*/
void Gui_Settings::on_buttonImport_clicked() {
    QString path;

#ifdef Q_OS_ANDROID
    path = QFileDialog::getOpenFileName( this, this->tr( "Load log database" ), "/sdcard/", this->tr( "sqlite database (*.db)" ));
#else
    path = QFileDialog::getOpenFileName( this, this->tr( "Load log database" ), QDir::currentPath(), this->tr( "sqlite database (*.db)" ));
#endif

    // this is somewhat risky
    m.importDatabase( path );
}

/*
================
buttonExportCSV->clicked
================
*/
void Gui_Settings::on_buttonExportCSV_clicked() {
    QString path;
#ifdef Q_OS_ANDROID
    path = QFileDialog::getSaveFileName( this, this->tr( "Export tasks to csv format" ), "/sdcard/", this->tr( "CSV file (*.csv)" ));
#else
    path = QFileDialog::getSaveFileName( this, this->tr( "Export tasks to csv format" ), QDir::homePath(), this->tr( "CSV file (*.csv)" ));
#endif
    QFile csv( path );

    if ( csv.open( QFile::WriteOnly | QFile::Truncate )) {
        QTextStream out( &csv );
        out.setCodec( "UTF-8" );
        out << this->tr( "name;type;style;points;multi" )
#ifdef Q_OS_WIN
               .append( "\r" )
#endif
               .append( "\n" );
        foreach ( TaskEntry *taskPtr, m.taskList ) {
            out << QString( "%1;%2;%3;%4;%5%6" )
                   .arg( taskPtr->name())
                   .arg( taskPtr->type())
                   .arg( taskPtr->style())
                   .arg( taskPtr->points())
                   .arg( taskPtr->multi())
#ifdef Q_OS_WIN
                   .arg( "\r\n" );
#else
                   .arg( "\n" );
#endif
        }
    }
    csv.close();
}

/*
================
buttonExport->clicked
================
*/
void Gui_Settings::on_buttonExport_clicked() {
    QString path;
#ifdef Q_OS_ANDROID
    path = QFileDialog::getSaveFileName( this, this->tr( "Export database" ), "/sdcard/", this->tr( "Database (*.db)" ));
#else
    path = QFileDialog::getSaveFileName( this, this->tr( "Export database" ), QDir::homePath(), this->tr( "Database (*.db)" ));
#endif
    QFile::copy( m.databasePath, path );
}

/*
================
backupPerform->stateChanged
================
*/
void Gui_Settings::on_backupPerform_stateChanged( int state ) {
    if ( state == Qt::Checked )
        this->ui->backupChanges->setEnabled( true );
    else
        this->ui->backupChanges->setDisabled( true );
}