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

#pragma once

//
// includes
//
#include <QPainter>

//
// classes
//
class Delegate;

/**
 * @brief The Item class
 */
class Item {
public:
    /**
     * @brief The Types enum
     */
    enum Types {
        Checkable,
        Numeric,
        Multi,
        Sum,
        Editor
    };

    /**
     * @brief The Action enum
     */
    enum Actions {
        NoAction = -1,
        Set,
        Edit,
        Remove,
        Combine,
        SetNumeric
    };

    /**
     * @brief Item
     * @param t
     * @param r
     * @param d
     */
    Item( Types t, const QRect &r, const Delegate *d ) :  m_type( t ), rect( r ), delegate( d ) {}
    void paint( QPainter *painter, const QModelIndex &index ) const;
    Actions action( const QModelIndex &index ) const;
    Types type() const { return this->m_type; }

private:
    Types m_type;
    QRect rect;
    const Delegate *delegate;

    /**
     * @brief lighter
     * @param colour
     * @return
     */
    static const QColor lighter( const QColor &colour ) { return QColor( colour.red(), colour.green(), colour.blue(), colour.alpha() / 2 ); }

    // colours
    static const QColor Green;
    static const QColor LtGreen;
    static const QColor Blue;
    static const QColor LtBlue;
    static const QColor Gray;
    static const QColor LtGray;
    static const QColor Black;
    static const QColor LtBlack;
    static const QColor Red;
    static const QColor LtRed;
};

// declare enums
Q_DECLARE_METATYPE( Item::Actions )
Q_DECLARE_METATYPE( Item::Types )
