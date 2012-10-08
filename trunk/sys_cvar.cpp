/*
===========================================================================
Copyright (C) 2009-2012 Edd 'Double Dee' Psycho

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
#include <QtXml/QDomNode>
#include "sys_common.h"
#include "sys_cvar.h"
#include "sys_filesystem.h"

//
// NOTE: using a modified YPlatorm2 (r25) code
//

//
// classes
//
Sys_Cvar cv;

/*
============
init
============
*/
void Sys_Cvar::init() {
    // we are up and running
    this->setInitialized();

    // create cvar name validator
    QRegExp rx( "[A-z0-9_\\-]+" );
    this->validator = new QRegExpValidator( rx, this );
}

/*
============
shutdown
============
*/
void Sys_Cvar::shutdown() {
    // failsafe
    if ( !this->hasInitialized())
        return;

    // remove validator
    delete this->validator;

    // cleanup
    this->clear();
}

/*
============
validate
============
*/
bool Sys_Cvar::validate( const QString &s ) const {
    int pos;

    // check for illegal chars
    if ( s.isNull())
        return false;

    // validate cvar name
    pos = 0;
    QString str( s );
    QRegExpValidator::State state = this->validator->validate( str, pos );

    // is valid string?
    if ( state == QRegExpValidator::Acceptable )
        return true;

    // fail
    return false;
}

/*
============
find
============
*/
pCvar *Sys_Cvar::find( const QString &name ) const {
    if ( !this->validate( name )) {
        com.error( StrSoftError + this->tr( "invalid cvar name \"%1\"\n" ).arg( name ));
        return NULL;
    }

    foreach ( pCvar *cvarPtr, this->cvarList ) {
        if ( !QString::compare( name, cvarPtr->name(), Qt::CaseInsensitive ))
            return cvarPtr;
    }
    return NULL;
}

/*
============
clear
============
*/
void Sys_Cvar::clear() {
    foreach ( pCvar *cvarPtr, this->cvarList )
        delete cvarPtr;
}

/*
============
create
============
*/
pCvar *Sys_Cvar::create( const QString &name, const QString &string, pCvar::Flags flags ) {
    if ( !this->validate( name )) {
        com.error( StrSoftError + this->tr( "invalid cvar name \"%1\"\n" ).arg( name ));
        return NULL;
    }

    // search for available cvars
    pCvar *cvarPtr = this->find( name );

    // create new
    if ( cvarPtr == NULL ) {
        // allocate & add to list
        cvarPtr = new pCvar( name, string, flags );
        this->cvarList << cvarPtr;
    }
    return cvarPtr;
}

/*
===============
parseConfig
===============
*/
void Sys_Cvar::parseConfig( const QString &filename ) {
    QDomDocument configFile;

    // read buffer
    QByteArray buffer = fs.readFile( filename, Sys_Filesystem::Silent );

    // failsafe
    if ( buffer.isNull()) {
        if ( !QString::compare( filename, Cvar::DefaultConfigFile )) {
            com.print( StrWarn + this->tr( "configuration file does not exist, creating \"%1\"\n" ).arg( filename ));
            fs.touch( filename, Sys_Filesystem::Silent );
        }
        return;
    }

    // failsafe
    if ( buffer.isEmpty())
        return;

    // parse document
    configFile.setContent( buffer );
    QDomNode configNode = configFile.firstChild();
    while ( !configNode.isNull()) {
        if ( configNode.isElement()) {
            QDomElement configElement = configNode.toElement();

            // check element name
            if ( QString::compare( configElement.tagName(), "config" )) {
                com.error( StrSoftError + this->tr( "expected <config> in \"%1\"\n" ).arg( filename ));
                return;
            }

            QDomNode cvarNode = configElement.firstChild();
            while ( !cvarNode.isNull()) {
                if ( cvarNode.isElement()) {
                    QDomElement cvarElement = cvarNode.toElement();

                    // check element name
                    if ( !QString::compare( cvarElement.tagName(), "cvar" )) {
                        // check cvar name
                        if ( !cvarElement.hasAttribute( "name" )) {
                            com.error( StrSoftError + this->tr( "parseConfig: nameless <cvar> in \"%1\"\n" ).arg( filename ));
                            return;
                        }

                        // now find the cvar
                        QString cvarName = cvarElement.attribute( "name" );
                        pCvar *cvarPtr = this->find( cvarName );

                        // if it doesn't exist - create it, if it already exists - force the stored value
                        if ( cvarPtr != NULL )
                            cvarPtr->set( cvarElement.text(), static_cast<pCvar::AccessFlags>( pCvar::Force ));
                        else
                            this->create( cvarName, cvarElement.text(), ( pCvar::Flags )cvarElement.attribute( "flags" ).toInt());
                    } else {
                        com.error( StrSoftError + this->tr( "expected <cvar> or <cmd> in \"%1\"\n" ).arg( filename ));
                        return;
                    }
                }
                cvarNode = cvarNode.nextSibling();
            }
            break;
        }
        configNode = configNode.nextSibling();
    }

    // clear buffer
    buffer.clear();
}

/*
===============
saveConfig
===============
*/
void Sys_Cvar::saveConfig( const QString &filename ) {
    // create document
    QDomDocument configFile;

    // create config tag
    QDomElement configElement = configFile.createElement( "config" );
    configElement.setAttribute( "version", "1.1" );
    configElement.setAttribute( "generator", "KetoEvent2" );
    configFile.appendChild( configElement );

    // generate cvars config strings
    foreach ( pCvar *cvarPtr, this->cvarList ) {
        if ( cvarPtr->flags.testFlag( pCvar::Archive )) {
            QDomElement cvarElement = configFile.createElement( "cvar" );
            cvarElement.setAttribute( "name", cvarPtr->name());
            cvarElement.setAttribute( "flags", cvarPtr->flags );
            configElement.appendChild( cvarElement );

            QDomText cvarText = configFile.createTextNode( cvarPtr->string());
            cvarElement.appendChild( cvarText );
        }
    }

    // write out
    fileHandle_t fileOut;
    if ( fs.open( pFile::Write, filename, fileOut, Sys_Filesystem::Silent ) != -1 ) {
        fs.print( fileOut, configFile.toString());
        fs.close( fileOut );
    }
}
