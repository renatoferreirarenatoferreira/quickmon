#-------------------------------------------------
#
# Project created by QtCreator 2014-12-11T23:36:15
#
#-------------------------------------------------

QT       += core gui sql network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QuickMon
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    localdata.cpp \
    hosteditor.cpp \
    pingwindow.cpp \
    dragablesqlquerymodel.cpp \
    toolslistwidget.cpp \
    hoststreeview.cpp \
    pinger.cpp \
    pingstatistics.cpp \
    qcustomplot/qcustomplot.cpp

HEADERS  += mainwindow.h \
    localdata.h \
    hosteditor.h \
    pingwindow.h \
    dragablesqlquerymodel.h \
    toolslistwidget.h \
    hoststreeview.h \
    pinger.h \
    pingstatistics.h \
    qcustomplot/qcustomplot.h

FORMS    += mainwindow.ui \
    hosteditor.ui \
    pingwindow.ui

RESOURCES += \
    MainResources.qrc

VERSION = 0.1.0
DEFINES += APPLICATION_NAME=\\\"QuickMon\\\"
DEFINES += APPLICATION_VERSION=\\\"0.1.0\\\"
