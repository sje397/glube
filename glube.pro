#-------------------------------------------------
#
# Project created by QtCreator 2014-04-30T16:24:02
#
#-------------------------------------------------

QT += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = glube
TEMPLATE = app

INCLUDEPATH += /usr/local/include

SOURCES += main.cpp \
    mainwindow.cpp \
    widget.cpp \
    chunk.cpp \
    drawable.cpp \
    mapnode.cpp \
    vao.cpp \
    simplex.c

HEADERS  += mainwindow.h \
    widget.h \
    chunk.h \
    drawable.h \
    mapnode.h \
    vao.h \
    simplex.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    vertex.shader \
    fragment.shader

RESOURCES += \
    res.qrc
