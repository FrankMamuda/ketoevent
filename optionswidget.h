/*
 * Copyright (C) 2019 Factory #12
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
#include <QLabel>
#include <QHBoxLayout>
#include <QVariant>

/**
 * @brief The OptionsWidget class
 */
class OptionsWidget : public QWidget {
    Q_OBJECT
    Q_ENUMS( Types )
    Q_PROPERTY( Types type READ type )

public:
    enum Types {
        NoType = 0,
        Integer,
        Time,
        Bool,
        String
    };

    explicit OptionsWidget( const Types &type = Types::NoType, const QString &label = QString(), const QVariant &value = QVariant(), QWidget *parent = nullptr );
    ~OptionsWidget();
    Types type() const { return this->m_type; }

private:
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    QLabel *label = new QLabel();
    QWidget *widget;
    Types m_type;
};
