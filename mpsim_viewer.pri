TOP_DIR   = $$PWD
MPSIM_DIR = $$TOP_DIR

DEFINES += WITH_QT

######################################################################  PROJECT NAME
PROJECT_MAIN = main.cpp

######################################################################  RELATIVE PATHS
SRC_DIR    = $$TOP_DIR/src
GLM_DIR    = $$MPSIM_DIR/glm
GL3W_DIR   = $$MPSIM_DIR/gl3w
MSRC_DIR   = $$MPSIM_DIR/src
SHD_DIR    = $$MPSIM_DIR/shaders

######################################################################  HEADERS and SOURCES

MY_HEADERS  = $$SRC_DIR/qtdefs.h \
              $$SRC_DIR/MainWindow.h \
              $$SRC_DIR/OpenGL2d.h \
              $$SRC_DIR/OpenGL3d.h \
              $$SRC_DIR/SystemData.h \
              $$SRC_DIR/SystemView.h \
              $$SRC_DIR/DoubleEdit.h \
              $$SRC_DIR/GLShader.h \
              $$SRC_DIR/Camera.h \
              $$SRC_DIR/glutils.h

MY_SOURCES  = $$SRC_DIR/MainWindow.cpp \
              $$SRC_DIR/OpenGL2d.cpp \
              $$SRC_DIR/OpenGL3d.cpp \
              $$SRC_DIR/SystemData.cpp \
              $$SRC_DIR/SystemView.cpp \
              $$SRC_DIR/DoubleEdit.cpp \
              $$SRC_DIR/GLShader.cpp \
              $$SRC_DIR/Camera.cpp \
              $$SRC_DIR/glutils.cpp


######################################################################  INCLUDE and DEPEND

HEADERS += $$GL3W_DIR/GL3/gl3.h $$GL3W_DIR/GL3/gl3w.h
SOURCES += $$GL3W_DIR/gl3w.c

HEADERS += $$GLM_DIR/glm.hpp \
           $$GLM_DIR/core/*.hpp \
           $$GLM_DIR/gtc/*.hpp \
           $$GLM_DIR/gtx/*.hpp

INCLUDEPATH +=  . .. $$SRC_DIR $$GL3W_DIR $$GLM_DIR  $$MSRC_DIR

######################################################################  intermediate moc and object files
CONFIG(debug, debug|release) {
    TARGET       = MPSimd
    DESTDIR      = $$TOP_DIR
    MOC_DIR      = $$TOP_DIR/compiled/debug/moc
    OBJECTS_DIR  = $$TOP_DIR/compiled/debug/object
    RCC_DIR      = $$TOP_DIR/compiled/debug
}
CONFIG(release, debug|release) {
    TARGET       = MPSim
    DESTDIR      = $$TOP_DIR
    MOC_DIR      = $$TOP_DIR/compiled/release/moc
    OBJECTS_DIR  = $$TOP_DIR/compiled/release/object
    RCC_DIR      = $$TOP_DIR/compiled/release
}

######################################################################  OTHER_FILES: SHADERS
OTHER_FILES += $$SHD_DIR/quad.frag \
               $$SHD_DIR/quad.vert \
               $$SHD_DIR/line.vert \
               $$SHD_DIR/line.frag \
               $$SHD_DIR/rod.vert \
               $$SHD_DIR/rod.geom \
               $$SHD_DIR/rod.frag \
               $$SHD_DIR/pendulum.vert \
               $$SHD_DIR/pendulum.frag \
               $$SHD_DIR/pendulum.comp \
               $$SHD_DIR/magnet.vert \
               $$SHD_DIR/magnet.geom \
               $$SHD_DIR/magnet.frag \
               $$SHD_DIR/scene.vert \
               $$SHD_DIR/scene.geom \
               $$SHD_DIR/scene.frag \
               resources/viewer.css

#RC_FILE = resources/mpphys_app_icon.rc


######################################################################  feste Angaben
CONFIG   += console warn_on
QT       += core gui opengl script
TEMPLATE  = app

contains(QT_VERSION, ^5\\.[0-6]\\..*) {
QT += widgets
}

######################################################################  Input
HEADERS += $$MY_HEADERS
SOURCES += $$MY_SOURCES $$PROJECT_MAIN

RESOURCES += mpsim_viewer.qrc


unix:!macx {    
    system("mkdir -p qtlibs")
    LIBS += -ldl
    QMAKE_CXXFLAGS += -Wall -Wno-comment
    QMAKE_LFLAGS = -Lqtlibs -Wl,-rpath qtlibs $$QMAKE_LFLAGS
}

win32 {
    INCLUDEPATH += $$TOP_DIR/../windows/
    HEADERS += wglext.h
}
