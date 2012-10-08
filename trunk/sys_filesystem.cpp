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
// NOTE: using a modified YPlatorm2 (r25) code
//

//
// includes
//
#include "sys_common.h"
#include "sys_filesystem.h"
#include "sys_cvar.h"

//
// classes
//
class Sys_Filesystem fs;

//
// cvars
//
pCvar *fs_homePath;
pCvar *fs_appPath;
pCvar *fs_basePath;

/*
============
init
============
*/
void Sys_Filesystem::init() {
    // get homePath
    QString homePath = QDir::homePath();
    homePath.append( Filesystem::PathSeparator );
    homePath.append( Filesystem::PlatformDirectory );

    // create cvars
    fs_homePath = cv.create( "fs_homePath", homePath.replace( '\\', Filesystem::PathSeparator ).append( Filesystem::PathSeparator ), ( pCvar::ReadOnly ));
    fs_appPath = cv.create( "fs_appPath", QDir::currentPath().replace( '\\', Filesystem::PathSeparator ).append( Filesystem::PathSeparator ), ( pCvar::ReadOnly ));
    fs_basePath = cv.create( "fs_basePath", Filesystem::BaseDirectory + Filesystem::PathSeparator, ( pCvar::Flags )( pCvar::Archive | pCvar::ReadOnly ));

    // add search paths
    this->addSearchPath( ":/", Filesystem::InternalPathID );
    this->addSearchPath( fs_homePath->string().append( fs_basePath->string()), Filesystem::HomePathID );
    this->addSearchPath( fs_appPath->string().append( fs_basePath->string()), Filesystem::AppPathID );

    // reset handles
    // -1 and 0 is reserved
    this->numFileHandles = 1;

    // all done
    this->setInitialized();

    // add resources
    QDir::setSearchPaths( ":", QStringList( ":/" ));
}

// latin4 chars
static unsigned int latin4Array[] = {
    224, // aa
    232, // ch
    186, // ee
    187, // gj
    239, // ii
    243, // kj
    182, // lj
    241, // nj
    242, // oo
    179, // rj
    185, // sh
    254, // uu
    190, // zh
    192, // AA
    200, // CH
    170, // EE
    171, // GJ
    207, // II
    211, // KJ
    166, // LJ
    209, // NJ
    210, // OO
    163, // RJ
    169, // SH
    222, // UU
    174  // ZH
};
static int latin4ArraySize = sizeof( latin4Array ) / sizeof( int );


// latin4 chars
static unsigned int latin4ArrayB[] = {
    257, // aa
    269, // ch
    275, // ee
    291, // gj
    299, // ii
    311, // kj
    316, // lj
    326, // nj
    333, // oo
    343, // rj
    353, // sh
    363, // uu
    382, // zh
    256, // AA
    268, // CH
    274, // EE
    290, // GJ
    298, // II
    310, // KJ
    315, // LJ
    325, // NJ
    332, // OO
    342, // RJ
    352, // SH
    362, // UU
    381  // ZH
};

// latin1 corresponding chars
static char latin1Array[] = {
    'a', // aa
    'c', // ch
    'e', // ee
    'g', // gj
    'i', // ii
    'k', // kj
    'l', // lj
    'n', // nj
    'o', // oo
    'r', // rj
    's', // sh
    'u', // uu
    'z', // zh
    'A', // AA
    'C', // CH
    'E', // EE
    'G', // GJ
    'I', // II
    'K', // KJ
    'L', // LJ
    'N', // NJ
    'O', // OO
    'R', // RJ
    'S', // SH
    'U', // UU
    'Z'  // ZH
};

/*
============
transliteratePath

  NOTE: currently transliterates only latvian latin-4 strings
============
*/
QString Sys_Filesystem::transliteratePath( const QString &path ) {
    int y;
    QString out;

    foreach ( QChar ch, path ) {
        for ( y = 0; y < latin4ArraySize; y++ ) {
            if ( ch == QChar( latin4Array[y] ) || ch == QChar( latin4ArrayB[y] ))
                ch = latin1Array[y];
        }
        out.append( ch );
    }
    return out;
}

/*
============
safeTeamFilename
============
*/
QString Sys_Filesystem::safeTeamFilename( const QString &teamName ) {
    QString filename = teamName;
    filename.replace( QRegExp( "[\\W]" ), "_" );
    filename = this->transliteratePath( filename );
    filename.toLower();

    // add preffix/suffix
    if ( !filename.startsWith( "logs/" ))
        filename.prepend( "logs/" );

    return this->defaultExtension( filename, ".xml" );
}

