#-------------------------------------------------
#
# Project created by QtCreator 2013-07-24T11:37:06
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#CONFIG += static
TARGET = KetoEvent3
TEMPLATE = app
TRANSLATIONS = i18n/ketoevent_lv_LV.ts
win32:RC_FILE = icon.rc
ICON = icon.icns

SOURCES += main.cpp\
        gui_main.cpp \
    databaseentry.cpp \
    teamentry.cpp \
    logentry.cpp \
    taskentry.cpp \
    gui_teamedit.cpp \
    gui_teamlistmodel.cpp \
    gui_taskedit.cpp \
    gui_tasklistmodel.cpp \
    gui_rankingmodel.cpp \
    gui_rankings.cpp \
    gui_about.cpp \
    gui_license.cpp \
    taskwidget.cpp \
    settingsvariable.cpp \
    gui_settings.cpp \
    evententry.cpp \
    database.cpp \
    team.cpp \
    log.cpp \
    task.cpp \
    event.cpp \
    variable.cpp \
    sort.cpp \
    gui_console.cpp \
    gui_event.cpp \
    gui_combos.cpp \
    gui_combomodel.cpp \
    combo.cpp \
    gui_addedit.cpp \
    cmd.cpp

HEADERS  += gui_main.h \
    databaseentry.h \
    teamentry.h \
    logentry.h \
    taskentry.h \
    main.h \
    gui_teamedit.h \
    gui_teamlistmodel.h \
    gui_taskedit.h \
    gui_tasklistmodel.h \
    gui_rankingmodel.h \
    gui_rankings.h \
    gui_about.h \
    gui_license.h \
    taskwidget.h \
    consolevariable.h \
    settingsvariable.h \
    gui_settings.h \
    evententry.h \
    gui_console.h \
    gui_event.h \
    gui_settingsdialog.h \
    gui_combos.h \
    gui_combomodel.h \
    gui_addedit.h \
    cmd.h

FORMS += gui_main.ui \
    gui_teamedit.ui \
    gui_taskedit.ui \
    gui_rankings.ui \
    gui_about.ui \
    gui_license.ui \
    gui_settings.ui \
    gui_console.ui \
    gui_event.ui \
    gui_combos.ui \
    gui_addedit.ui

RESOURCES += \
    resources.qrc
