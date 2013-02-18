#-------------------------------------------------
#
# Project created by QtCreator 2013-02-03T20:52:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qrcode_time_reader
TEMPLATE = app

INCLUDEPATH += /usr/include/opencv/

LIBS += -lexiv2 -ldecodeqr -lopencv_highgui -lopencv_core

SOURCES += main.cpp\
        qrcode_time_reader.cpp \
    resizinglabel.cpp

HEADERS  += qrcode_time_reader.h \
    resizinglabel.h

FORMS    += qrcode_time_reader.ui
