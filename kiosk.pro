TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

CONFIG -= c++11
CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17
QMAKE_LFLAGS += -std=c++17
LIBS += -lstdc++fs

PACKAGES = sdl2 SDL2_image

QMAKE_CFLAGS   += $$system("pkg-config --cflags $$PACKAGES | sed 's|-I|-isystem |g'")
QMAKE_CXXFLAGS += $$system("pkg-config --cflags $$PACKAGES | sed 's|-I|-isystem |g'")
QMAKE_LFLAGS   += $$system("pkg-config --libs   $$PACKAGES")

SOURCES += \
        main.cpp \
    modules/screensaver.cpp \
    module.cpp \
    modules/mainmenu.cpp \
    gui_module.cpp \
    widget.cpp \
    widgets/button.cpp \
    modules/lightroom.cpp

HEADERS += \
    modules/screensaver.hpp \
    module.hpp \
    kiosk.hpp \
    modules/mainmenu.hpp \
    gui_module.hpp \
    widget.hpp \
    widgets/button.hpp \
    modules/lightroom.hpp
