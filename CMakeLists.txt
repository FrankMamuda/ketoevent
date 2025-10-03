cmake_minimum_required(VERSION 3.19)
project(ketoevent LANGUAGES CXX)

find_package(Qt6 6.5 COMPONENTS REQUIRED Core Widgets Sql Xml LinguistTools)

qt_standard_project_setup()

file(GLOB project_sources *.cpp)
file(GLOB project_headers *.h)
file(GLOB project_ui *.ui)
file(GLOB project_res *.qrc)
file(GLOB project_ts i18n/*.ts)

if(WIN32)
    set(icon_sources icon.rc)
endif(WIN32)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_compile_definitions(FORCE_LV_LOCALE)

set(TS_FILES ${project_ts})
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "i18n")
qt_add_translation(qmFiles ${TS_FILES})

qt_add_executable(ketoevent
    WIN32 MACOSX_BUNDLE
    ${project_sources}
    ${project_headers}
    ${project_ui}
    ${project_res}
    ${icon_sources}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ketoevent
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
        Qt::Xml
)

include(GNUInstallDirs)

install(TARGETS ketoevent
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ketoevent
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
