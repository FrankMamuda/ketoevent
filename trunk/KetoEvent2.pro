#
# Copyright (C) 2012 Edd 'Double Dee' Psycho
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

QT       += core gui xml

TARGET = KetoEvent2
TEMPLATE = app
#CONFIG += static
win32:RC_FILE = icon.rc
SOURCES +=\
        gui_teamedit.cpp \
    app_taskentry.cpp \
    app_tasklog.cpp \
    app_teamentry.cpp \
    app_main.cpp \
    sys_common.cpp \
    app_logparser.cpp \
    app_taskparser.cpp \
    sys_filesystem.cpp \
    sys_cvar.cpp \
    sys_cvarfunc.cpp \
    gui_teamlistmodel.cpp \
    gui_main.cpp \
    gui_taskedit.cpp \
    gui_tasklog.cpp \
    gui_tasklistmodel.cpp \
    gui_rankingmodel.cpp \
    gui_rankings.cpp \
    gui_settings.cpp \
    gui_settingscvar.cpp \
    gui_about.cpp \
    gui_license.cpp

HEADERS  += gui_teamedit.h \
    app_tasklog.h \
    app_taskentry.h \
    app_teamentry.h \
    app_main.h \
    sys_common.h \
    sys_shared.h \
    app_logparser.h \
    app_taskparser.h \
    sys_filesystem.h \
    sys_cvar.h \
    sys_cvarfunc.h \
    gui_teamlistmodel.h \
    gui_main.h \
    gui_taskedit.h \
    gui_tasklistmodel.h \
    gui_tasklog.h \
    gui_rankingmodel.h \
    gui_rankings.h \
    gui_settings.h \
    gui_settingscvar.h \
    gui_about.h \
    gui_license.h

FORMS    += gui_teamedit.ui \
    gui_main.ui \
    gui_taskedit.ui \
    gui_rankings.ui \
    gui_settings.ui \
    gui_about.ui \
    gui_license.ui

RESOURCES += \
    res_main.qrc

TRANSLATIONS = i18n/KetoEvent_lv_LV.ts

OTHER_FILES += \
    CHANGELOG.txt



