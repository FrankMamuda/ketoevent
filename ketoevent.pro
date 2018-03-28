#
# Copyright (C) 2013-2018 Factory #12
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see http://www.gnu.org/licenses/.
#
#

QT       += core gui sql xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#CONFIG += static
TARGET = ketoevent
TEMPLATE = app
TRANSLATIONS = i18n/ketoevent_lv_LV.ts
win32:RC_FILE = icon.rc
ICON = icon.icns

SOURCES += main.cpp\
    taskwidget.cpp \
    settingsvariable.cpp \
    variable.cpp \
    sort.cpp \
    combo.cpp \
    cmd.cpp \
    team.cpp \
    task.cpp \
    event.cpp \
    log.cpp \
    database.cpp \
    databaseentry.cpp \
    xmltools.cpp \
    combomodel.cpp \
    rankings.cpp \
    settings.cpp \
    mainwindow.cpp \
    taskmodel.cpp \
    teammodel.cpp \
    teamdialog.cpp \
    taskdialog.cpp \
    combodialog.cpp \
    eventdialog.cpp \
    console.cpp

HEADERS  += \
    main.h \
    taskwidget.h \
    settingsvariable.h \
    cmd.h \
    team.h \
    task.h \
    event.h \
    log.h \
    combo.h \
    database.h \
    databaseentry.h \
    variable.h \
    callonce.h \
    singleton.h \
    xmltools.h \
    about.h \
    combomodel.h \
    combodialog.h \
    console.h \
    teamlistmodel.h \
    teamdialog.h \
    settings.h \
    eventdialog.h \
    description.h \
    license.h \
    mainwindow.h \
    rankings.h \
    taskdialog.h \
    dialog.h \
    tasklistmodel.h

FORMS += \
    mainwindow.ui \
    about.ui \
    combodialog.ui \
    console.ui \
    description.ui \
    license.ui \
    eventdialog.ui \
    rankings.ui \
    settings.ui \
    taskdialog.ui \
    teamdialog.ui

RESOURCES += \
    resources.qrc
