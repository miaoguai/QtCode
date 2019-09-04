 #-------------------------------------------------
#
# Project created by QtCreator 2019-07-25T23:21:57
#
#-------------------------------------------------

QT += core gui
QT += concurrent
QT += webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FQA
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32:LIBS += -LE:/openssl-1.0.2j/OpenSSL64/lib/ -llibeay32
win32:LIBS += -LE:/openssl-1.0.2j/OpenSSL64/lib/ -lssleay32

INCLUDEPATH += E:/openssl-1.0.2j/OpenSSL64/include
DEPENDPATH += E:/openssl-1.0.2j/OpenSSL64/include

win32:LIBS += -L$$PWD/lib/ -lEverything64

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

SOURCES += \
        login.cpp \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        login.h \
        mainwindow.h

FORMS += \
        login.ui \
        mainwindow.ui

RESOURCES += \
    resource.qrc
