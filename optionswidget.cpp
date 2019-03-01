/*
 * Copyright (C) 2019 Factory #12
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
#include "optionswidget.h"
#include "event.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimeEdit>
#include <QDebug>

/**
 * @brief OptionsWidget::OptionsWidget
 * @param parent
 */
OptionsWidget::OptionsWidget( const Types &type, const QString &label, const QVariant &value, QWidget *parent ) : QWidget( parent ) {
    switch ( type ) {
    case Integer:
    {
        QSpinBox *spinBox( new QSpinBox());
        spinBox->setMaximum( std::numeric_limits<int>::max());
        spinBox->setValue( value.toInt());
        this->widget = qobject_cast<QWidget*>( spinBox );
    }
        break;

    case Time:
    {
        QTimeEdit *timeEdit( new QTimeEdit());
        timeEdit->setTime( value.toTime());
        this->widget = qobject_cast<QWidget*>( timeEdit );
    }
        break;

    case String:
    {
        QLineEdit *lineEdit( new QLineEdit());
        lineEdit->setText( value.toString());
        this->widget = qobject_cast<QWidget*>( lineEdit );
    }
        break;

    case Bool:
    {
        QCheckBox *checkBox( new QCheckBox());
        checkBox->setChecked( value.toBool());
        this->widget = qobject_cast<QWidget*>( checkBox );
    }
        break;

    case Double:
    {
        QDoubleSpinBox *doubleBox( new QDoubleSpinBox());
        doubleBox->setMaximum( std::numeric_limits<double>::max());
        doubleBox->setValue( value.toDouble());
        this->widget = qobject_cast<QWidget*>( doubleBox );
    }
        break;

    case NoType:
        return;
    }

    this->label->setText( label );
    this->optionsLayout->addWidget( this->label );
    this->optionsLayout->addWidget( this->widget );
    this->setLayout( this->optionsLayout );
}

/**
 * @brief OptionsWidget::~OptionsWidget
 */
OptionsWidget::~OptionsWidget() {
    delete this->label;
    delete this->widget;
    delete this->optionsLayout;
}

/**
 * @brief EventEdit::addWidget
 * @param parms
 */
void OptionsWidget::add( const QStringList &options, QListWidget *container ) {
    if ( options.count() < 3 || container == nullptr )
        return;

    // parse values
    Types type = OptionsWidget::NoType;
    const QString label( options.at( 1 ));
    const QVariant value( QVariant( options.at( 2 )));

    if ( !QString::compare( options.at( 0 ), "bool" ))
        type = OptionsWidget::Bool;
    else if ( !QString::compare( options.at( 0 ), "string" ))
        type = OptionsWidget::String;
    else if ( !QString::compare( options.at( 0 ), "integer" ))
        type = OptionsWidget::Integer;
    else if ( !QString::compare( options.at( 0 ), "time" ))
        type = OptionsWidget::Time;
    else if ( !QString::compare( options.at( 0 ), "double" ))
        type = OptionsWidget::Double;

    // unsupported widget
    if ( type == OptionsWidget::NoType )
        return;

    OptionsWidget *widget( new OptionsWidget( type, label, value, container ));
    widget->item = new QListWidgetItem( container );
    widget->item->setSizeHint( widget->sizeHint());
    widget->setType( type );
    container->setItemWidget( widget->item, widget );
    widget->show();
}

/**
 * @brief OptionsWidget::value
 * @return
 */
QVariant OptionsWidget::value() const {
    switch ( this->type()) {
    case Integer:
        return qobject_cast<QSpinBox*>( this->widget )->value();

    case Time:
        return qobject_cast<QTimeEdit*>( this->widget )->time().toString( Database_::TimeFormat );

    case String:
        return qobject_cast<QLineEdit*>( this->widget )->text();

    case Bool:
        return qobject_cast<QCheckBox*>( this->widget )->isChecked();

    case Double:
        return qobject_cast<QDoubleSpinBox*>( this->widget )->value();

    case NoType:
        break;
    }

    return QVariant();
}
