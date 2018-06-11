#pragma once

#include <QListView>

class TaskView : public QListView {
    Q_OBJECT

public:
    explicit TaskView( QWidget *parent = nullptr );
    QModelIndex check;
    QModelIndex combine;

signals:

//public slots:
//    void setParent( const QObject *parent );

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:

};
