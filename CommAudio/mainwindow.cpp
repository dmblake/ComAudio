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


MainWindow::MainWindow(bool server,bool client, QString ipaddr):_server(server),_client(client),_ipaddr(ipaddr),
    ui(new Ui::MainWindow){

    ui->setupUi(this);
    qDebug() << server;
    qDebug() << client;
    qDebug() << ipaddr;
}

MainWindow::~MainWindow()
{
    delete ui;
}


//hank
void MainWindow::printToListView(std::string msg)
{
    ui->listWidget->addItem(QString::fromStdString(msg));
}

//void MainWindow::on_playbackButton_clicked()
//{
//    playback();
//}

void MainWindow::on_pushButton_3_clicked()
{
    startServer();
}

void MainWindow::on_pushButton_4_clicked()
{
    startClientMulticastSession();
}
