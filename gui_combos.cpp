#include "gui_combos.h"
#include "ui_gui_combos.h"
#include "gui_main.h"

Gui_Combos::Gui_Combos( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_Combos ) {
    ui->setupUi(this);
    this->fillTeams();
}

Gui_Combos::~Gui_Combos() {
    delete ui;
}

/*
================
fillTeams
================
*/
void Gui_Combos::fillTeams() {
    // abort if partially initialized
    if ( !m.isInitialized())
        return;

    // clear list
    this->ui->comboTeams->clear();

    // repopulate list
    foreach ( TeamEntry *teamPtr, m.teamList )
        this->ui->comboTeams->addItem( teamPtr->name(), teamPtr->id());

    // set to current team
    Gui_Main *mPtr = qobject_cast<Gui_Main*>( this->parent());
    this->ui->comboTeams->setCurrentIndex( mPtr->currentTeamIndex());
}