/*
============
shutdown
============
*/
void Sys_Filesystem::shutdown() {
    // failsafe
    if ( !this->hasInitialized())
        return;

    foreach ( pSearchPath *sp, this->searchPaths )
        delete sp;
    this->searchPaths.clear();

    foreach ( pFile *filePtr, this->fileList ) {
        if ( filePtr->mode() >= pFile::Read )
            filePtr->fHandle.close();
        delete filePtr;
    }
    this->fileList.clear();
}

/*
============
addSearchPath
============
*/
void Sys_Filesystem::addSearchPath( const QString &path, const QString &id ) {
    pSearchPath *searchPath = new pSearchPath();
    searchPath->setPath( path );
    searchPath->setId( id );
    this->searchPaths << searchPath;
}

/*
============
getSearchPathIndex
============
*/
int Sys_Filesystem::getSearchPathIndex( const QString &id ) const {
    int y = 0;

    foreach ( pSearchPath *sp, this->searchPaths ) {
        if ( !QString::compare( id, sp->id()))
            return y;

        y++;
    }
    return -1;
}

/*
============
buildPath
============
*/
QString Sys_Filesystem::buildPath( const QString &filename, const QString &basePath, bool *ok ) {
    QString path;
    *ok = false;

    if ( basePath.isNull() || basePath.isEmpty() || filename.isNull() || filename.isEmpty()) {
        com.error( StrSoftError + this->tr( "invalid path\n" ));
        return "";
    }

    path = basePath + filename;
    path.replace( '\\', Filesystem::PathSeparator );
    *ok = true;

    return path;
}

/*
============
existsExt
============
*/
bool Sys_Filesystem::existsExt( QString &filename, OpenFlags &flags, int &searchPathIndex ) {
    bool ok;
    int y = -1;

    if ( filename.startsWith( ":/" )) {
        // abort with skipInternal flag and :/ paths
        if ( flags.testFlag( Sys_Filesystem::SkipInternal ))
            return false;

        if ( flags.testFlag( Absolute )) {
            com.error( StrFatalError + this->tr( "(Absolute) flag set for internal path\n" ));
            return false;
        }

        if ( QFile::exists( filename )) {
            searchPathIndex = this->getSearchPathIndex( Filesystem::InternalPathID );
            return true;
        } else {
            return false;
        }
    }

    if ( flags.testFlag( Absolute )) {
        QFile tmpFile( filename );
        if ( tmpFile.open( QIODevice::ReadOnly )) {
            tmpFile.close();
            return true;
        }
    } else {
        foreach ( pSearchPath *sp, this->searchPaths ) {
            y++;

            // skip internal paths
            if ( sp->id() == Filesystem::InternalPathID && flags.testFlag( Sys_Filesystem::SkipInternal ))
                continue;

            QFile tmpFile( this->buildPath( filename, sp->path(), &ok ));
            if ( !ok ) {
                y++;
                continue;
            } else {
                if ( !tmpFile.open( QIODevice::ReadOnly ))
                    continue;

                searchPathIndex = y;
                tmpFile.close();
                return true;
            }
        }
    }
    return false;
}

/*
============
openInReadMode
============
*/
long Sys_Filesystem::openInReadMode( const QString &filename, fileHandle_t &fHandle, int searchPathIndex, OpenFlags flags ) {
    pFile *filePtr;

    // assign next available handle
    fHandle = this->numFileHandles++;

    // allocate and mark as currently being read
    filePtr = new pFile();
    this->fileList << filePtr;
    filePtr->setHandle( fHandle );
    filePtr->setOpenMode( pFile::Read );
    filePtr->setPathIndex( searchPathIndex );

    // absolute path
    if ( flags.testFlag( Absolute )) {
        filePtr->fHandle.setFileName( filename );
        if ( !filePtr->fHandle.open( QIODevice::ReadOnly ))
            return -1;

        filePtr->setName( filename );
        return filePtr->fHandle.size();
    }

    // open the file
    if ( !filename.startsWith( ":/" ))
        filePtr->fHandle.setFileName( QString( filename ).prepend( this->searchPaths.at( searchPathIndex )->path()));
    else
        filePtr->fHandle.setFileName( filename );

    if ( !filePtr->fHandle.open( QIODevice::ReadOnly ))
        return -1;

    filePtr->setName( filename );
    return filePtr->fHandle.size();
}

