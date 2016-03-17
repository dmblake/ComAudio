#include "mainwindow.h"
#include <QApplication>

#include "shared.h"
#include "main.h"
#include "network.h"
#include "server.h"
#include "client.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    startWinsock();
    fillMyAddrStruct();
    //startServer();
    startFileTransfer();
    //setUdpSocket();

    int r = a.exec();

    serverCleanup();
    clientCleanup();

    return r;
}




