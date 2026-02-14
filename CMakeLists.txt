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
    Sql
)

qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

qt_add_executable(chronicle
    resources.qrc
    src/main.cpp
    src/history_model.hpp
    src/history_search_proxy.hpp
    src/message.hpp
    src/message.cpp
    src/messenger.hpp
    src/kopete.hpp
    src/kopete.cpp
    src/facebook.hpp
    src/facebook.cpp
    src/skype.hpp
    src/skype.cpp
)

qt_add_qml_module(chronicle
    URI Chronicle
    VERSION 1.0
    QML_FILES "src/Main.qml"
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
    Qt6::Sql
)

qt_finalize_executable(chronicle)
