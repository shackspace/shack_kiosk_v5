TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

CONFIG -= c++11
CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17
QMAKE_LFLAGS += -std=c++17
LIBS += -lstdc++fs
CONFIG += static

#PACKAGES = SDL2_image
#QMAKE_CFLAGS   += $$system("pkg-config --cflags $$PACKAGES | sed 's|-I|-isystem |g'")
#QMAKE_CXXFLAGS += $$system("pkg-config --cflags $$PACKAGES | sed 's|-I|-isystem |g'")
#QMAKE_LFLAGS   += $$system("pkg-config --libs   $$PACKAGES")

INCLUDEPATH += /home/felix/projects/SDL/include
LIBS += /home/felix/projects/SDL/build/libSDL2.a

INCLUDEPATH += /home/felix/projects/SDL_image
LIBS += /home/felix/projects/SDL_image/.libs/libSDL2_image.a

LIBS += -pthread -ldl  -lcurl

include(/home/felix/projects/xqlib/pri/stb.pri)
include(/home/felix/projects/xqlib/pri/json.pri)

SOURCES += \
    main.cpp \
    modules/screensaver.cpp \
    module.cpp \
    modules/mainmenu.cpp \
    gui_module.cpp \
    widget.cpp \
    widgets/button.cpp \
    modules/lightroom.cpp \
    modules/tramview.cpp \
    http_client.cpp

HEADERS += \
    modules/screensaver.hpp \
    module.hpp \
    kiosk.hpp \
    modules/mainmenu.hpp \
    gui_module.hpp \
    widget.hpp \
    widgets/button.hpp \
    modules/lightroom.hpp \
    modules/tramview.hpp \
    http_client.hpp
