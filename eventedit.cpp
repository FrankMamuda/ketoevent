/*
 * Copyright (C) 2018 Factory #12
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
#include "editordialog.h"
#include "event.h"
#include "main.h"
#include "eventedit.h"
#include "ui_eventedit.h"
#include "variable.h"
#include "optionswidget.h"
#include "script.h"
#include <QMessageBox>

/**
 * @brief EventEdit::EventEdit
 * @param parent
 */
EventEdit::EventEdit( QWidget *parent ) : QWidget( parent ), ui( new Ui::EventEdit ), m_edit( false ) {
    // set up ui
    this->ui->setupUi( this );

    // only visible in quick add
    this->setWindowTitle( this->tr( "Add event" ));
    this->setWindowIcon( QIcon( ":/icons/ketone" ));

    // empty event title check
    auto emptyTitle = [ this ]() {
        // warn upon empty event title
        if ( this->ui->titleEdit->text().isEmpty()) {
            QMessageBox::information( this, this->tr( "Empty event title" ), this->tr( "Please enter event title" ));
            return true;
        }
        return false;
    };

    // add button action
    this->connect( this->ui->addButton, &QPushButton::clicked, [ this, emptyTitle ]() {
        const QString eventTitle( this->ui->titleEdit->text());

        // abort on empty event title
        if ( emptyTitle())
            return;

        // abort on existing event
        if ( Event::instance()->contains( Event::Title, eventTitle ) && !this->isEditing()) {
            QMessageBox::information( this, this->tr( "Event already exists" ), this->tr( "Event already exists\nChoose a different title" ));
            return;
        }

        // if everything is ok, add a new event
        Row event = Row::Invalid;
        if ( !this->isEditing()) {
            event = Event::instance()->add( eventTitle, "", this->valueList());
        } else {
            const Row event = Event::instance()->row( EditorDialog::instance()->container->currentIndex().row());

            if ( event == Row::Invalid )
                return;

            Event::instance()->setTitle( event, eventTitle );
            Event::instance()->setOptions( event, this->valueList());
        }

        if ( event != Row::Invalid )
            MainWindow::instance()->setCurrentEvent( event );

        // close dock
        if ( EditorDialog::instance()->isDockVisible())
            EditorDialog::instance()->hideDock();
        else
            this->close();
    });

    // cancel button just closes the dialog
    this->connect( this->ui->cancelButton, &QPushButton::clicked, [ this ]() {
        if ( EditorDialog::instance()->isDockVisible())
            EditorDialog::instance()->hideDock();
        else
            this->close();
    } );

    // edit script button
    this->connect( this->ui->scriptButton, &QPushButton::clicked, [ this ]() {
        //Options options( this );
        //options.setAttribute( Qt::WA_DeleteOnClose, true );

        //options.exec();

        // ideally page would look like this:
        //
        // Event title: ______________________
        // list of all event variables:
        //   times,
        //   bonus points
        //   penalty
        // ( Edit Script ) -> opens a javascript editor
        //
        // making title the only hard coded variable, the rest
        //   being imported from a script
    } );

    // add to garbage man
    GarbageMan::instance()->add( this );
}

/**
 * @brief EventEdit::~EventEdit
 */
EventEdit::~EventEdit() {
    // disconnect lambdas
    this->disconnect( this->ui->addButton, SIGNAL( clicked()));
    this->disconnect( this->ui->cancelButton, SIGNAL( clicked()));
    this->disconnect( this->ui->titleEdit, SIGNAL( returnPressed()));

    // delete all widgets
    this->ui->optionList->clear();

    // delete ui
    delete this->ui;
}

/**
 * @brief EventEdit::reset
 * @param edit
 */
void EventEdit::reset( bool edit ) {
    this->m_edit = edit;

    // TODO: do proper garbage collection
    this->ui->optionList->clear();

    if ( !this->isEditing()) {
        // reset ui components to default values
        this->ui->titleEdit->clear();
   } else {
        // get event related variables
        const Row event = Event::instance()->row( EditorDialog::instance()->container->currentIndex().row());

        if ( event == Row::Invalid )
            return;

        this->ui->titleEdit->setText( Event::instance()->title( event ));

        // evalute current event script
        Script::instance()->evaluate( Event::instance()->script( event ));

        // get option values from database
        QStringList values( Event::instance()->options( event ).split( ";" ));

        // get default options from script
        QStringList defaultValues;
        foreach ( const QVariant &var, Script::instance()->call( "defaultValues" ).toVariant().toList())
            defaultValues << var.toString();

        // call column function
        QStringList list;
        QVariantList vars( Script::instance()->call( "options" ).toVariant().toList());
        if ( defaultValues.count() != vars.count()) {
            // TODO: context
            qCritical() << "incompatible field/value count";
            return;
        }

        for ( int y = 0; y < vars.count(); y++ ) {
            const QString value(( values.count() == vars.count()) ? values.at( y ) : defaultValues.at( y ));
            this->addWidget( vars.at( y ).toString().split( ";" ) << value );
        }
    }

    this->ui->titleEdit->setFocus();
    this->ui->addButton->setDefault( false );
    this->ui->addButton->setAutoDefault( false );
}

/**
 * @brief EventEdit::optionCount
 * @return
 */
int EventEdit::optionCount() const {
    return this->ui->optionList->count();
}

/**
 * @brief EventEdit::value
 * @param index
 * @return
 */
QVariant EventEdit::value( int index ) const {
    if ( index < 0 || index >= this->optionCount())
        return QVariant();

    OptionsWidget *widget( qobject_cast<OptionsWidget*>( this->ui->optionList->itemWidget( this->ui->optionList->item( index ))));
    return ( widget != nullptr ) ? widget->value() : QVariant();
}

/**
 * @brief EventEdit::valueList
 * @return
 */
QString EventEdit::valueList() const {
    QStringList list;

    for ( int y = 0; y < this->optionCount(); y++ )
        list << this->value( y ).toString();

    return list.join( ";" );
}

/**
 * @brief EventEdit::addWidget
 * @param parms
 */
void EventEdit::addWidget( const QStringList &parms ) {
    OptionsWidget::add( parms, this->ui->optionList );
}
