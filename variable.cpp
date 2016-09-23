/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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
#include "variable.h"
#include "main.h"

/**
 * @brief Variable::add
 * @param varPtr
 */
void Variable::add( const QString &key, QSettings *settingsPtr, const QVariant &defaultValue ) {
    // avoid duplicates
    if ( Variable::find( key ) != NULL )
        return;

    m.cvarList << new Variable( key, settingsPtr, defaultValue );
}

/**
 * @brief find
 * @param key
 * @return
 */
Variable *Variable::find( const QString &key ) {
    foreach ( Variable *varPtr, m.cvarList ) {
        if ( !QString::compare( varPtr->key(), key ))
            return varPtr;
    }
    return NULL;
}
