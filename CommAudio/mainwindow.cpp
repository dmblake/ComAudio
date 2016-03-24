#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "server.h"
#include "client.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startServerButton_clicked()
{
    startServer();
}

void MainWindow::on_startMulticastServer_clicked()
{

}

void MainWindow::on_startClientButton_clicked()
{
    startFileTransfer();
}

void MainWindow::on_startClientMulticast_clicked()
{
    startClientMulticastSession();
}
