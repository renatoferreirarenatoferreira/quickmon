#-------------------------------------------------
#
# Project created by QtCreator 2014-12-11T23:36:15
#
#-------------------------------------------------

QT       += core gui sql network printsupport xml svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QuickMon
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    localdata.cpp \
    hosteditor.cpp \
    Tools/pingwindow.cpp \
    dragablesqlquerymodel.cpp \
    toolslistwidget.cpp \
    hoststreeview.cpp \
    Net/pinger.cpp \
    Tools/pingstatistics.cpp \
    qcustomplot/qcustomplot.cpp \
    Tools/traceroutewindow.cpp \
    Tools/tracerouteitem.cpp \
    Tools/tracerouteitemmanager.cpp \
    snmpdatatreewidget.cpp \
    Tools/snmplistwindow.cpp \
    Tools/snmpgraphwindow.cpp \
    Tools/snmptablewindow.cpp \
    Net/snmpclient.cpp \
    Tools/snmplistitem.cpp \
    Tools/snmpgraphlegenditem.cpp \
    Tools/snmpstatistics.cpp \
    Utils/snmptextualconventions.cpp

HEADERS  += mainwindow.h \
    localdata.h \
    hosteditor.h \
    Tools/pingwindow.h \
    dragablesqlquerymodel.h \
    toolslistwidget.h \
    hoststreeview.h \
    Net/pinger.h \
    Tools/pingstatistics.h \
    qcustomplot/qcustomplot.h \
    Tools/traceroutewindow.h \
    Tools/tracerouteitem.h \
    Tools/tracerouteitemmanager.h \
    snmpdatatreewidget.h \
    Tools/snmplistwindow.h \
    Tools/snmpgraphwindow.h \
    Tools/snmptablewindow.h \
    Net/snmpclient.h \
    Tools/snmplistitem.h \
    Tools/snmpgraphlegenditem.h \
    Tools/snmpstatistics.h \
    Utils/snmptextualconventions.h

FORMS    += mainwindow.ui \
    hosteditor.ui \
    Tools/pingwindow.ui \
    Tools/traceroutewindow.ui \
    Tools/snmplistwindow.ui \
    Tools/snmpgraphwindow.ui \
    Tools/snmptablewindow.ui

RESOURCES += \
    MainResources.qrc

VERSION = 0.2.0
DEFINES += APPLICATION_NAME=\\\"QuickMon\\\"
DEFINES += APPLICATION_VERSION=\\\"0.2.0\\\"

QMAKE_TARGET_COMPANY = "Renato A. Ferreira"
QMAKE_TARGET_DESCRIPTION = "Network troubleshooting tool."
QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2015 Renato A. Ferreira"
QMAKE_TARGET_PRODUCT = "QuickMon"

win32:RC_ICONS = quickmon.ico

Release:CONFIG += static

win32:QMAKE_CFLAGS_RELEASE += /MT
win32:QMAKE_CXXFLAGS_RELEASE += /MT
win32:QMAKE_CFLAGS_RELEASE -= -MD
win32:QMAKE_CXXFLAGS_RELEASE -= -MD
