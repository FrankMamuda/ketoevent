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
#include "table.h"
#include <QAction>
#include <QCheckBox>
#include <QWidget>
#include <QLineEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QLoggingCategory>

/**
 * @brief The Widget_ namespace
 */
namespace Widget_ {
    const static QLoggingCategory Debug( "widget" );
}

//
// classes
//
class Variable;

/**
 * @brief The Widget class
 */
class Widget final : public QObject {
    Q_OBJECT
    Q_ENUMS( Types )
    friend class Variable;

public:
    enum Types {
        NoType = -1,
        CheckBox,
        Action,
        LineEdit,
        TimeEdit,
        SpinBox,
        ComboBox
    };

    /**
     * @brief type
     * @return
     */
    Types type() const { return this->m_type; }

    /**
     * @brief name
     * @return
     */
    QString name() const { return ( this->widget != nullptr ) ? this->widget->objectName() : QString(); }

    /**
     * @brief Widget
     * @param w
     */
    Widget( QWidget *w ) : m_type( NoType ), widget( w ) {
        // determine widget type
        if ( !QString::compare( widget->metaObject()->className(), "QCheckBox" )) {
            this->connection = this->connect( qobject_cast<QCheckBox*>( widget ), SIGNAL( stateChanged( int )), this, SLOT( valueChanged()));
            this->m_type = CheckBox;
        } else if ( !QString::compare( widget->metaObject()->className(), "QAction" )) {
            this->connection = this->connect( qobject_cast<QAction*>( widget ), SIGNAL( toggled( bool )), this, SLOT( valueChanged()));
            this->m_type = Action;
        } else if ( !QString::compare( widget->metaObject()->className(), "QLineEdit" )) {
            this->connection = this->connect( qobject_cast<QLineEdit*>( widget ), SIGNAL( textChanged( QString )), this, SLOT( valueChanged()));
            this->m_type = LineEdit;
        } else if ( !QString::compare( widget->metaObject()->className(), "QTimeEdit" )) {
            this->connection = this->connect( qobject_cast<QTimeEdit*>( widget ), SIGNAL( timeChanged( QTime )), this, SLOT( valueChanged()));
            this->m_type = TimeEdit;
        } else if ( !QString::compare( widget->metaObject()->className(), "QSpinBox" )) {
            this->connection = this->connect( qobject_cast<QSpinBox*>( widget ), SIGNAL( valueChanged( int )), this, SLOT( valueChanged()));
            this->m_type = SpinBox;
        } else if ( !QString::compare( widget->metaObject()->className(), "QComboBox" )) {
            this->connection = this->connect( qobject_cast<QComboBox*>( widget ), SIGNAL( currentIndexChanged( int )), this, SLOT( valueChanged()));
            this->m_type = ComboBox;
        } else {
            qCWarning( Widget_::Debug ) << this->tr( "unsupported container \"%1\"" ).arg( widget->metaObject()->className());
        }
    }

    /**
     * @brief ~Widget
     */
    ~Widget() { this->disconnect( this->connection ); }

    /**
     * @brief value
     * @return
     */
    QVariant value() const {
        if ( this->widget == nullptr )
            return QVariant();

        switch ( this->type()) {
        case CheckBox:
            return qobject_cast<QCheckBox*>( this->widget )->isChecked();

        case Action:
            return qobject_cast<QAction*>( this->widget )->isChecked();

        case LineEdit:
            return qobject_cast<QLineEdit*>( this->widget )->text();

        case TimeEdit:
            return qobject_cast<QTimeEdit*>( this->widget )->time();

        case SpinBox:
            return qobject_cast<QSpinBox*>( this->widget )->value();

        case ComboBox:
        {
            QComboBox *comboBox( qobject_cast<QComboBox*>( this->widget ));
            QAbstractItemModel *model( comboBox->model());
            if ( model != nullptr )
                return model->data( model->index( comboBox->currentIndex(), 0 ), Qt::UserRole );
        }
            break;

        case NoType:
            break;
        }

        return QVariant();
    }

public slots:
    void setValue( const QVariant &value ) {
        if ( this->widget == nullptr )
            return;

        this->widget->blockSignals( true );

        switch ( this->type()) {
        case CheckBox:
            qobject_cast<QCheckBox*>( this->widget )->setChecked( static_cast<bool>( value.toInt()));
            break;

        case Action:
            qobject_cast<QAction*>( this->widget )->setChecked( static_cast<bool>( value.toInt()));
            break;

        case LineEdit:
            qobject_cast<QLineEdit*>( this->widget )->setText( value.toString());
            break;

        case TimeEdit:
            qobject_cast<QTimeEdit*>( this->widget )->setTime( value.toTime());
            break;

        case SpinBox:
            qobject_cast<QSpinBox*>( this->widget )->setValue( value.toInt());
            break;

        case ComboBox:
        {
            QComboBox *comboBox( qobject_cast<QComboBox*>( this->widget ));
            QAbstractItemModel *model( comboBox->model());
            int y;

            if ( model != nullptr ) {
                for ( y = 0; y < model->rowCount(); y++ ) {
                    if ( model->data( model->index( y, 0 ), Qt::UserRole ) == value ) {
                        comboBox->setCurrentIndex( y );
                        break;
                    }
                }
            }
        }
            break;

        case NoType:
            break;
        }

        this->widget->blockSignals( false );
    }

private slots:
    void valueChanged() { emit this->changed( this->value()); }

signals:
    void changed( const QVariant &variant );

private:
    Types m_type;
    QWidget *widget;
    QMetaObject::Connection connection;
};
