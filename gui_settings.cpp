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
#include "gui_settings.h"
#include "ui_gui_settings.h"
#include <QFileDialog>
#include <QTextStream>
#include "main.h"
#include "gui_main.h"

/*
================
construct
================
*/
Gui_Settings::Gui_Settings( QWidget *parent ) : Gui_SettingsDialog( parent ), ui( new Ui::Gui_Settings ) {
    ui->setupUi( this );

    if ( m.isInitialized())
        this->bindVars();
    else
        this->reject();

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
    this->unbindVars();
    delete ui;
}

/*
================
bindVars
================
*/
void Gui_Settings::bindVars() {
    // lock vars
    this->lockVariables();

    // bind vars
    this->bindVariable( "backup/changes", this->ui->backupChanges );
    this->bindVariable( "backup/perform", this->ui->backupPerform );
    this->bindVariable( "misc/sortTasks", this->ui->sort );
    this->bindVariable( "databasePath", this->ui->dbPath );

    // unlock vars
    this->lockVariables( false );
}

#if 0
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
#endif

/*
================
buttonExportCSV->clicked
================
*/
#if 0
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
        foreach ( TaskEntry *taskPtr, m.base.taskList ) {
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
#endif

#if 0
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
#endif

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

/*
================
pathButton->clicked
================
*/
void Gui_Settings::on_pathButton_clicked() {
    Gui_Main *gui;
    QString path, filePath;

    // get filename from dialog
    path = QString( QDir::currentPath() + "/" );
    filePath = QFileDialog::getSaveFileName( this, this->tr( "Select database" ), path, this->tr( "Database (*.db)" ), 0, QFileDialog::DontConfirmOverwrite );

    // check for empty filenames
    if ( filePath.isEmpty())
        return;

    // check if path is valid
    if ( !QFileInfo( filePath ).absoluteDir().isReadable())
        return;

    // append extension
    if ( !filePath.endsWith( ".db" ))
        filePath.append( ".db" );

    // don't reload the same database
    if ( !QString::compare( filePath, m.cvar( "databasePath" )->string()))
        return;

    // store database path
    this->ui->dbPath->setText( filePath );

    // clear data on reload
    m.clearEvent();

    // reload database
    m.reloadDatabase( m.cvar( "databasePath" )->string());

    // reinitialize gui
    gui = qobject_cast<Gui_Main*>( this->parent());
    if ( gui != NULL )
        gui->initialize( true );
}
