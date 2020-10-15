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
#include "variable.h"

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
        Editor,
        Info
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
        SetNumeric,
        AddCombo,
        RemoveCombo,
        InfoPopup
    };

    /**
     * @brief Item
     * @param t
     * @param r
     * @param d
     */
    explicit Item( Types t, const QRect &r, const Delegate *d ) :  m_type( t ), rect( r ), delegate( d ) {}
    void paint( QPainter *painter, const QModelIndex &index ) const;
    [[nodiscard]] Actions action( const QModelIndex &index ) const;
    [[nodiscard]] Types type() const { return this->m_type; }

private:
    Types m_type;
    QRect rect;
    const Delegate *delegate;

    /**
     * @brief lighter
     * @param colour
     * @return
     */
    [[nodiscard]] static const QColor lighter( const QColor &colour ) { return QColor( colour.red(), colour.green(), colour.blue(), colour.alpha() / 2 ); }

    // colours
    static const QColor Green() { return Variable::isEnabled( "darkMode" ) ? QColor( 77, 194, 77, 216 ) : QColor( 57, 174, 57, 196 ); }
    static const QColor LtGreen() { return lighter( Item::Green()); }
    static const QColor Blue() { return Variable::isEnabled( "darkMode" ) ? QColor( 20, 194, 255, 196 ) : QColor( 0, 174, 255, 196 ); }
    static const QColor LtBlue() { return lighter( Item::Blue()); }
    static const QColor Gray() { return Variable::isEnabled( "darkMode" ) ? QColor( 164, 164, 164, 196 ) : QColor( 64, 64, 64, 196 ); }
    static const QColor LtGray() { return lighter( Item::Gray()); }
    static const QColor Black() { return Variable::isEnabled( "darkMode" ) ? QColor( 255, 255, 255, 196 ) : QColor( 0, 0, 0, 196 ); }
    static const QColor LtBlack() { return Variable::isEnabled( "darkMode" ) ? QColor( 255, 255, 255, 128 ) : QColor( 0, 0, 0, 128 ); }
    static const QColor Red() { return Variable::isEnabled( "darkMode" ) ? QColor( 255, 80, 80, 216 ) : QColor( 190, 0, 0, 196 ); }
    static const QColor LtRed() { return lighter( Item::Red()); }
    static const QColor Yellow() { return Variable::isEnabled( "darkMode" ) ? QColor( 210, 210, 20, 196 ) : QColor( 190, 190, 0, 196 ); }
    static const QColor LtYellow() { return lighter( Item::Yellow()); }
};

// declare enums
Q_DECLARE_METATYPE( Item::Actions )
Q_DECLARE_METATYPE( Item::Types )
