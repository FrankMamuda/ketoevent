/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020 Armands Aleksejevs
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

/*
 * includes
 */
#include "team.h"
#include "database.h"
#include "event.h"
#include "mainwindow.h"
#include <QSqlQuery>

// singleton
Team *Team::i = nullptr;

/**
 * @brief Team::Team
 */
Team::Team() : Table( "teams" ) {
    PRIMARY_FIELD( ID );
    FIELD( Title,    QMetaType::QString );
    FIELD( Members,  QMetaType::Int );
    FIELD( Finish,   QMetaType::QString );
    FIELD( Lock,     QMetaType::Int );
    FIELD( Reviewer, QMetaType::QString );
    FIELD( Event,    QMetaType::Int );
    this->addUniqueConstraint( QList<QSqlField>() << this->field( Title ) << this->field( Event ));
}

/**
 * @brief Team::add
 * @param title
 * @param members
 * @param finishTime
 * @param reviewer
 * @return
 */
Row Team::add( const QString &title, int members, const QTime &finishTime, const QString &reviewer ) {
    // failsafe
    const Row event = MainWindow::instance()->currentEvent();
    if ( event == Row::Invalid ) {
        qDebug() << this->tr( "no active event, aborting" );
        return Row::Invalid;
    }

    return Table::add( QVariantList() <<
                Database_::null <<
                title <<
                members <<
                finishTime.toString( Database_::TimeFormat ) <<
                0 <<
                reviewer <<
                       static_cast<int>( Event::instance()->id( event )));
}

/**
 * @brief Team::removeOrphanedEntries
 */
void Team::removeOrphanedEntries() {
    QSqlQuery query;

    // remove orphaned teams
    query.exec( QString( "delete from %1 where %2 not in (select %3 from %4)" )
                .arg( this->tableName(),
                      this->fieldName( Event ),
                      Event::instance()->fieldName( Event::ID ),
                      Event::instance()->tableName()));
    this->select();
}
