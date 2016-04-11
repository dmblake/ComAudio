#-------------------------------------------------
#
# Project created by QtCreator 2016-03-15T16:58:14
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CommAudio
TEMPLATE = app



SOURCES += main.cpp\
        mainwindow.cpp \
    server.cpp \
    client.cpp \
    network.cpp \
    playback.cpp \
    circularbuffer.cpp \
    FileUtil.cpp \
    settingsdialog.cpp \
    microphonedialog.cpp

HEADERS  += mainwindow.h \
    server.h \
    main.h \
    shared.h \
    client.h \
    network.h \
    playback.h \
    circularbuffer.h \
    bass.h \
    dirent.h \
    FileUtil.h \
    settingsDialog.h \
    microphonedialog.h


LIBS += -LC:\dicom\lib -lws2_32 -lwsock32 -L"$$PWD\lib" -lbass

INCLUDEPATH += D:\QtProjects\ComAudio\CommAudio\lib

FORMS    += mainwindow.ui \
    settingsdialog.ui \
    microphonedialog.ui

RESOURCES += \
    images.qrc \
    qdarkstyle/style.qrc
