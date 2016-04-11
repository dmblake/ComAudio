#include "mainwindow.h"
#include <QApplication>

#include "shared.h"
#include "main.h"
#include "network.h"
#include "server.h"
#include "settingsDialog.h"
#include <iostream>

bool server;

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
//    if (isServer)
//    {
//        serverCleanup();
//    }
//    else
//    {
//        clientCleanup();
//    }

    return r;
}




