/*
===========================================================================
Copyright (C) 2012 Edd 'Double Dee' Psycho

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

//
// includes
//
#include "app_taskentry.h"
#include "app_tasklog.h"
#include "sys_common.h"
#include "sys_cvarfunc.h"

//
// cvars
//
extern pCvar *k_singleComboPoints;
extern pCvar *k_doubleComboPoints;
extern pCvar *k_tripleComboPoints;

/*
================
construct
================
*/
App_TaskEntry::App_TaskEntry( const QString &name, Types type, int points, int multi, bool challenge ) {
    // initialize values
    this->setName( name );
    this->setType( type );

    if ( points < 0 )
        points = 0;

    if ( multi < 0 )
        multi = 0;

    this->setPoints( points );
    this->setMaxMulti( multi );

    if ( this->type() == Special )
        this->setChallenge( false );
    else
        this->setChallenge( challenge );
}

/*
================
setName
================
*/
void App_TaskEntry::setName( const QString &name ) {
    this->m_name = name;

    // generate hash
    QCryptographicHash hash( QCryptographicHash::Md5 );

    // generate unique value to avoid mismatch
    hash.addData( this->name().toLatin1().constData(), this->name().length());
    this->m_hash = QString( hash.result().toHex());
}

/*
================
calculate
================
*/
int App_TaskEntry::calculate( QObject *parentPtr ) {
    int value = 0;
    App_TaskLog *logPtr = NULL;

    if ( parentPtr == NULL )
        return 0;

    // get log parent
    logPtr = qobject_cast<App_TaskLog*>( parentPtr );

    if ( logPtr == NULL )
        return 0;

    // get initial points
    switch ( this->type()) {
    case Check:
        if ( logPtr->value())
            value += this->points();
        break;

    case Multi:
        if ( logPtr->value() > this->maxMulti())
            value += this->points() * this->maxMulti();
        else
            value += this->points() * logPtr->value();
        break;

    case Special:
        value += logPtr->value();
        break;

    default:
        // error
        return 0;
    }

    // get combo points
    //value += logPtr->combo();
    switch ( logPtr->combo()) {
    case App_TaskLog::Single:
        value += k_singleComboPoints->integer();
        break;

    case App_TaskLog::Double:
        value += k_doubleComboPoints->integer();
        break;

    case App_TaskLog::Triple:
        value += k_tripleComboPoints->integer();
        break;

    default:
    case App_TaskLog::NoCombo:
        break;
    }

    // we're done
    return value;
}

/*
================
generateXMLTag
================
*/
QString App_TaskEntry::generateXMLTag() {
    QString safeName;

    // make sure buffer is xml safe
    safeName = this->name();
    safeName.replace( "&", "&amp;" );
    safeName.replace( "'", "&apos;" );
    safeName.replace( "\"", "&quot;" );
    safeName.replace( "<", "&lt;"  );
    safeName.replace( ">", "&gt;" );

    // we don't really care about start time since it is the same for everyone
    if ( this->type() == Check ) {
        return QString( "  <task name=\"%1\" points=\"%2\" type=\"%3\" challenge=\"%4\"/>\n" )
                .arg( safeName )
                .arg( this->points())
                .arg( Check )
                .arg( this->isChallenge());
    } else if ( this->type() == Multi ) {
        return QString( "  <task name=\"%1\" points=\"%2\" max=\"%3\" type=\"%4\" challenge=\"%5\"/>\n" )
                .arg( safeName )
                .arg( this->points())
                .arg( this->maxMulti())
                .arg( Multi )
                .arg( this->isChallenge());
    } else if ( this->type() == Special ) {
        return QString( "  <task name=\"%1\" type=\"%2\"/>\n" )
                .arg( safeName )
                .arg( Special );
    }

    // unknown type
    com.error( StrSoftError + this->tr( "unknown task type \"%1\"\n" ).arg( this->type()));
    return "";
}
