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
#include "main.h"

/*
================
construct
================
*/
Gui_Settings::Gui_Settings( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_Settings ) {
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
    foreach ( SettingsVariable *varPtr, m.svarList )
        varPtr->unbind();

    delete ui;
}

/*
================
intializeVariables
================
*/
void Gui_Settings::intializeVariables() {
    // lock vars
    this->lockVariables();

    // bind vars
    m.svar( "startTime" )->bind( this->ui->startTime, qobject_cast<QObject*>( this ));
    m.svar( "finishTime" )->bind( this->ui->finishTime, qobject_cast<QObject*>( this ));
    m.svar( "finalTime" )->bind( this->ui->finalTime, qobject_cast<QObject*>( this ));
    m.svar( "penalty" )->bind( this->ui->penalty, qobject_cast<QObject*>( this ));
    m.svar( "singleCombo" )->bind( this->ui->sCombo, qobject_cast<QObject*>( this ));
    m.svar( "doubleCombo" )->bind( this->ui->dCombo, qobject_cast<QObject*>( this ));
    m.svar( "tripleCombo" )->bind( this->ui->tCombo, qobject_cast<QObject*>( this ));
    m.svar( "minMembers" )->bind( this->ui->min, qobject_cast<QObject*>( this ));
    m.svar( "maxMembers" )->bind( this->ui->max, qobject_cast<QObject*>( this ));
    m.svar( "backup/changes" )->bind( this->ui->backupChanges, qobject_cast<QObject*>( this ));
    m.svar( "backup/perform" )->bind( this->ui->backupPerform, qobject_cast<QObject*>( this ));
    m.svar( "misc/sortTasks" )->bind( this->ui->sort, qobject_cast<QObject*>( this ));
    m.svar( "name" )->bind( this->ui->titleEdit, qobject_cast<QObject*>( this ));

    // unlock vars
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
