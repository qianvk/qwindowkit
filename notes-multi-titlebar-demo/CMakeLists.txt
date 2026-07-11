cmake_minimum_required(VERSION 3.21)

project(NotesMultiTitlebarDemo
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.10 REQUIRED COMPONENTS Core Gui Widgets)
#find_package(QWindowKit REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

set(_demo_sources
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/demostyle.h
    src/demostyle.cpp
    src/sidebartogglebutton.h
    src/sidebartogglebutton.cpp
)

if(WIN32)
    list(APPEND _demo_sources src/mainwindow_win.cpp)
elseif(APPLE)
    list(APPEND _demo_sources src/mainwindow_mac.cpp)
else()
    list(APPEND _demo_sources src/mainwindow_generic.cpp)
endif()

qt_add_executable(NotesMultiTitlebarDemo WIN32 ${_demo_sources})

target_link_libraries(NotesMultiTitlebarDemo
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        QWindowKit::Widgets
)

if(WIN32)
    target_compile_definitions(NotesMultiTitlebarDemo PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()
