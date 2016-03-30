#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "server.h"
#include "client.h"

// hank revision
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mw = this;
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

//hank
void MainWindow::printToListView(std::string msg)
{
    ui->listWidget->addItem(QString::fromStdString(msg));
}
