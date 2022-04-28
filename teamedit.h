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
class TeamEdit;
}

/**
 * @brief The TeamEdit class
 */
class TeamEdit final : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE( TeamEdit )

public:
    static TeamEdit *instance() { if ( TeamEdit::i == nullptr ) TeamEdit::i = new TeamEdit(); return TeamEdit::i; }
    ~TeamEdit() override;
    void reset( bool edit = false );
    [[nodiscard]] bool isEditing() const { return this->m_edit; }

public slots:
    void setCurrentTime();

private:
    static TeamEdit *i;
    explicit TeamEdit( QWidget *parent = nullptr );
    Ui::TeamEdit *ui;
    bool m_edit;
};
