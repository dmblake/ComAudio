#-------------------------------------------------
#
# Project created by QtCreator 2016-03-15T16:58:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CommAudio
TEMPLATE = app



SOURCES += main.cpp\
        mainwindow.cpp \
    server.cpp \
    client.cpp \
    network.cpp \
    playback.cpp \
    circularbuffer.cpp

HEADERS  += mainwindow.h \
    server.h \
    main.h \
    shared.h \
    client.h \
    network.h \
    playback.h \
    circularbuffer.h \
    bass.h

LIBS += -LC:\dicom\lib -lws2_32 -lwsock32 -L"D:\QtProjects\ComAudio\CommAudio\lib\bass.lib"

INCLUDEPATH += D:\QtProjects\ComAudio\CommAudio\lib

FORMS    += mainwindow.ui
