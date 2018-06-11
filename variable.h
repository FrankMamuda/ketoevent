/*
 * Copyright (C) 2013-2018 Factory #12
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
#include <QMap>
#include <QString>
#include <QMetaMethod>
#include <QSignalMapper>
#include <QLoggingCategory>
#include <QWidget>
#include "variableentry.h"

/**
 * @brief The Variable_ namespace
 */
namespace Variable_ {
    const static QLoggingCategory debug( "variable" );
}

/**
 * @brief The Variable class
 */
class Variable final : public QObject {
    Q_DISABLE_COPY( Variable )
    Q_OBJECT

public:
    ~Variable();

    /**
     * @brief instance
     * @return
     */
    static Variable *instance() { static Variable *instance( new Variable()); return instance; }
    bool contains( const QString &key ) const { return this->list.contains( key ); }

    // eventually move this to private
    QMap<QString, QSharedPointer<VariableEntry>> list;

    template<typename T>
    T value( const QString &key, bool defaultValue = false ) { if ( !this->contains( key )) return QVariant().value<T>(); if ( defaultValue ) return qvariant_cast<T>( this->list[key]->defaultValue()); return qvariant_cast<T>( this->list[key]->value()); }
    int integer( const QString &key, bool defaultValue = false ) { return Variable::instance()->value<int>( key, defaultValue ); }
    qreal decimalValue( const QString &key, bool defaultValue = false ) { return Variable::instance()->value<qreal>( key, defaultValue ); }
    bool isEnabled( const QString &key, bool defaultValue = false ) { return Variable::instance()->value<bool>( key, defaultValue ); }
    bool isDisabled( const QString &key, bool defaultValue = false ) { return !Variable::instance()->isEnabled( key, defaultValue ); }
    QString string( const QString &key, bool defaultValue = false ) { return Variable::instance()->value<QString>( key, defaultValue ); }

    template<typename T>
    void updateConnections( const QString &key, const T &value ) {
        if ( Variable::instance()->slotList.contains( key )) {
            QPair<QObject*, int> slot;

            slot = Variable::instance()->slotList[key];
            slot.first->metaObject()->method( slot.second ).invoke( slot.first, Qt::QueuedConnection, Q_ARG( QVariant, value ));
        }
    }

    template<typename T>
    void setValue( const QString &key, const T &value, bool initial = false ) {
        if ( initial ) {
            // initial read from configuration file
            Variable::instance()->list[key]->setValue( value );
        } else {
            QVariant currentValue;

            if ( !Variable::instance()->contains( key ))
                return;

            currentValue = Variable::instance()->list[key]->value();

            // any subsequent value changes emit a valueChanged signal
            if ( value != currentValue ) {
                Variable::instance()->list[key]->setValue( value );
                emit valueChanged( key );
                Variable::instance()->updateConnections( key, value );
            }
        }
    }

    template<typename T>
    void add( const QString &key, const T &value, VariableEntry::Flags flags = VariableEntry::NoFlags ) {
        this->add<VariableEntry,T>( key, value, flags );
    }

    template<class Container, typename T>
    void add( const QString &key, const T &value, VariableEntry::Flags flags = VariableEntry::NoFlags ) {
        if ( !Variable::instance()->list.contains( key ) && !key.isEmpty())
            Variable::instance()->list[key] = Container( key, QVariant( value ), flags ).copy();
    }

public slots:
    void setInteger( const QString &key, int value ) { Variable::instance()->setValue<int>( key, value ); }
    void setDecimalValue( const QString &key, qreal value ) { Variable::instance()->setValue<qreal>( key, value ); }
    void enable( const QString &key ) { Variable::instance()->setValue<bool>( key, true ); }
    void disable( const QString &key ) { Variable::instance()->setValue<bool>( key, false ); }
    void setString( const QString &key, const QString &string ) { Variable::instance()->setValue<QString>( key, string ); }
    void reset( const QString &key ) { if ( Variable::instance()->contains( key )) Variable::instance()->setValue<QVariant>( key, Variable::instance()->value<QVariant>( key, true )); }
    void bind( const QString &key, const QObject *receiver, const char *method );
    QString bind( const QString &key, QObject *object );
    QString bind( const QString &key, QWidget *widget ) { return this->bind( key, qobject_cast<QObject*>( widget )); }
    void unbind( const QString &key );
    void update( const QString &key ) { emit this->valueChanged( key ); }

signals:
    void valueChanged( const QString &key );

private slots:
    void setBoundValue( const QString &key, bool internal );

private:
    explicit Variable();
    QMap<QString, QObject*>boundVariables;
    QSignalMapper *signalMapper;
    QMap<QString, QPair<QObject*, int> > slotList;
};
