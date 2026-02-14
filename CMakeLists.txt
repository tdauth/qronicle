cmake_minimum_required(VERSION 3.16)

project(chronicle VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Qml
    Quick
    QuickLayouts
    QuickControls2
    Xml
)

qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)

qt_add_executable(chronicle
    src/gui/main.cpp
    src/gui/history_model.hpp
    src/gui/message.hpp
    src/gui/message.cpp
    src/gui/messenger.hpp
    src/gui/kopete.hpp
    src/gui/kopete.cpp
    src/gui/facebook.hpp
    src/gui/facebook.cpp
)

qt_add_qml_module(chronicle
    URI Chronicle
    VERSION 1.0
    QML_FILES "src/gui/Main.qml"
    DEPENDENCIES QtQuick QtQuick.Layouts QtQuick.Controls
)

target_link_libraries(chronicle PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickLayouts
    Qt6::QuickControls2
    Qt6::Xml
)

qt_finalize_executable(chronicle)
