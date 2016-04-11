#include "mainwindow.h"
#include <QApplication>

#include "shared.h"
#include "main.h"
#include "network.h"
#include "server.h"
#include "settingsDialog.h"
#include <iostream>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //MainWindow w;
    //w.show();

    settingsDialog s;
    s.show();


    startWinsock();
    fillMyAddrStruct();
    fillMcastAddrStruct();


    //setUdpSocket();

    int r = a.exec();

    //serverCleanup();
    //clientCleanup();

    return r;
}




