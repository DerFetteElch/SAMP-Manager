#-------------------------------------------------
#
# Project created by QtCreator 2011-07-25T11:30:31
#
#-------------------------------------------------

QT       += core
QT       += network
QT       += xml

QT       -= gui

TARGET = SAMP_Server
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    server.cpp \
    xmlparser.cpp \
    client.cpp \
    samp/sampserver.cpp \
    crypt/simpleqtrc5.cpp

HEADERS += \
    server.h \
    xmlparser.h \
    client.h \
    samp/sampserver.h \
    crypt/simpleqtrc5.h

OTHER_FILES += \
    config.ini
