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
#include <QToolBar>

/**
 * @brief The Ui namespace
 */
namespace Ui {
static const int ToolBarIconSize = 16;
};

/**
 * @brief The ToolBar class
 */
class ToolBar : public QToolBar {
    Q_OBJECT

public:
    /**
     * @brief ToolBar
     */
    explicit ToolBar( QWidget *parent = nullptr ) : QToolBar( parent ) {
        this->setFloatable( false );
        this->setMovable( false );
        this->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
        this->setIconSize( QSize( Ui::ToolBarIconSize, Ui::ToolBarIconSize ));
        this->setAllowedAreas( Qt::TopToolBarArea );
    }
    virtual ~ToolBar() = default;
};
