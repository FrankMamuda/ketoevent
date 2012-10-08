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

#ifndef SYS_FILESYSTEM_H
#define SYS_FILESYSTEM_H

//
// NOTE: using a modified YPlatorm2 (r25) code
//

//
// includes
//
#include "sys_shared.h"

//
// namespaces
//
namespace Filesystem {
    class Sys_Filesystem;
    static const QChar PathSeparator( '/' );
    static const QString BaseDirectory( "base" );
    static const QString HomePathID( "homePath" );
    static const QString AppPathID( "appPath" );
    static const QString InternalPathID( "internalPath" );
#ifdef Q_OS_WIN
    static const QString PlatformDirectory( "KetoEvent" );
#else
    static const QString PlatformDirectory( ".ketoevent" );
#endif
}

//
// class:pFile
//
class pFile : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Filesystem file info" )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( fileHandle_t handle READ handle WRITE setHandle )
    Q_PROPERTY( OpenModes mode READ mode WRITE setOpenMode )
    Q_PROPERTY( int index READ pathIndex WRITE setPathIndex )
    Q_ENUMS( OpenModes )

public:
    enum OpenModes {
        Closed = -1,
        Read,
        Write
    };
    fileHandle_t    handle() const { return this->m_handle; }
    QString         name() const { return this->m_name; }
    OpenModes       mode() const { return this->m_mode; }
    int             pathIndex() const { return this->m_index; }
    QFile           fHandle;

public slots:
    void setName( const QString &name ) { this->m_name = name; }
    void setHandle( fileHandle_t handle ) { this->m_handle = handle; }
    void setOpenMode( OpenModes mode ) { this->m_mode = mode; }
    void setPathIndex( int index ) { this->m_index = index; }

private:
    fileHandle_t    m_handle;
    QString         m_name;
    OpenModes       m_mode;
    int             m_index;
};

//
// class:pSearchPath
//
class pSearchPath : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Filesystem search path" )
    Q_PROPERTY( QString path READ path WRITE setPath )
    Q_PROPERTY( QString id READ id WRITE setId )

public:
    QString path() const { return this->m_path; }
    QString id() const { return this->m_id; }

public slots:
    void setPath( const QString &path ) { this->m_path = path; }
    void setId( const QString &id ) { this->m_id = id; }

private:
    QString m_path;
    QString m_id;
};

//
// class:Sys_Filesystem
//
class Sys_Filesystem : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Filesystem" )
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )
    Q_FLAGS( OpenFlags OpenFlag )
    Q_ENUMS( ListModes )
    
public:
    enum OpenFlag {
        NoFlags         = 0x00,
        Absolute        = 0x01,
        DirsOnly        = 0x02,
        Unused0         = 0x04,
        Force           = 0x08,
        Silent          = 0x10,
        Unused1         = 0x20,
        SkipInternal    = 0x40
    };
    Q_DECLARE_FLAGS ( OpenFlags, OpenFlag )
    enum ListModes {
        ListAll = 0,
        ListDirs,
        ListFiles
    };
    bool hasInitialized() const { return this->m_initialized; }
    QList <pSearchPath*> searchPaths;
    long length( fileHandle_t ) const;
    long open( pFile::OpenModes mode, const QString &filename, fileHandle_t &fHandle, OpenFlags flags = NoFlags );
    long read( byte *buffer, unsigned long len, const fileHandle_t fHandle, OpenFlags flags = NoFlags );
    long write( const byte *buffer, unsigned long len, const fileHandle_t fHandle, OpenFlags flags = NoFlags );
    long write( const QByteArray buffer, const fileHandle_t fHandle, OpenFlags flags = NoFlags );
    void close( const fileHandle_t fHandle, OpenFlags flags = NoFlags );
    void close( const QString &filename, OpenFlags flags = NoFlags );
    void print( const fileHandle_t fHandle, const QString &msg, OpenFlags flags = NoFlags );
    QByteArray readFile( const QString &filename, OpenFlags flags = NoFlags );
    QString readTextFile( const QString &filename, const char *codec = "UTF-8", OpenFlags flags = NoFlags );
    void touch( const QString &filename, OpenFlags flags = NoFlags );
    QString defaultExtension( const QString &filename, const QString &extension ) const;
    bool extract( const QString &filename );
    bool exists( QString &path, OpenFlags &flags, int &searchPathIndex );
    bool exists( const QString &path, OpenFlags &flags, int &searchPathIndex );
    bool exists( const QString &path, OpenFlags flags, int &searchPathIndex );
    bool exists( const QString &path, OpenFlags flags = NoFlags );
    QStringList list( QString directory, const QRegExp &filter = QRegExp(), ListModes mode = ListAll );
    pFile *fileForHandle( fileHandle_t handle ) const;
    QString transliteratePath( const QString &path );
    QString safeTeamFilename( const QString &teamName );
    QString buildPath( const QString &filename, const QString &basePath, bool *ok );

private:
    bool m_initialized;
    void addSearchPath( const QString &path, const QString &id = QString::null );
    void openInWriteMode( const QString &filename, fileHandle_t &fHandle, OpenFlags flags = NoFlags );
    long openInReadMode( const QString &filename, fileHandle_t &fHandle, int searchPathIndex, OpenFlags flags = NoFlags );
    int getSearchPathIndex( const QString &id ) const;
    int numFileHandles;
    QList <pFile*> fileList;
    bool existsExt( QString &path, OpenFlags &flags, int &searchPathIndex );
    QStringList listDirectory( QString searchDir, const QString &path, ListModes mode );

public slots:
    void setInitialized( bool intialized = true ) { this->m_initialized = intialized; }
    void init();
    void shutdown();
};

//
// externals
//
extern class Sys_Filesystem fs;

//
// inlines for fileExists
//
inline bool Sys_Filesystem::exists( QString &path, OpenFlags &flags, int &searchPathIndex ) { return this->existsExt( path, flags, searchPathIndex ); }
inline bool Sys_Filesystem::exists( const QString &path, OpenFlags &flags, int &searchPathIndex ) { QString filename = path; return this->existsExt( filename, flags, searchPathIndex ); }
inline bool Sys_Filesystem::exists( const QString &path, OpenFlags flags, int &searchPathIndex ) { QString filename = path; return this->existsExt( filename, flags, searchPathIndex ); }
inline bool Sys_Filesystem::exists( const QString &path, OpenFlags flags ) { QString filename = path; int index; return this->existsExt( filename, flags, index ); }

#endif // SYS_FILESYSTEM_H
