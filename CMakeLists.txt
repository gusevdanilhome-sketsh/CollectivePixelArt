cmake_minimum_required(VERSION 3.19)
project(CollectivePixelArt LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(CollectivePixelArt
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    canvaswidget.h canvaswidget.cpp
    colorpalette.h colorpalette.cpp
    spritewidget.h spritewidget.cpp
)

qt_add_translations(
    TARGETS CollectivePixelArt
    TS_FILES CollectivePixelArt_ru_RU.ts
)

target_link_libraries(CollectivePixelArt
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS CollectivePixelArt
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET CollectivePixelArt
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
