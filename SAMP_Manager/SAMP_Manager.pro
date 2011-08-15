#-------------------------------------------------
#
# Project created by QtCreator 2011-07-25T13:51:51
#
#-------------------------------------------------

QT       += core gui
QT       += network

TARGET = SAMP_Manager
TEMPLATE = app

TRANSLATIONS = sampManager_de.ts


SOURCES +=  main.cpp\
            gui/mainwindow.cpp \
            gui/adminwindow.cpp \
            client.cpp \
            gui/login.cpp \
    gui/subgui/adminaddserver.cpp \
    gui/subgui/adminadduser.cpp \
    gui/subgui/filecreatedir.cpp \
    gui/subgui/filetransfer.cpp \
    gui/subgui/changepassword.cpp \
    gui/subgui/showfile.cpp \
    crypt/simpleqtrc5.cpp \
    gui/subgui/adminaddnews.cpp

HEADERS  += gui/mainwindow.h \
            gui/adminwindow.h \
            client.h \
            gui/login.h \
    gui/subgui/adminaddserver.h \
    gui/subgui/adminadduser.h \
    gui/subgui/filecreatedir.h \
    gui/subgui/filetransfer.h \
    gui/subgui/changepassword.h \
    gui/subgui/showfile.h \
    crypt/simpleqtrc5.h \
    gui/subgui/adminaddnews.h

FORMS    += gui/mainwindow.ui \
            gui/adminwindow.ui \
            gui/login.ui \
    gui/subgui/adminaddserver.ui \
    gui/subgui/adminadduser.ui \
    gui/subgui/filecreatedir.ui \
    gui/subgui/filetransfer.ui \
    gui/subgui/changepassword.ui \
    gui/subgui/showfile.ui \
    gui/subgui/adminaddnews.ui

OTHER_FILES += \
    config.ini
