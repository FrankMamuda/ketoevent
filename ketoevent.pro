#-------------------------------------------------
#
# Project created by QtCreator 2018-04-18T10:32:00
#
#-------------------------------------------------

QT       += core gui sql xml concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ketoevent
TEMPLATE = app

CONFIG += c++14
CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    event.cpp \
    table.cpp \
    database.cpp \
    task.cpp \
    team.cpp \
    log.cpp \
    taskview.cpp \
    editordialog.cpp \
    teamtoolbar.cpp \
    tasktoolbar.cpp \
    teamedit.cpp \
    taskedit.cpp \
    rankings.cpp \
    theme.cpp \
    variable.cpp \
    xmltools.cpp \
    settings.cpp \
    rankingsmodel.cpp \
    cmd.cpp \
    console.cpp \
    combos.cpp \
    combomodel.cpp \
    delegate.cpp \
    item.cpp \
    eventedit.cpp \
    eventtoolbar.cpp \
    popup.cpp \
    about.cpp

HEADERS += \
    mainwindow.h \
    event.h \
    table.h \
    database.h \
    task.h \
    team.h \
    log.h \
    taskview.h \
    editordialog.h \
    teamtoolbar.h \
    tasktoolbar.h \
    theme.h \
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
    console.h \
    combos.h \
    combomodel.h \
    delegate.h \
    item.h \
    eventedit.h \
    eventtoolbar.h \
    popup.h \
    about.h

FORMS += \
        mainwindow.ui \
    editordialog.ui \
    teamedit.ui \
    taskedit.ui \
    rankings.ui \
    settings.ui \
    console.ui \
    combos.ui \
    eventedit.ui \
    about.ui

RESOURCES += \
    dark.qrc \
    light.qrc \
    resources.qrc

win32:RC_FILE = icon.rc

# custom sqlite lib switch
CONFIG += sqlite_custom
sqlite_custom {
SOURCES += 3rdparty/sqlite-amalgamation/sqlite3.c
HEADERS += 3rdparty/sqlite-amalgamation/sqlite3.h
INCLUDEPATH += $$PWD/3rdparty/sqlite-amalgamation
DEFINES += SQLITE_CUSTOM
}

# latvian locale
DEFINES += FORCE_LV_LOCALE
TRANSLATIONS = i18n/ketoevent_lv_LV.ts

# special stats for KK6
#DEFINES += KK6_SPECIAL

TRANSLATION_TARGET_DIR = $${_PRO_FILE_PWD_}/i18n/

isEmpty(QMAKE_LUPDATE) {
    win32:LANGUPD = $$[QT_INSTALL_BINS]\lupdate.exe
    else:LANGUPD = $$[QT_INSTALL_BINS]/lupdate
}

isEmpty(QMAKE_LRELEASE) {
    win32:LANGREL = $$[QT_INSTALL_BINS]\lrelease.exe
    else:LANGREL = $$[QT_INSTALL_BINS]/lrelease
}

langupd.command = $$LANGUPD $$shell_path($$_PRO_FILE_) -ts $$_PRO_FILE_PWD_/$$TRANSLATIONS
langrel.depends = langupd
langrel.input = TRANSLATIONS
langrel.output = $$TRANSLATION_TARGET_DIR/${QMAKE_FILE_BASE}.qm
langrel.commands = $$LANGREL ${QMAKE_FILE_IN} -qm $$TRANSLATION_TARGET_DIR/${QMAKE_FILE_BASE}.qm
langrel.CONFIG += no_link

QMAKE_EXTRA_TARGETS += langupd
QMAKE_EXTRA_COMPILERS += langrel
PRE_TARGETDEPS += langupd compiler_langrel_make_all

CONFIG += xlsx_support
xlsx_support {
include(3rdparty/QXlsx/QXlsx.pri)
DEFINES += XLSX_SUPPORT
}