/*
============
openInWriteMode
============
*/
void Sys_Filesystem::openInWriteMode( const QString &filename, fileHandle_t &fHandle, OpenFlags flags ) {
    pFile *filePtr;

    // failsafe
    if ( filename.startsWith( ":/" )) {
        com.error( StrFatalError + this->tr( "cannot write to internal filesystem\n" ));
        return;
    }

    // we store all new files at home
    int searchPathIndex = this->getSearchPathIndex( Filesystem::HomePathID );

    // failed
    if ( searchPathIndex < 0 ) {
        com.error( StrFatalError + this->tr( "openInWriteMode: could not resolve homePath\n" ));
        return;
    }

    // assign next available handle
    fHandle = this->numFileHandles++;

    // allocate and mark as currently being read in write mode
    filePtr = new pFile();
    this->fileList << filePtr;
    filePtr->setOpenMode( pFile::Write );
    filePtr->setPathIndex( searchPathIndex );

    QDir tmpDir;
    if ( flags.testFlag( Absolute )) {
        filePtr->fHandle.setFileName( filename );
        tmpDir = QFileInfo( filePtr->fHandle ).absoluteDir();
    } else {
        filePtr->fHandle.setFileName( QString( filename ).prepend( this->searchPaths.at( searchPathIndex )->path()));
        tmpDir.setPath( this->searchPaths.at( searchPathIndex )->path() + QFileInfo( filename ).path());
    }

    if ( !tmpDir.exists()) {
        tmpDir.mkpath( tmpDir.absolutePath());
        if ( !tmpDir.exists()) {
            com.error( StrFatalError + this->tr( "could not create path \"%1\"\n" ).arg( this->searchPaths.at( searchPathIndex )->path()));
            return;
        }
    }

    if ( !filePtr->fHandle.open( QIODevice::WriteOnly )) {
        if ( !( flags.testFlag( Silent )))
            com.error( StrSoftError + this->tr( "could not open \"%1\" in write mode\n" ).arg( filename ));
        return;
    }

    filePtr->setHandle( fHandle );
    filePtr->setName( filename );
}

/*
============
open
============
*/
long Sys_Filesystem::open( pFile::OpenModes mode, const QString &filename, fileHandle_t &fHandle, OpenFlags flags ) {
    int fsp;
    QString path;

    // minus one means fail
    long fileLength = -1;

    // copy path
    path = filename;

    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return fileLength;
    }

    // abort with skipInternal flag and :/ paths
    if ( filename.startsWith( ":/" ) && flags.testFlag( Sys_Filesystem::SkipInternal ))
        return fileLength;

    // we cannot have these symbols in paths
    if ( path.contains( ".." ) || path.contains( "::" )) {
        com.error( StrSoftError + this->tr( "invalid path \"%1\"\n" ).arg( path ));
        return fileLength;
    }

    // check if it exists at all
    if ( !this->exists( path, flags, fsp ) && mode != pFile::Write ) {
        if ( !( flags.testFlag( Silent )))
            com.error( StrSoftError + this->tr( "could not open file \"%1\"\n" ).arg( path ));
        return fileLength;
    } else {
        // already open?
        if ( !( flags.testFlag( Force ))) {
            foreach ( pFile *filePtr, this->fileList ) {
                if ( !QString::compare( path, filePtr->name()/*, Qt::CaseInsensitive */) && ( fsp == filePtr->pathIndex()) && filePtr->mode() >= pFile::Read ) {
                    switch( filePtr->mode()) {
                    case pFile::Closed:
                        break;

                    case pFile::Read:
                        if ( !( flags.testFlag( Silent )))
                            com.error( StrSoftError + this->tr( "file \"%1\" already open in read mode\n" ).arg( path ));
                        break;

                    case pFile::Write:
                        if ( !( flags.testFlag( Silent )))
                            com.error( StrSoftError + this->tr( "file \"%1\" already open in write mode\n" ).arg( path ));
                        break;
                    }
                    return fileLength;
                }
            }
        }

        // open according to mode
        switch( mode ) {
        case pFile::Read:
            fileLength = this->openInReadMode( path, fHandle, fsp, flags );
            break;

        case pFile::Write:
            this->openInWriteMode( path, fHandle, flags );
            fileLength = 0;
            break;

        default:
            com.error( StrFatalError + this->tr( "invalid mode %1\n" ).arg( mode ));
            return -1;
        }
    }
    return fileLength;
}

