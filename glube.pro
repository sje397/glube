#-------------------------------------------------
#
# Project created by QtCreator 2014-04-30T16:24:02
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = glube
TEMPLATE = app


SOURCES += main.cpp \
    mainwindow.cpp \
    widget.cpp

HEADERS  += mainwindow.h \
    widget.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    vertex.shader \
    fragment.shader

RESOURCES += \
    res.qrc
