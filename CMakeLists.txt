cmake_minimum_required(VERSION 3.10)
project(LinHub VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Qt5 QUIET COMPONENTS Core Widgets Sql)
if(Qt5_FOUND)
    set(QT_VERSION_MAJOR 5)
else()
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)
    set(QT_VERSION_MAJOR 6)
endif()

set(LINHUB_SOURCES
    src/main.cpp
    src/app/application.cpp
    src/app/appsettings.cpp
    src/core/session.cpp
    src/storage/databasemanager.cpp
    src/storage/sessionrepository.cpp
    src/storage/historyrepository.cpp
    src/storage/commandrepository.cpp
    src/storage/transferrepository.cpp
    src/pty/ptyprocess.cpp
    src/terminal/screen.cpp
    src/terminal/parser.cpp
    src/terminal/terminalwidget.cpp
    src/terminal/colorscheme.cpp
    src/net/sessionlauncher.cpp
    src/net/askpass.cpp
    src/net/sftpclient.cpp
    src/logging/sessionlogger.cpp
    src/util/crypto.cpp
    src/util/sshconfigimporter.cpp
    src/ui/mainwindow.cpp
    src/ui/sessiontreewidget.cpp
    src/ui/sessioneditdialog.cpp
    src/ui/quickconnectbar.cpp
    src/ui/composebar.cpp
    src/ui/historypanel.cpp
    src/ui/settingsdialog.cpp
    src/ui/sftpwidget.cpp
    src/ui/tabterminal.cpp
    src/ui/sessionpane.cpp
    src/ui/icons.cpp
    src/ui/startpage.cpp
    src/ui/findbar.cpp
    src/ui/authpromptdialog.cpp
    resources/linhub.qrc
)

add_executable(linhub ${LINHUB_SOURCES})

target_include_directories(linhub PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(linhub PRIVATE
    LINHUB_VERSION="${PROJECT_VERSION}"
    QT_NO_CAST_FROM_ASCII
    QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
)

target_link_libraries(linhub PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

if(UNIX AND NOT APPLE)
    target_link_libraries(linhub PRIVATE util)
    find_package(X11)
    if(X11_FOUND)
        target_link_libraries(linhub PRIVATE ${X11_LIBRARIES})
        target_include_directories(linhub PRIVATE ${X11_INCLUDE_DIR})
        target_compile_definitions(linhub PRIVATE LINHUB_HAVE_X11)
    endif()
endif()

if(MSVC)
    target_compile_options(linhub PRIVATE /utf-8 /W4)
else()
    target_compile_options(linhub PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(GNUInstallDirs)

install(TARGETS linhub RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES resources/linhub.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES resources/icons/linhub.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(FILES resources/icons/linhub.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps RENAME linhub.png)
install(FILES resources/icons/linhub.png DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps RENAME linhub.png)

if(UNIX AND NOT APPLE)
    add_executable(linhub-askpass src/net/askpass_main.cpp src/net/askpass.cpp)
    target_include_directories(linhub-askpass PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(linhub-askpass PRIVATE Qt${QT_VERSION_MAJOR}::Core)
    install(TARGETS linhub-askpass RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/linhub)
endif()
