#ifndef GUI_COMBOS_H
#define GUI_COMBOS_H

#include <QDialog>

namespace Ui {
class Gui_Combos;
}

class Gui_Combos : public QDialog
{
    Q_OBJECT

public:
    explicit Gui_Combos(QWidget *parent = 0);
    ~Gui_Combos();

private slots:
    void fillTeams();

private:
    Ui::Gui_Combos *ui;
};

#endif // GUI_COMBOS_H
