
//
// includes
//
#include "gui_teamedit.h"
#include "app_teamentry.h"
#include "app_main.h"
#include "ui_gui_teamedit.h"
#include <QMessageBox>
#include "sys_filesystem.h"

//
// cvars
//
extern pCvar *k_eventStartTime;
extern pCvar *k_eventFinishTime;
extern pCvar *k_minimumTeamMembers;
extern pCvar *k_maximumTeamMembers;

/*
================
construct
================
*/
Gui_TeamEdit::Gui_TeamEdit( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_TeamEdit ) {
    ui->setupUi( this );

    // set up view
    this->listModelPtr = new Gui_TeamListModel( this );
    this->ui->teamList->setModel( this->listModelPtr );
    this->ui->teamList->setAlternatingRowColors( true );
    this->ui->startTimeEdit->setMinimumTime( k_eventStartTime->time());
    this->ui->finishTimeEdit->setMinimumTime( k_eventStartTime->time());
    this->ui->teamMembersEdit->setMinimum( k_minimumTeamMembers->integer());
    this->ui->teamMembersEdit->setMaximum( k_maximumTeamMembers->integer());

    // hide add/edit widget
    this->toggleAddEditWidget( NoState );

    // connect
    this->connect( this->ui->closeButton, SIGNAL( clicked()), this, SLOT( close()));
}

/*
================
enableView
================
*/
void Gui_TeamEdit::enableView() {
    this->ui->addTeamButton->setEnabled( true );
    this->ui->removeTeamButton->setEnabled( true );
    this->ui->editTeamButton->setEnabled( true );
    this->ui->closeButton->setEnabled( true );
    this->ui->teamList->setEnabled( true );
    this->ui->teamList->setSelectionMode( QAbstractItemView::SingleSelection );
    this->ui->closeButton->setDefault( true );
}

/*
================
destruct
================
*/
Gui_TeamEdit::~Gui_TeamEdit() {
    delete ui;
    delete this->listModelPtr;
}

/*
================
toggleAddEditWidget
================
*/
void Gui_TeamEdit::toggleAddEditWidget( AddEditState state ) {
    this->setState( state );

    if ( !this->ui->addEditWidget->isHidden()) {
        this->ui->addEditWidget->close();
        this->ui->teamList->setEnabled( true );
        this->enableView();
    } else {
        App_TeamEntry *entryPtr = NULL;

        // disable everything
        this->ui->addEditWidget->show();
        this->ui->addTeamButton->setDisabled( true );
        this->ui->removeTeamButton->setDisabled( true );
        this->ui->editTeamButton->setDisabled( true );
        this->ui->closeButton->setDisabled( true );
        this->ui->doneButton->setDefault( true );
        this->ui->teamList->setDisabled( true );

        switch ( state ) {
        case Add:
            this->ui->teamNameEdit->clear();
            this->ui->startTimeEdit->setTime( k_eventStartTime->time());
            this->ui->finishTimeEdit->setTime( k_eventStartTime->time());
            this->ui->teamMembersEdit->setValue( k_minimumTeamMembers->value());
            this->ui->addEditWidget->setWindowTitle( this->tr( "Add team" ));
            break;

        case Edit:
            // match name to be sure
            entryPtr = m.findTeamByName( this->ui->teamList->model()->data( this->ui->teamList->currentIndex(), Qt::DisplayRole ).toString());

            if ( entryPtr == NULL ) {
                this->toggleAddEditWidget( NoState );
                return;
            }

            this->ui->teamNameEdit->setText( entryPtr->name());
            this->ui->startTimeEdit->setTime( entryPtr->startTime());
            this->ui->finishTimeEdit->setTime( entryPtr->finishTime());
            this->ui->teamMembersEdit->setValue( entryPtr->members());
            this->ui->addEditWidget->setWindowTitle( this->tr( "Edit team" ));
            break;

            // bad state
        case NoState:
        default:
            return;
        }
    }
}

/*
================
removeTeamButton->clicked
================
*/
void Gui_TeamEdit::on_removeTeamButton_clicked() {
    int state;

    // match actual names as failsafe
    App_TeamEntry *entryPtr = m.findTeamByName( this->ui->teamList->model()->data( this->ui->teamList->currentIndex(), Qt::DisplayRole ).toString());
    if ( entryPtr != NULL ) {
        QMessageBox msgBox;
        msgBox.setText( this->tr( "Do you really want to remove \"%1\"?" ).arg( entryPtr->name()));
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
        msgBox.setDefaultButton( QMessageBox::Yes );
        msgBox.setIcon( QMessageBox::Warning );
        msgBox.setWindowIcon( QIcon( ":/icons/team_delete_22" ));
        state = msgBox.exec();

        // check options
        switch ( state ) {
        case QMessageBox::Yes:
            m.removeTeam( entryPtr );
            this->listModelPtr->resetModelData();
            break;

        case QMessageBox::No:
        default:
            return;
        }
    }
}

/*
================
doneButton->clicked
================
*/
void Gui_TeamEdit::on_doneButton_clicked() {
    if ( this->ui->teamNameEdit->text().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText( this->tr( "Please specify team name" ));
        msgBox.setIcon( QMessageBox::Information );
        msgBox.exec();
        return;
    }

    if ( this->state() == Add ) {
        App_TeamEntry *teamPtr = new App_TeamEntry( this->ui->teamNameEdit->text(), this->ui->teamMembersEdit->value(), this->ui->startTimeEdit->time(), this->ui->finishTimeEdit->time());
        teamPtr->setFilename( fs.safeTeamFilename( this->ui->teamNameEdit->text()));
        m.addTeam( teamPtr );

        // save just in case
        teamPtr->save();
    } else if ( this->state() == Edit ) {
        // match name to be sure
        App_TeamEntry *entryPtr = m.findTeamByName( this->ui->teamList->model()->data( this->ui->teamList->currentIndex(), Qt::DisplayRole ).toString());

        if ( entryPtr == NULL ) {
            this->toggleAddEditWidget( NoState );
            return;
        }

        // set edited data
        entryPtr->setName( this->ui->teamNameEdit->text());
        entryPtr->setStartTime( this->ui->startTimeEdit->time());
        entryPtr->setFinishTime( this->ui->finishTimeEdit->time());
        entryPtr->setMembers( this->ui->teamMembersEdit->value());

        // save just in case
        entryPtr->save();
        m.updateTeamList();
    }
    this->toggleAddEditWidget( NoState );
    this->listModelPtr->resetModelData();
}
