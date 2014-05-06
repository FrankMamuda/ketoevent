#ifndef GUI_ADDEVENT_H
#define GUI_ADDEVENT_H

#include <QDialog>

namespace Ui {
class Gui_AddEvent;
}

class Gui_AddEvent : public QDialog
{
    Q_OBJECT

public:
    explicit Gui_AddEvent(QWidget *parent = 0);
    ~Gui_AddEvent();

private slots:
    void on_addButton_clicked();

private:
    Ui::Gui_AddEvent *ui;
};

#endif // GUI_ADDEVENT_H
