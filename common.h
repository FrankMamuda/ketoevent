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

#ifndef COMMON_H
#define COMMON_H

//
// includes
//
#include <QObject>

// message macro
#ifdef Q_CC_MSVC
#define ClassFunc QString( "%1::%2: " ).arg( this->metaObject()->className()).arg( __FUNCTION__ )
#define ClassLessFunc __FUNCTION__
#else
#define ClassFunc QString( "%1::%2: " ).arg( this->metaObject()->className()).arg( __func__ )
#define ClassLessFunc __func__
#endif


// error message macros
#define StrMsg ClassFunc
#define StrFatalError Common::FatalError, ClassFunc
#define StrSoftError Common::SoftError, ClassFunc
#define StrWarn QObject::trUtf8( "WARNING:" ) + ClassFunc
#define CLMsg ClassLessFunc
#define CLFatalError Common::FatalError, ClassLessFunc
#define CLSoftError Common::SoftError, ClassLessFunc

/**
 * @brief The Common class
 */
class Common : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Common functions" )
    Q_ENUMS( ErrorTypes )
    Q_FLAGS( DebugLevels )
    //Q_PROPERTY( DebugLevels debugLevel READ debugLevel WRITE setDebugLevel )

public:
    // error types
    enum ErrorTypes {
        SoftError = 0,
        FatalError
    };

    // debug levels
    enum DebugLevel {
        NoDebug =    0x0,
        System =     0x0001,
        GuiMain =    0x0002,
        DatabaseDebug =   0x0004,
        LogDebug =   0x0008,
        TaskDebug =  0x0010,
        TeamDebug =  0x0020,
        EventDebug = 0x0040,
        Debug =      0x0080
    };

    Q_DECLARE_FLAGS( DebugLevels, DebugLevel )

    // console io
    static void error( ErrorTypes type = SoftError, const QString &func = QString::null, const QString &msg = QString::null );
    static void print( const QString &msg, DebugLevel = NoDebug );
};

#endif // COMMON_H