/*
============
close
============
*/
void Sys_Filesystem::close( const QString &filename, OpenFlags flags ) {
    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return;
    }

    foreach ( pFile *filePtr, this->fileList ) {
        if ( !QString::compare( filename, filePtr->name())) {
            if ( filePtr->mode() == pFile::Closed ) {
                if ( !( flags.testFlag( Silent )))
                    com.print( StrWarn + this->tr( "file \"%1\" already closed\n" ).arg( filename ));
                return;
            }
            filePtr->setOpenMode( pFile::Closed );
            filePtr->fHandle.close();

            return;
        }
    }
}

/*
============
close
============
*/
void Sys_Filesystem::close( const fileHandle_t fHandle, OpenFlags flags ) {
    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return;
    }

    if ( !fHandle )
        return;

    foreach ( pFile *filePtr, this->fileList ) {
        if ( fHandle == filePtr->handle()) {
            if ( filePtr->mode() == pFile::Closed ) {
                if ( !( flags.testFlag( Silent )))
                    com.print( StrWarn + this->tr( "file \"%1\" already closed\n" ).arg( filePtr->name()));
                return;
            }
            filePtr->setOpenMode( pFile::Closed );
            filePtr->fHandle.close();

            return;
        }
    }
    com.print( StrWarn + this->tr( "file with handle %1 has not been opened\n" ).arg( fHandle ));
}

/*
============
read
============
*/
long Sys_Filesystem::read( byte *buffer, unsigned long len, fileHandle_t fHandle, OpenFlags flags ) {
    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return -1;
    }

    if ( !fHandle ) {
        com.error( StrSoftError + this->tr( "called without fileHandle\n" ));
        return -1;
    }

    foreach ( pFile *filePtr, this->fileList ) {
        if ( fHandle == filePtr->handle()) {
            if ( filePtr->mode() != pFile::Read ) {
                if ( !( flags.testFlag( Silent )))
                    com.error( StrSoftError + this->tr( "file \"%1\" opened in wrong mode, aborting\n" ).arg( filePtr->name()));
                return -1;
            }
            return filePtr->fHandle.read( reinterpret_cast<char*>( buffer ), len );
        }
    }
    if ( !( flags.testFlag( Silent )))
        com.error( StrSoftError + this->tr( "file with handle %1 has not been opened\n" ).arg( fHandle ));
    return -1;
}

/*
============
write
============
*/
long Sys_Filesystem::write( const byte *buffer, unsigned long len, fileHandle_t fHandle, OpenFlags flags ) {
    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return -1;
    }

    if ( !fHandle ) {
        com.error( StrSoftError + this->tr( "called without fileHandle\n" ));
        return -1;
    }

    foreach ( pFile *filePtr, this->fileList ) {
        if ( fHandle == filePtr->handle()) {
            if ( filePtr->mode() != pFile::Write ) {
                if ( !( flags.testFlag( Silent )))
                    com.error( StrSoftError + this->tr( "file \"%1\" opened in wrong mode\n" ).arg( filePtr->name()));
                return -1;
            }
            return filePtr->fHandle.write( QByteArray( reinterpret_cast<const char*>( buffer ), len ));
        }
    }
    if ( !( flags.testFlag( Silent )))
        com.error( StrSoftError + this->tr( "file with handle %1 has not been opened\n" ).arg( fHandle ));
    return -1;
}

/*
============
write
============
*/
long Sys_Filesystem::write( const QByteArray buffer, fileHandle_t fHandle, OpenFlags flags ) {
    // dd, this is bad, we lose important info this way (like encoding)
    //return this->write( reinterpret_cast<const byte*>( buffer.constData()), buffer.length(), fHandle, flags );

    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return -1;
    }

    if ( !fHandle ) {
        com.error( StrSoftError + this->tr( "called without fileHandle\n" ));
        return -1;
    }

    foreach ( pFile *filePtr, this->fileList ) {
        if ( fHandle == filePtr->handle()) {
            if ( filePtr->mode() != pFile::Write ) {
                if ( !( flags.testFlag( Silent )))
                    com.error( StrSoftError + this->tr( "file \"%1\" opened in wrong mode\n" ).arg( filePtr->name()));
                return -1;
            }
            return filePtr->fHandle.write( buffer );
        }
    }
    if ( !( flags.testFlag( Silent )))
        com.error( StrSoftError + this->tr( "file with handle %1 has not been opened\n" ).arg( fHandle ));
    return -1;
}

/*
=================
print
=================
*/
void Sys_Filesystem::print( const fileHandle_t fHandle, const QString &msg, OpenFlags flags ) {
   //QByteArray array;
   // ..array.append( );
    this->write( msg.toUtf8(), fHandle, flags );
}

