TARGET = veterinaria
CONFIG += c++17 console
CONFIG -= qt app_bundle

INCLUDEPATH += "$$PWD/include" "$$PWD/src"
HEADERS += $$files("$$PWD/include/*.h", true) $$files("$$PWD/src/*.h", true)
SOURCES += $$files("$$PWD/src/*.cpp", true)
DISTFILES += README.md $$files("$$PWD/src/README.md", true) datos/README.md arte/README.md

DESTDIR = "$$OUT_PWD/bin"

# Permite compilar modulos antes de integrar el punto de entrada.
exists("$$PWD/src/main.cpp") {
    TEMPLATE = app
} else:!isEmpty(SOURCES) {
    TEMPLATE = lib
    CONFIG += staticlib
} else {
    TEMPLATE = aux
}

contains(CONFIG, usar_raylib) {
    win32 {
        isEmpty(RAYLIB_DIR): RAYLIB_DIR = $$(RAYLIB_DIR)
        isEmpty(RAYLIB_DIR): error(Indica RAYLIB_DIR con las carpetas include y lib de raylib.)
        !exists("$$RAYLIB_DIR/include/raylib.h"): error(Falta include/raylib.h en RAYLIB_DIR.)
        INCLUDEPATH += "$$RAYLIB_DIR/include"

        win32-g++ {
            RAYLIB_LIB = "$$RAYLIB_DIR/lib/libraylib.a"
            LIBS += "$$RAYLIB_LIB" -lopengl32 -lgdi32 -lwinmm
        } else {
            RAYLIB_LIB = "$$RAYLIB_DIR/lib/raylib.lib"
            LIBS += "$$RAYLIB_LIB" opengl32.lib gdi32.lib winmm.lib
        }

        !exists("$$RAYLIB_LIB"): error(Falta la biblioteca raylib para el compilador del kit.)
    } else {
        !packagesExist(raylib): error(Instala raylib y comprueba que pkg-config --exists raylib funcione.)
        CONFIG += link_pkgconfig
        PKGCONFIG += raylib
    }
}
