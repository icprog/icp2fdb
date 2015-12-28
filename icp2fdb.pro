#-------------------------------------------------
#
# Project created by QtCreator 2015-07-09T08:32:29
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = icp2fdb
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    src_libmodbus/modbus-tcp.c \
    src_libmodbus/modbus.c \
    src_libmodbus/modbus-data.c \
    src_libmodbus/modbus-rtu.c

LIBS += -lws2_32

HEADERS  += mainwindow.h \
    autostopthread.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc

#add ico to windows application
RC_FILE = myapp.rc