/*
============
readTextFile
============
*/
QString Sys_Filesystem::readTextFile( const QString &filename, const char *codec, OpenFlags flags ) {
    QTextStream stream( this->readFile( filename, flags ));
    stream.setCodec( codec );
    return stream.readAll();
}

/*
============
readFile
============
*/
QByteArray Sys_Filesystem::readFile( const QString &filename, OpenFlags flags ) {
    long len;
    fileHandle_t fHandle;
    byte *buf = NULL;
    QByteArray buffer;

    // abort with skipInternal flag and :/ paths
    if ( filename.startsWith( ":/" ) && flags.testFlag( Sys_Filesystem::SkipInternal ))
        return QByteArray();

    // look for it in the filesystem
    len = this->open( pFile::Read, filename, fHandle, flags );

    if ( len <= 0 ) {
        if ( !( flags.testFlag( Silent )))
            com.error( StrSoftError + this->tr( "could not read file \"%1\"\n" ).arg( filename ));
        return QByteArray();
    }

    // allocate in memory, read and store as a byte array
    buf = new byte[len];
    this->read( buf, len, fHandle, flags );
    buffer = QByteArray( reinterpret_cast<const char*>( buf ), len );

    // clear temporary buffer, close file
    delete []buf;
    this->close( fHandle, flags );

    // return buffer
    return buffer;
}

/*
============
touch
============
*/
void Sys_Filesystem::touch( const QString &filename, OpenFlags flags ) {
    fileHandle_t handle;
    this->openInWriteMode( filename, handle, flags );
    this->close( handle, flags );
}

/*
============
listDirectory
============
*/
QStringList Sys_Filesystem::listDirectory( QString searchDir, const QString &path, ListModes mode ) {
    QStringList foundFiles;
    QDir dir;

    // this is how we get local files
    if ( searchDir != path )
        dir = QDir( path + searchDir );
    else
        dir = QDir( path );

    if ( mode == ListAll )
        dir.setFilter( QDir::NoDotAndDotDot | QDir::AllEntries );
    else if ( mode == ListFiles )
        dir.setFilter( QDir::NoDotAndDotDot | QDir::Files );
    else if ( mode == ListDirs )
        dir.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );
    else {
        com.error( StrSoftError + this->tr( "invalid list mode\n" ));
        return QStringList();
    }
    QFileInfoList entryList = dir.entryInfoList();
    foreach ( QFileInfo info, entryList ) {
        if ( searchDir.startsWith( "/" )) {
            if ( !info.isDir())
                foundFiles << searchDir.remove( 0, 1 ) + info.fileName();
            else
                foundFiles << searchDir.remove( 0, 1 ) + info.fileName().append( "/" );
        } else {
            if ( !info.isDir())
                foundFiles << searchDir + info.fileName();
            else
                foundFiles << searchDir + info.fileName().append( "/" );
        }
    }
    return foundFiles;
}

/*
============
list
============
*/
QStringList Sys_Filesystem::list( QString searchDir, const QRegExp &filter, ListModes mode ) {
    QStringList foundFiles;

    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return QStringList();
    }

    if ( searchDir.startsWith( "." )) {
        com.error( StrSoftError + this->tr( "Dot and DotDot paths not supported\n" ));
        return QStringList();
    }

    if ( !searchDir.endsWith( "/" ))
        searchDir.append( "/" );

    foreach ( pSearchPath *sp, this->searchPaths )
        foundFiles << this->listDirectory( searchDir, sp->path(), mode );

    // filter
    foundFiles = foundFiles.filter( filter );
    foundFiles.removeDuplicates();

    // return our files
    return foundFiles;
}

/*
============
defaultExtenstion
============
*/
QString Sys_Filesystem::defaultExtension( const QString &filename, const QString &extension ) const {
    if ( !filename.endsWith( extension ))
        return filename + extension;
    else
        return filename;
}

/*
================
length
================
*/
long Sys_Filesystem::length( fileHandle_t handle ) const {
    const pFile *filePtr = this->fileForHandle( handle );
    if ( filePtr != NULL )
        return filePtr->fHandle.size();
    else
        return -1;
}

/*
================
fileForHandle
================
*/
pFile *Sys_Filesystem::fileForHandle( fileHandle_t handle ) const {
    foreach ( pFile *filePtr, this->fileList ) {
        if ( filePtr->handle() == handle )
            return filePtr;
    }
    return NULL;
}

