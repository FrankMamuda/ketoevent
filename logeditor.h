#ifndef LOGEDITOR_H
#define LOGEDITOR_H

#include <QWidget>

namespace Ui {
class LogEditor;
}

class LogEditor : public QWidget
{
    Q_OBJECT

public:
    explicit LogEditor(QWidget *parent = 0);
    ~LogEditor();

public slots:
    void setName( const QString &name );

private:
    Ui::LogEditor *ui;
};

#endif // LOGEDITOR_H
