cmake_minimum_required(VERSION 3.16)

project(qronicle VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Concurrent
    Gui
    Widgets
    Qml
    Quick
    QuickLayouts
    QuickControls2
    Svg
    Xml
    Sql
    LinguistTools
)

qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

qt_add_executable(qronicle
    resources.qrc
    src/main.cpp
    src/database.hpp
    src/database.cpp
    src/history_model.hpp
    src/history_model.cpp
    src/history_search_proxy.hpp
    src/history_search_proxy.cpp
    src/message.hpp
    src/message.cpp
    src/messenger.hpp
    src/messenger.cpp
    src/kopete.hpp
    src/kopete.cpp
    src/trillian.hpp
    src/trillian.cpp
    src/facebook.hpp
    src/facebook.cpp
    src/skype.hpp
    src/skype.cpp
    src/whatsapp.hpp
    src/whatsapp.cpp
    src/psi.hpp
    src/psi.cpp
    src/amsn.hpp
    src/amsn.cpp
    src/knuddels.hpp
    src/knuddels.cpp
)

qt_add_qml_module(qronicle
    URI qronicle_qml
    VERSION 1.0
    QML_FILES
      "src/Main.qml"
    DEPENDENCIES
      QtQuick
      QtQuick.Layouts
      QtQuick.Controls
)

qt_add_translations(qronicle
    TS_FILES
      "i18n/app_de.ts"
      "i18n/app_en.ts"
    SOURCES ${PROJECT_SOURCES} # Alle deine .cpp und .qml Dateien
)

target_link_libraries(qronicle PRIVATE
    Qt6::Core
    Qt6::Concurrent
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickLayouts
    Qt6::QuickControls2
    Qt6::Svg
    Qt6::Xml
    Qt6::Sql
)

install(TARGETS qronicle RUNTIME DESTINATION bin)

install(FILES
  assets/icons/qronicle.png
  DESTINATION share/icons/hicolor/scalable/apps
)
install(FILES
  qronicle.desktop
  DESTINATION share/applications
)

set(CPACK_PACKAGE_NAME "qronicle")
set(CPACK_PACKAGE_VENDOR "Tamino Dauth")
set(CPACK_PACKAGE_CONTACT "taminojdauth@gmail.com")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Offline Qt-based messenger history visualizer")

set(CPACK_GENERATOR "DEB")

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libqt6sql6-sqlite"
    "qml6-module-qtquick-layouts"
    "qml6-module-qtquick-controls"
    "qml6-module-qtquick-templates"
    "libqt6svg6"
)

set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

include(CPack)
