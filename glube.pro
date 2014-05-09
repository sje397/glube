#-------------------------------------------------
#
# Project created by QtCreator 2014-04-30T16:24:02
#
#-------------------------------------------------

QT += core gui opengl
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = glube
TEMPLATE = app

INCLUDEPATH += /usr/local/include

LIBS += -l boost_thread-mt

SOURCES += main.cpp \
    mainwindow.cpp \
    widget.cpp \
    chunk.cpp \
    drawable.cpp \
    mapnode.cpp \
    vao.cpp \
    simplex.c \
    camera.cpp

HEADERS  += mainwindow.h \
    widget.h \
    chunk.h \
    drawable.h \
    mapnode.h \
    vao.h \
    simplex.h \
    camera.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    vertex.shader \
    fragment.shader

RESOURCES += \
    res.qrc
