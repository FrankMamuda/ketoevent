/*
 * Copyright (C) 2018 Factory #12
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
#include "rankings.h"
#include "ui_rankings.h"
#include "mainwindow.h"
#include "team.h"
#include "log.h"
#include <QtConcurrent>

/**
 * @brief Rankings::Rankings
 * @param parent
 */
Rankings::Rankings( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::Rankings ) {
    this->ui->setupUi( this );
    this->setWindowModality( Qt::ApplicationModal );

    this->progress.setLabelText( this->tr( "Recalculating stats using %1 thread(s)..." ).arg( QThread::idealThreadCount()));

    // TODO: disconnect
    this->connect( &this->futureWatcher, SIGNAL( finished()), &this->progress, SLOT( reset()));
    this->connect( &this->progress, SIGNAL( canceled()), &this->futureWatcher, SLOT(cancel()));
    this->connect( &this->futureWatcher, SIGNAL( progressRangeChanged( int, int )), &this->progress, SLOT( setRange( int, int )));
    this->connect( &this->futureWatcher, SIGNAL ( progressValueChanged( int )), &this->progress, SLOT( setValue( int )));
}

/**
 * @brief Rankings::~Rankings
 */
Rankings::~Rankings() {
    delete this->ui;
}


/**
 * @brief spin
 * @param iteration
 */
void spin( int &row ) {
    int y;
    //volatile int points;
    volatile int completed = 0;

    for ( y = 0; y < Log::instance()->count(); y++ ) {
        if ( Log::instance()->teamId( y ) != row )
            continue;

        if ( Log::instance()->multiplier( y ) > 0 )
            completed++;
    }

    qDebug() << "team" << Team::instance()->title( row ) << "has completed" << completed << "tasks";
}

/**
 * @brief Rankings::on_actionUpdate_triggered
 */
void Rankings::on_actionUpdate_triggered() {
    QVector<int> vector;

    // NOTE: it would be faster to just process logs not teams
    for ( int y = 0; y < Team::instance()->count(); y++ )
        vector.append( y );

    futureWatcher.setFuture( QtConcurrent::map( vector, spin ));
    this->progress.exec();
    futureWatcher.waitForFinished();

    //qDebug() << "Canceled?" << futureWatcher.future().isCanceled();
}
