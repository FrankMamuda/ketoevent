cmake_minimum_required( VERSION 3.5 )

project( ketoevent LANGUAGES CXX )

find_package( Qt5 COMPONENTS REQUIRED Core Gui Widgets Sql Xml )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_INCLUDE_CURRENT_DIR ON )

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

file( GLOB project_sources *.cpp )
file( GLOB project_headers *.h )

qt5_wrap_cpp( moc_sources ${project_headers} )

file( GLOB project_ui *.ui )

qt5_wrap_ui( ui_wrap ${project_ui} )

set( project_rcc
     resources.qrc )

qt5_add_resources( rcc_sources ${project_rcc} )

if( WIN32 )
    set( icon_sources icon.rc )
endif( WIN32 )

add_executable( ketoevent
    ${project_sources}
    ${project_headers}
    ${ui_wrap}
    ${moc_sources}
    ${rcc_sources}
    ${icon_sources}
    )

target_link_libraries( ketoevent
        PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Sql
        Qt5::Xml
		)

if( WIN32 )
    set_property( TARGET ketoevent PROPERTY WIN32_EXECUTABLE true )
endif( WIN32 )
