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
#include <QWidget>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class TaskEdit;
}

/**
 * @brief The TeamWidget class
 */
class TaskEdit final : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE( TaskEdit )

public:
    static TaskEdit& instance() { static TaskEdit instance; return instance; }
    ~TaskEdit() override;
    void reset( bool edit = false );
    [[nodiscard]] bool isEditing() const { return this->m_edit; }

private:
    explicit TaskEdit( QWidget *parent = nullptr );
    Ui::TaskEdit *ui;
    bool m_edit;
};
