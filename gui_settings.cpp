/*
===========================================================================
Copyright (C) 2009-2012 Edd 'Double Dee' Psycho

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
// NOTE: using a modified YPlatorm2 (r25) code
//

//
// includes
//
#include "gui_settings.h"
#include "app_main.h"
#include "sys_filesystem.h"
#include "ui_gui_settings.h"
#include "app_logparser.h"
#include <QFileDialog>

//
// cvars
//
extern pCvar *k_minimumTeamMembers;
extern pCvar *k_maximumTeamMembers;
extern pCvar *k_eventStartTime;
extern pCvar *k_eventFinishTime;
extern pCvar *k_eventFinalTime;
extern pCvar *k_singleComboPoints;
extern pCvar *k_doubleComboPoints;
extern pCvar *k_tripleComboPoints;
extern pCvar *k_defaultTaskList;
extern pCvar *k_penaltyMultiplier;
extern pCvar *k_autoSave;
extern pCvar *k_drunkMode;
extern pCvar *fs_homePath;
extern pCvar *fs_basePath;

/*
================
construct
================
*/
Gui_Settings::Gui_Settings( QWidget *parent ) : QDialog(parent), ui( new Ui::Gui_Settings ) {
    ui->setupUi( this );
    this->intializeCvars();
    this->updateDrunkMode( k_drunkMode->name(), k_drunkMode->string());
}

/*
================
destruct
================
*/
Gui_Settings::~Gui_Settings() {
    this->disconnect( k_drunkMode, SIGNAL( valueChanged( QString, QString )), this, SLOT( updateDrunkMode( QString, QString )));
    delete ui;
}

/*
================
initializeCvars
================
*/
void Gui_Settings::intializeCvars() {
    // lock cvars
    this->lockCvars();

    // set default values
    this->addCvar( k_eventStartTime, pSettingsCvar::TimeEdit, this->ui->startTime );
    this->addCvar( k_eventFinishTime, pSettingsCvar::TimeEdit, this->ui->finishTime );
    this->addCvar( k_eventFinalTime, pSettingsCvar::TimeEdit, this->ui->finalTime );
    this->addCvar( k_penaltyMultiplier, pSettingsCvar::SpinBox, this->ui->penalty );
    this->addCvar( k_singleComboPoints, pSettingsCvar::SpinBox, this->ui->sCombo );
    this->addCvar( k_doubleComboPoints, pSettingsCvar::SpinBox, this->ui->dCombo );
    this->addCvar( k_tripleComboPoints, pSettingsCvar::SpinBox, this->ui->tCombo );
    this->addCvar( k_minimumTeamMembers, pSettingsCvar::SpinBox, this->ui->min );
    this->addCvar( k_maximumTeamMembers, pSettingsCvar::SpinBox, this->ui->max );
    this->addCvar( k_autoSave, pSettingsCvar::CheckBox, this->ui->autoSave );
    this->addCvar( k_drunkMode, pSettingsCvar::CheckBox, this->ui->drunkMode );

    // set state
    foreach ( pSettingsCvar *scPtr, this->cvarList )
        scPtr->setState();

    // unlock cvars
    this->lockCvars( false );

    // connect for drunk updates
    this->connect( k_drunkMode, SIGNAL( valueChanged( QString, QString )), this, SLOT( updateDrunkMode( QString, QString )));
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
        this->ui->autoSave->setDisabled( true );
        this->ui->buttonImport->setDisabled( true );
        this->ui->buttonExport->setDisabled( true );
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
        this->ui->autoSave->setEnabled( true );
        this->ui->buttonImport->setEnabled( true );
        this->ui->buttonExport->setEnabled( true );
    }
}

/*
================
getFileName
================
*/
QString Gui_Settings::getFilename( FileDialog type ) {
    QString filename, path;
    bool ok;

    // get path
    path = fs.buildPath( "logs/", fs_homePath->string() + fs_basePath->string(), &ok );
    if ( !ok )
        return filename;

    switch ( type ) {
    case Open:
#ifdef Q_OS_WIN
    filename = QFileDialog::getOpenFileName( this, this->tr( "Load log database" ), path, this->tr( "Team log database (*.db)" ));
#else
    // for some reason Qt 4.8 cannot open in default dir (at least on linux)
    filename = QFileDialog::getOpenFileName( this, this->tr( "Load log database" ), path, this->tr( "Team log database (*.db)" ), 0, QFileDialog::DontUseNativeDialog );
#endif
        break;

    case Save:
#ifdef Q_OS_WIN
    filename = QFileDialog::getSaveFileName( this, this->tr( "Save log database" ), path + "export.db", this->tr( "Team log database (*.db)" ));
#else
    // for some reason Qt 4.8 cannot open in default dir (at least on linux)
    filename = QFileDialog::getSaveFileName( this, this->tr( "Save log database" ), path + "export.db", this->tr( "Team log database (*.db)" ), 0, QFileDialog::DontUseNativeDialog );
#endif
        break;

    default:
        return filename;
    }

    // make sure we add extension, but not to empty files
    if ( !filename.isEmpty())
        filename = fs.defaultExtension( filename, ".db" );

    return filename;
}

/*
================
buttonExport->clicked
================
*/
void Gui_Settings::on_buttonExport_clicked() {
    QString buffer, filename;
    fileHandle_t out;

    // get filename
    filename = this->getFilename( Save );
    if ( filename.isEmpty())
        return;

    // generate macro list
    buffer.append( "<teams>\n" );
    foreach ( App_TeamEntry *teamPtr, m.teamList ) {
        // ignore team with no logs
        if ( teamPtr->logList.count())
            buffer.append( teamPtr->generateXMLLog());
    }
    buffer.append( "</teams>\n" );

    // open file in read mode
    fs.open( pFile::Write, filename, out, Sys_Filesystem::Absolute );
    fs.print( out, buffer );
    fs.close( out );
}

/*
================
buttonImport->clicked
================
*/
void Gui_Settings::on_buttonImport_clicked() {
    QString buffer, filename;
    App_LogParser *handlerPtr;

    // get filename
    filename = this->getFilename( Open );
    if ( filename.isEmpty())
        return;

    // set up xml handler
    handlerPtr = new App_LogParser();

    // read file
    buffer = fs.readTextFile( filename, "UTF-8", Sys_Filesystem::Absolute );
    handlerPtr->setFilename( filename );
    m.parseXML( filename, buffer, handlerPtr );
    m.sort( App_Main::Teams );

    // get rid of handler
    delete handlerPtr;
    buffer.clear();
}
