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

/**
 * @brief OptionsWidget::OptionsWidget
 * @param parent
 */
OptionsWidget::OptionsWidget( const Types &type, const QString &label, const QVariant &value, QWidget *parent ) : QWidget( parent ) {
    switch ( type ) {
    case Integer:
    {
        QSpinBox *spinBox( new QSpinBox());
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
