#-------------------------------------------------
#
# Project created by QtCreator 2018-04-18T10:32:00
#
#-------------------------------------------------

QT       += core gui sql xml concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ketoevent
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    event.cpp \
    table.cpp \
    database.cpp \
    task.cpp \
    team.cpp \
    log.cpp \
    logdelegate.cpp \
    logeditor.cpp \
    taskview.cpp \
    editordialog.cpp \
    teamtoolbar.cpp \
    tasktoolbar.cpp \
    teamedit.cpp \
    taskedit.cpp \
    rankings.cpp \
    variable.cpp \
    xmltools.cpp \
    settings.cpp \
    rankingsmodel.cpp \
    cmd.cpp \
    console.cpp

HEADERS += \
        mainwindow.h \
    field.h \
    event.h \
    table.h \
    database.h \
    task.h \
    team.h \
    log.h \
    logdelegate.h \
    logeditor.h \
    taskview.h \
    editordialog.h \
    teamtoolbar.h \
    tasktoolbar.h \
    toolbar.h \
    dockwidget.h \
    teamedit.h \
    taskedit.h \
    rankings.h \
    variable.h \
    xmltools.h \
    settings.h \
    variableentry.h \
    modalwindow.h \
    rankingsmodel.h \
    main.h \
    combobox.h \
    widget.h \
    cmd.h \
    console.h

FORMS += \
        mainwindow.ui \
    logeditor.ui \
    editordialog.ui \
    teamedit.ui \
    taskedit.ui \
    rankings.ui \
    settings.ui \
    console.ui

RESOURCES += \
    resources.qrc
