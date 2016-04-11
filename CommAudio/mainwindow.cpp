#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "server.h"
#include "FileUtil.h"
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
    mw = this;

    if(server == true){
        startServer();
    }
    else if (client == true){
        startClientMulticastSession();
        setupTcpSocket(ipaddr);
    }

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


void MainWindow::on_playButton_clicked()
{
    ui->playButton->setDisabled(true);
    ui->pauseButton->setEnabled(true);
    ui->stopButton->setEnabled(true);
}

void MainWindow::on_pauseButton_clicked()
{
    ui->pauseButton->setDisabled(true);
    ui->playButton->setEnabled(true);
    ui->stopButton->setEnabled(true);
}

void MainWindow::on_stopButton_clicked()
{
    ui->stopButton->setDisabled(true);
    ui->pauseButton->setEnabled(true);
    ui->playButton->setEnabled(true);
}



void MainWindow::on_updateButton_clicked()
{
    startFileTransfer();
}
