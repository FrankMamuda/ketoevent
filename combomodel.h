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

#pragma once

/*
 * includes
 */
#include "main.h"
#include "table.h"
#include <QRgb>
#include <QStringListModel>

/**
 * @brief The ComboModel class
 */
class ComboModel final : public QStringListModel {
    Q_DISABLE_COPY( ComboModel )
    Q_OBJECT
    friend class Combos;

public:
    // disable move
    ComboModel( ComboModel&& ) = delete;
    ComboModel& operator=( ComboModel&& ) = delete;

    static ComboModel *instance() { static ComboModel *instance( new ComboModel()); return instance; }
    ~ComboModel() override = default;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

    /**
     * @brief colourForId
     * @param id
     * @return
     */
    [[nodiscard]] static QRgb colourForId( int id ) {
        if ( id < 0 )
            return ComboModel::colourList.at( 0 );

        if ( id >= ComboModel::colourList.count())
            return ComboModel::colourForId( id - ComboModel::colourList.count() );

        return ComboModel::colourList.at( id );
    }

public slots:
    void reset( const Id &id );

private:
    explicit ComboModel() : combos( 0 ), points( 0 ) { GarbageMan::instance()->add( this ); }
    QMultiMap<Id, QString> map;
    QMap<QString, QRgb> colours;
    static QList<QRgb> colourList;
    int combos;
    int points;
};
