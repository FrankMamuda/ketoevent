#
# Copyright (C) 2013-2016 Avotu Briezhaudzetava
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

QT       += core gui sql xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#CONFIG += static
TARGET = ketoevent
TEMPLATE = app
TRANSLATIONS = i18n/ketoevent_lv_LV.ts
win32:RC_FILE = icon.rc
ICON = icon.icns

SOURCES += main.cpp\
        gui_main.cpp \
    teamentry.cpp \
    logentry.cpp \
    taskentry.cpp \
    gui_teamlistmodel.cpp \
    gui_tasklistmodel.cpp \
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
    gui_combos.cpp \
    gui_combomodel.cpp \
    combo.cpp \
    gui_addedit.cpp \
    cmd.cpp \
    gui_event.cpp \
    gui_team.cpp \
    gui_task.cpp \
    databaseentry.cpp \
    gui_description.cpp

HEADERS  += gui_main.h \
    databaseentry.h \
    teamentry.h \
    logentry.h \
    taskentry.h \
    main.h \
    gui_teamlistmodel.h \
    gui_tasklistmodel.h \
    gui_rankings.h \
    gui_about.h \
    gui_license.h \
    taskwidget.h \
    consolevariable.h \
    settingsvariable.h \
    gui_settings.h \
    evententry.h \
    gui_console.h \
    gui_combos.h \
    gui_combomodel.h \
    gui_addedit.h \
    cmd.h \
    gui_event.h \
    gui_team.h \
    gui_dialog.h \
    gui_task.h \
    gui_description.h

FORMS += gui_main.ui \
    gui_about.ui \
    gui_license.ui \
    gui_console.ui \
    gui_combos.ui \
    gui_addedit.ui \
    gui_event.ui \
    gui_team.ui \
    gui_task.ui \
    gui_rankings.ui \
    gui_settings.ui \
    gui_description.ui

RESOURCES += \
    resources.qrc
