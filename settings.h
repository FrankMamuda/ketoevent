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
#include "modalwindow.h"

/**
 * @brief The Ui namespace
 */
namespace Ui {
class Settings;
}

/**
 * @brief The Settings class
 */
class Settings final : public ModalWindow {
    Q_DISABLE_COPY_MOVE( Settings )
    Q_OBJECT

public:
    static Settings *instance() { if ( Settings::i == nullptr ) Settings::i = new Settings(); return Settings::i; }
    ~Settings() override;

private:
    static Settings *i;
    explicit Settings();
    QStringList variables;
    Ui::Settings *ui;
};
